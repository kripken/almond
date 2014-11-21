/*****************************************************************************
*
*                      Higgs JavaScript Virtual Machine
*
*  This file is part of the Higgs project. The project is distributed at:
*  https://github.com/maximecb/Higgs
*
*  Copyright (c) 2011-2014, Maxime Chevalier-Boisvert. All rights reserved.
*
*  This software is licensed under the following license (Modified BSD
*  License):
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are
*  met:
*   1. Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*   3. The name of the author may not be used to endorse or promote
*      products derived from this software without specific prior written
*      permission.
*
*  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
*  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
*  NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
*  NOT LIMITED TO PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
*  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

namespace almond {

/**
Parsing error exception
*/
struct ParseError
{
    std::string msg;

    /// Source position
    SrcPos* pos;

    ParseError(std::string msg_, SrcPos* pos_) : msg(msg_), pos(pos_)
    {
        assert (pos); // "source position is null");
    }

    std::string toString()
    {
        return "ParseError: " + msg;
    }
};

template<class ASTNode, class Builder>
struct Parser {

/**
Read and consume a separator token. A parse error
is thrown if the separator is missing.
*/
void readSep(TokenStream& input, std::string sep)
{
    if (!input.matchSep(sep))
    {
        throw new ParseError(
            "expected \"" + sep + "\" separator",
            input.getPos()
        );
    }
}

/**
Read and consume a keyword token. A parse error
is thrown if the keyword is missing.
*/
void readKw(TokenStream& input, std::string keyword)
{
    if (input.matchKw(keyword) == false)
    {
        throw new ParseError(
            "expected \"" + keyword + "\" keyword",
            input.getPos()
        );
    }
}

/**
Test if a semicolon is present or one could be automatically inserted
at the current position
*/
bool peekSemiAuto(TokenStream& input)
{
    return (
        input.peekSep(";") ||
        input.peekSep("}") ||
        input.newline() ||
        input.eof()
    );
}

/**
Read and consume a semicolon or an automatically inserted semicolon
*/
void readSemiAuto(TokenStream& input)
{
    if (!input.matchSep(";") && !peekSemiAuto(input))
    {
        throw new ParseError(
            "expected semicolon or end of statement",
            input.getPos()
        );
    }
}

/**
Read an identifier token from the input
*/
std::string readIdent(TokenStream& input)
{
    auto t = input.read();

    if (t->type != Token::IDENT)
        throw new ParseError("expected identifier", t->pos);

    return t->stringVal;
    // XXX leak    delete t;
}

/**
Parse a source file
*/
ASTNode* parseFile(std::string fileName, char *src, bool isRuntime = false)
{
    StrStream strStream(src, fileName);

    // If there is a shebang line at the beginning of the file
    if (strStream.match("#!"))
    {
        // Consume all characters until the end of line
        for (;;)
        {
            auto ch = strStream.peekCh();

            if (ch == '\r' || ch == '\n')
            {
                break;
            }

            if (ch == '\0')
            {
                throw new ParseError(
                    "end of input in shebang line",
                    strStream.getPos()
                );
            }

            strStream.readCh();
        }
    }

    TokenStream input(&strStream);

    return parseProgram(input, isRuntime);
}

/**
Parse a source string
*/
ASTNode* parseString(char* src, std::string fileName = "", bool isRuntime = false)
{
    StrStream strStream(src, fileName);
    TokenStream input(&strStream);

    return parseProgram(input, isRuntime);
}

/**
Parse a top-level program node
*/
ASTNode* parseProgram(TokenStream& input, bool isRuntime)
{
    SrcPos* pos = input.getPos();

    ASTNode* program = Builder::makeToplevel();

    while (!input.eof())
    {
        ASTNode* stmt = parseStmt(input);
        Builder::appendStatement(program, stmt);
    }

    return program;
}

class ScopeExit {
  std::function<void ()> func;
public:
  ScopeExit(std::function<void ()> func_) : func(func_) {}
  ~ScopeExit() { func(); }
};

/**
Parse a statement
*/
ASTNode* parseStmt(TokenStream& input)
{
    /// Test if this is a label statement and backtrack
    auto isLabel = [](TokenStream& input)
    {
        // Copy the starting input to allow backtracking
        TokenStream startInput(input);

        // On return, backtrack to the start
        ScopeExit([&]() {
            input.backtrack(startInput);
        });

        auto t = input.peek();
        if (t->type != Token::IDENT)
            return false;
        input.read();

        return input.matchSep(":");
    };

    // Get the current source position
    SrcPos* pos = input.getPos();

    // Empty statement
    if (input.matchSep(";"))
    {
        return Builder::makeEmpty();
    }

    // Block statement
    else if (input.matchSep("{"))
    {
        ASTNode* stmts = nullptr; // MAKE block

        for (;;)
        {
            if (input.matchSep("}"))
                break;

            if (input.eof())
            {
                throw new ParseError(
                    "end of input in block statement",
                    input.getPos()
                );
            }

            Builder::appendStatement(stmts, parseStmt(input));
        }

        return stmts;
    }

    // If statement
    else if (input.matchKw("if"))
    {
        readSep(input, "(");
        ASTNode* testExpr = parseExpr(input);
        readSep(input, ")");

        auto trueStmt = parseStmt(input);

        ASTNode* falseStmt;
        if (input.matchKw("else"))
            falseStmt = parseStmt(input);
        else
            falseStmt = nullptr;

        return Builder::makeIf(testExpr, trueStmt, falseStmt);
    }

    // While loop
    else if (input.matchKw("while"))
    {
        readSep(input, "(");
        auto testExpr = parseExpr(input);
        readSep(input, ")");
        auto bodyStmt = parseStmt(input);

        return Builder::makeWhile(testExpr, bodyStmt);
    }

    // Do-while loop
    else if (input.matchKw("do"))
    {
        auto bodyStmt = parseStmt(input);
        if (input.matchKw("while") == false)
            throw new ParseError("expected while", input.getPos());
        readSep(input, "(");
        auto testExpr = parseExpr(input);
        readSep(input, ")");

        return Builder::makeDo(bodyStmt, testExpr);
    }

    // For or for-in loop
    else if (input.peekKw("for"))
    {
        return parseForStmt(input);
    }

    // Switch statement
    else if (input.matchKw("switch"))
    {
        readSep(input, "(");
        auto switchExpr = parseExpr(input);
        readSep(input, ")");
        readSep(input, "{");

        bool defaultSeen = false;

        printf("switch\n");
        ASTNode* switch_ = Builder::makeSwitch(switchExpr);

        // For each case
        for (;;)
        {
            if (input.matchSep("}"))
            {
                break;
            }

            else if (input.matchKw("case"))
            {
                ASTNode* caseExpr = parseExpr(input);
                readSep(input, ":");

                Builder::appendSwitchCase(switch_, caseExpr);
            }

            else if (input.matchKw("default"))
            {
                readSep(input, ":");
                if (defaultSeen)
                    throw new ParseError("duplicate default label", input.getPos());

                defaultSeen = true;
                Builder::appendSwitchDefault(switch_);
            }

            else
            {
                ASTNode* statement = parseStmt(input);
                Builder::appendSwitchStatement(switch_, statement);
            }
        }

        return switch_;
    }

    // Break statement
    else if (input.matchKw("break"))
    {
        auto label = peekSemiAuto(input) ? nullptr : readIdent(input);
        readSemiAuto(input);
        return Builder::makeBreak(label);
    }

    // Continue statement
    else if (input.matchKw("continue"))
    {
        auto label = peekSemiAuto(input) ? nullptr : readIdent(input);
        readSemiAuto(input);
        return Builder::makeContinue(label);
    }

    // Return statement
    else if (input.matchKw("return"))
    {
        if (input.matchSep(";") || peekSemiAuto(input))
            return Builder::makeReturn(nullptr);

        ASTNode* expr = parseExpr(input);
        readSemiAuto(input);
        return Builder::makeReturn(expr);
    }

    // Throw statement
    else if (input.matchKw("throw"))
    {
        ASTNode* expr = parseExpr(input);
        readSemiAuto(input);
        return Builder::makeThrow(expr);
    }

    // Try-catch-finally statement
    else if (input.matchKw("try"))
    {
        auto tryStmt = parseStmt(input);

        ASTNode* catchIdent = nullptr;
        ASTNode* catchStmt = nullptr;
        if (input.matchKw("catch"))
        {
            readSep(input, "(");
            catchIdent = parseExpr(input);
            if (catchIdent == nullptr)
                throw new ParseError("invalid catch identifier", input.getPos());
            readSep(input, ")");
            catchStmt = parseStmt(input);
        }

        ASTNode* finallyStmt = nullptr;
        if (input.matchKw("finally"))
        {
            finallyStmt = parseStmt(input);
        }

        if (!catchStmt && !finallyStmt)
            throw new ParseError("no catch or finally block", input.getPos());

        return Builder::makeTry(
            tryStmt, 
            catchIdent, 
            catchStmt,
            finallyStmt
        );
    }

    // Variable declaration/initialization statement
    else if (input.matchKw("var"))
    {
        bool firstIdent = true;

        ASTNode* vars = Builder::makeVars();

        // For each declaration
        for (;;)
        {
            // If this is not the first declaration and there is no comma
            if (!firstIdent && input.matchSep(",") == false)
            {
                readSemiAuto(input);
                break;
            }

            auto name = input.read();
            if (name->type != Token::IDENT)
            {
                throw new ParseError(
                    "expected identifier in variable declaration",
                    name->pos
                );
            }

            ASTNode* initExpr = nullptr;
            auto op = input.peek();

            if (op->type == Token::OP && op->stringVal == "=")
            {
                input.read(); 
                initExpr = parseExpr(input, COMMA_PREC+1);
            }

            Builder::appendVar(vars, name->stringVal.c_str(), initExpr);
            firstIdent = false;
        }

        return vars;
    }

    // Function declaration statement
    else if (input.peekKw("function"))
    {
        auto funExpr = parseAtom(input);

        // Weed out trailing semicolons
        if (input.peekSep(";"))
            input.read();

        return funExpr;
    }

    // If this is a labelled statement
    else if (isLabel(input))
    {
        auto label = input.read();
        assert(label->type == Token::IDENT);
        readSep(input, ":");
        auto stmt = parseStmt(input);

        return Builder::makeLabel(label->stringVal.c_str(), stmt);
    }

    // Peek at the token at the start of the expression
    auto startTok = input.peek();

    // Parse as an expression statement
    ASTNode* expr = parseExpr(input);

    // Peek at the token after the expression
    auto endTok = input.peek();

    // If the statement is empty
    if (*endTok == *startTok)
    {
        throw new ParseError(
            "empty statements must be terminated by semicolons",
            endTok->pos
        );
    }

    // Read the terminating semicolon
    readSemiAuto(input);

    return expr;
}

/**
Parse a for or for-in loop statement
*/
ASTNode* parseForStmt(TokenStream& input)
{
    /// Test if this is a for-in statement and backtrack
    auto isForIn = [&](TokenStream& input)
    {
        // Copy the starting input to allow backtracking
        TokenStream startInput(input);

        // On return, backtrack to the start
        ScopeExit([&]() {
            input.backtrack(startInput);
        });

        // Test if there is a variable declaration
        auto hasDecl = input.matchKw("var");

        if (input.peekSep(";"))
            return false;

        // Parse the first expression, stop at comma if there is a declaration
        auto firstExpr = parseExpr(input, hasDecl ? (COMMA_PREC+1) : COMMA_PREC);

        if (input.peekSep(";"))
            return false;

        if (Builder::isBinary(firstExpr, "in"))
            return true;

        return false;
    };

    // Get the current position
    auto pos = input.getPos();

    // Read the for keyword and the opening parenthesis
    readKw(input, "for");
    readSep(input, "(");

    // If this is a regular for-loop statement
    if (isForIn(input) == false)
    {
        // Parse the init statement
        auto initStmt = parseStmt(input);
        if (!Builder::isVar(initStmt) && !Builder::isExpression(initStmt))
            throw new ParseError("invalid for-loop init statement", initStmt.pos);

        // Parse the test expression
        pos = input.getPos();
        ASTNode* testExpr;
        if (input.matchSep(";"))
        {
            testExpr = nullptr;
        }
        else
        {
            testExpr = parseExpr(input);
            readSep(input, ";");
        }

        // Parse the inccrement expression
        pos = input.getPos();
        ASTNode* incrExpr;
        if (input.matchSep(")"))
        {
            incrExpr = nullptr;
        }
        else
        {
            incrExpr = parseExpr(input);
            readSep(input, ")");
        }

        // Parse the loop body
        auto bodyStmt = parseStmt(input);

        return Builder::makeFor(initStmt, testExpr, incrExpr, bodyStmt, pos);
    }

    // This is a for-in statement
    else
    {
        auto hasDecl = input.matchKw("var");
        auto varExpr = parseExpr(input, IN_PREC+1);
        if (hasDecl && !Builder::isName(varExpr)) // XXX
            throw new ParseError("invalid variable expression in for-in loop", pos);

        auto inTok = input.peek();
        if (inTok->type != Token::OP || inTok->stringVal != "in")
            throw new ParseError("expected \"in\" keyword", input.getPos());
        input.read();

        auto inExpr = parseExpr(input);

        readSep(input, ")");

        // Parse the loop body
        auto bodyStmt = parseStmt(input);

        return Builder::makeForIn(hasDecl, varExpr, inExpr, bodyStmt, pos);
    }
}

/**
Parse an expression
*/
ASTNode* parseExpr(TokenStream& input, int minPrec = 0)
{
    // Expression parsing using the precedence climbing algorithm
    //    
    // The first call has min precedence 0
    //
    // Each call loops to grab everything of the current precedence or
    // greater and builds a left-sided subtree out of it, associating
    // operators to their left operand
    //
    // If an operator has less than the current precedence, the loop
    // breaks, returning us to the previous loop level, this will attach
    // the atom to the previous operator (on the right)
    //
    // If an operator has the mininum precedence or greater, it will
    // associate the current atom to its left and then parse the rhs

    // Parse the first atom
    ASTNode* lhsExpr = parseAtom(input);

    for (;;)
    {
        // Peek at the current token
        auto cur = input.peek();

        // If the token is not an operator or separator, break out
        if (cur->type != Token::OP && cur->type != Token::SEP)
            break;

        //writefln("op str: %s", cur->stringVal);

        // Attempt to find a corresponding operator
        auto op = findOperator(cur->stringVal, 2);
        if (!op)
            op = findOperator(cur->stringVal, 1, 'l');
        if (!op && cur->stringVal == "?")
            op = findOperator(cur->stringVal, 3);

        // If no operator matches, break out
        if (!op)
            break;

        // If the new operator has lower precedence, break out
        if (op->prec < minPrec)
            break;

        // Compute the minimal precedence for the recursive call (if any)
        int nextMinPrec = (op->assoc == 'l') ? (op->prec + 1) : op->prec;

        // If this is a function call expression
        if (cur->stringVal == "(")
        {
            // Parse the argument list and create the call expression
            auto argExprs = parseExprList(input, "(", ")");
            lhsExpr = Builder::makeCall(lhsExpr, argExprs); // lhsExpr.pos);
        }

        // If this is an array indexing expression
        else if (input.matchSep("["))
        {
            auto indexExpr = parseExpr(input);
            readSep(input, "]");
            lhsExpr = Builder::makeSub(lhsExpr, indexExpr); //, lhsExpr.pos);
        }

        // If this is a member expression
        else if (op->str == ".")
        {
            input.read();

            // Parse the identifier string
            auto tok = input.read();
            if (!(tok->type == Token::IDENT) &&
                !(tok->type == Token::KEYWORD) &&
                !(tok->type == Token::OP && ident(tok->stringVal.c_str())))
            {
                throw new ParseError(
                    "invalid member identifier \"" + tok->toString() + "\"", 
                    tok->pos
                );
            }

            // Produce an indexing expression
            lhsExpr = Builder::makeIndex(lhsExpr, tok->stringVal); //, lhsExpr.pos);
        }

        // If this is the ternary conditional operator
        else if (cur->stringVal == "?")
        {
            // Consume the current token
            input.read();

            auto trueExpr = parseExpr(input);
            readSep(input, ":");
            auto falseExpr = parseExpr(input, op->prec-1);

            lhsExpr = Builder::makeConditional(lhsExpr, trueExpr, falseExpr); //, lhsExpr.pos);
        }

        // If this is a binary operator
        else if (op->arity == 2)
        {
            // Consume the current token
            input.read();

            // Recursively parse the rhs
            ASTNode* rhsExpr = parseExpr(input, nextMinPrec);

            // Convert expressions of the form "x <op>= y" to "x = x <op> y" // XXX
            auto eqOp = findOperator("=", 2, 'r');
            if (op->str.size() >= 2 && op->str.back() == '=' && op->prec == eqOp->prec)
            {
                auto rhsOp = findOperator(op->str.substr(0, op->str.size()-1), 2);
                assert (rhsOp != nullptr);
                rhsExpr = Builder::makeBinary(rhsOp, lhsExpr, rhsExpr); //, rhsExpr->pos);
                op = eqOp;
            }

            // Update lhs with the new value
            lhsExpr = Builder::makeBinary(op, lhsExpr, rhsExpr); //, lhsExpr.pos);
        }

        // If this is a unary operator
        else if (op->arity == 1)
        {
            // Consume the current token
            input.read();

            // Update lhs with the new value
            lhsExpr = Builder::makeUnary(op, lhsExpr); //, lhsExpr.pos);
        }

        else
        {
            assert (false); // "unhandled operator");
        }
    }

    // Return the parsed expression
    return lhsExpr;
}

/**
Parse an atomic expression
*/
ASTNode* parseAtom(TokenStream& input)
{
    auto t = input.peek(LEX_MAYBE_RE);
    SrcPos* pos = t->pos;

    // End of file
    if (input.eof())
    {
        throw new ParseError("end of input inside expression", pos);
    }

    // Parenthesized expression
    else if (input.matchSep("("))
    {
        ASTNode* expr = parseExpr(input);
        readSep(input, ")");
        return expr;
    }

    // Array literal
    else if (t->type == Token::SEP && t->stringVal == "[")
    {
        auto exprs = parseExprList(input, "[", "]");
        return Builder::makeArray(exprs, pos);
    }

    // Object literal
    else if (input.matchSep("{"))
    {
        assert(0);
        /*
        StringExpr[] names = [];
        ASTNode*[] values = [];

        // For each property
        for (;;)
        {
            // If this is the end of the literal, stop
            if (input.matchSep("}"))
                break;

            // Read a property name
            auto tok = input.read();
            StringExpr stringExpr = null;
            if (tok.type is Token::IDENT ||
                tok.type is Token::KEYWORD ||
                tok.type is Token::STRING)
                stringExpr = new StringExpr(tok.stringVal, tok.pos);
            if (tok.type is Token::OP && ident(tok.stringVal))
                stringExpr = new StringExpr(tok.stringVal, tok.pos);
            else if (tok.type is Token::INT)
                stringExpr = new StringExpr(to!std::string(tok.intVal), tok.pos);

            if (!stringExpr)
                throw new ParseError("expected property name in object literal", tok.pos);
            names ~= [stringExpr];

            readSep(input, ":");

            // Parse an expression with priority above the comma operator
            auto valueExpr = parseExpr(input, COMMA_PREC+1);
            values ~= [valueExpr];

            // If there is no separating comma
            if (!input.matchSep(","))
            {
                // If this is the end of the literal, stop
                if (input.matchSep("}"))
                    break;

                // Comma expected before next property
                throw new ParseError("expected comma in object literal", input.getPos());
            }
        }

        return new ObjectExpr(names, values, pos);
        */
    }

    // Regular expression literal
    else if (t->type == Token::REGEXP)
    {
        assert(0);
        /*
        input.read();
        return new RegexpExpr(t.regexpVal, t.flagsVal, pos);
        */
    }

    // New expression
    else if (t->type == Token::OP && t->stringVal == "new")
    {
        // Consume the "new" token
        input.read();

        // Parse the base expression
        auto op = findOperator(t->stringVal, 1, 'r');
        auto baseExpr = parseExpr(input, op->prec);

        // Parse the argument list (if present, otherwise assumed empty)
        auto argExprs = input.peekSep("(") ? parseExprList(input, "(", ")") : nullptr;

        // Create the new expression
        return Builder::makeNew(baseExpr, argExprs, t->pos);
    }

    // Function expression
    // function (params) body
    else if (input.matchKw("function"))
    {
        auto nextTok = input.peek();
        auto nameExpr = (nextTok->type != Token::SEP) ? parseAtom(input) : nullptr;
        if (nameExpr && !Builder::isName(nameExpr))
            throw new ParseError("invalid function name", nameExpr.pos);

        auto params = parseParamList(input);

        auto bodyStmt = parseStmt(input);

        return Builder::makeFun(nameExpr, params, bodyStmt, pos);
    }

    // Identifier/symbol literal
    else if (t->type == Token::IDENT)
    {
        input.read();
        return Builder::makeName(t->stringVal, pos);
    }

    // Integer literal
    else if (t->type == Token::INT)
    {
        input.read();
        return Builder::makeNum(t->intVal, pos);
    }

    // Floating-point literal
    else if (t->type == Token::FLOAT)
    {
        input.read();
        return Builder::makeNum(t->floatVal, pos);
    }

    // String literal
    else if (t->type == Token::STRING)
    {
        input.read();
        return Builder::makeString(t->stringVal, pos);
    }

    // True boolean constant
    else if (input.matchKw("true"))
    {
        return Builder::makeBool(true, pos);
    }

    // False boolean constant
    else if (input.matchKw("false"))
    {
        return Builder::makeBool(false, pos);
    }

    // Null constant
    else if (input.matchKw("null"))
    {
        return Builder::makeNull(pos);
    }

    // Unary expressions
    else if (t->type == Token::OP)
    {
        auto op = findOperator(t->stringVal, 1, 'r');
        if (!op)
        {
            throw new ParseError(
                "invalid unary operator \"" + t->stringVal + "\"", 
                pos
            );
        }

        // Consume the operator
        input.read();

        // Parse the right subexpression
        ASTNode* expr = parseExpr(input, op->prec);

        // Return the unary expression
        return Builder::makeUnary(op, expr, pos);
    }

    throw new ParseError("unexpected token: " + t->toString(), pos);
}

/**
Parse a list of expressions
*/
ASTNode* parseExprList(TokenStream& input, std::string openSep, std::string closeSep)
{
    readSep(input, openSep);

    ASTNode* exprs = Builder::makeList();

    for (;;)
    {
        if (input.matchSep(closeSep))
            break;

        // If this is not the first element and there
        // is no comma separator, throw an error
        if (Builder::getSize(exprs) > 0 && input.matchSep(",") == false)
            throw new ParseError("expected comma", input.getPos());

        // Handle missing array element syntax
        if (openSep == "[")
        {
            if (input.matchSep(closeSep))
                break;

            if (input.peekSep(",")) 
            {
                Builder::append(exprs, Builder::makeUndefined());
                continue;
            }
        }

        // Parse the current element
        Builder::append(exprs, parseExpr(input, COMMA_PREC+1));
    }

    return exprs;
}

/**
Parse a function declaration's parameter list
*/
ASTNode* parseParamList(TokenStream& input)
{
    readSep(input, "(");

    ASTNode* exprs = Builder::makeList();

    for (;;)
    {
        if (input.matchSep(")"))
            break;

        if (Builder::getSize(exprs) > 0 && input.matchSep(",") == false)
            throw new ParseError("expected comma", input.getPos());

        auto expr = parseAtom(input);
        if (!Builder::isName(expr))
            throw new ParseError("invalid parameter", expr.pos);

        Builder::append(exprs, expr);
    }

    return exprs;
}

}; // struct Parser

} // namespace almond

