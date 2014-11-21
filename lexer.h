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

#include <string>
#include <algorithm> // for sort
#include <regex>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

namespace almond {

/**
Operator information structure
*/
struct OpInfo
{
    /// String representation
    std::string str;

    /// Operator arity
    int arity;

    /// Precedence level
    int prec;

    /// Associativity, left-to-right or right-to-left
    char assoc;

    /// Non-associative flag (e.g.: - and / are not associative)
    bool nonAssoc;
};

typedef const OpInfo* Operator;

// Maximum op precedence
const int MAX_PREC = 16;

// Comma op precedence (least precedence)
const int COMMA_PREC = 0;

// In op precedence
const int IN_PREC = 9;

/**
Operator table
*/
OpInfo operators[] = {

    // Member op
    { ".", 2, 16, 'l', false },

    // Array indexing
    { "[", 1, 16, 'l', false },

    // New/constructor op
    { "new", 1, 16, 'r', false },

    // Function call
    { "(", 1, 15, 'l', false },

    // Postfix unary ops
    { "++", 1, 14, 'l', false },
    { "--", 1, 14, 'l', false },

    // Prefix unary ops
    { "+" , 1, 13, 'r', false },
    { "-" , 1, 13, 'r', false },
    { "!" , 1, 13, 'r', false },
    { "~" , 1, 13, 'r', false },
    { "++", 1, 13, 'r', false },
    { "--", 1, 13, 'r', false },
    { "typeof", 1, 13, 'r', false },
    { "void", 1, 13, 'r', false },
    { "delete", 1, 13, 'r', false },

    // Multiplication/division/modulus
    { "*", 2, 12, 'l', false },
    { "/", 2, 12, 'l', true },
    { "%", 2, 12, 'l', true },

    // Addition/subtraction
    { "+", 2, 11, 'l', false },
    { "-", 2, 11, 'l', true },

    // Bitwise shift
    { "<<" , 2, 10, 'l', false },
    { ">>" , 2, 10, 'l', false },
    { ">>>", 2, 10, 'l', false },

    // Relational ops
    { "<"         , 2, IN_PREC, 'l', false },
    { "<="        , 2, IN_PREC, 'l', false },
    { ">"         , 2, IN_PREC, 'l', false },
    { ">="        , 2, IN_PREC, 'l', false },
    { "in"        , 2, IN_PREC, 'l', false },
    { "instanceof", 2, IN_PREC, 'l', false },

    // Equality comparison
    { "==" , 2, 8, 'l', false },
    { "!=" , 2, 8, 'l', false },
    { "===", 2, 8, 'l', false },
    { "!==", 2, 8, 'l', false },

    // Bitwise ops
    { "&", 2, 7, 'l', false },
    { "^", 2, 6, 'l', false },
    { "|", 2, 5, 'l', false },

    // Logical ops
    { "&&", 2, 4, 'l', false },
    { "||", 2, 3, 'l', false },

    // Ternary conditional
    { "?", 3, 2, 'r', false },

    // Assignment
    { "="   , 2, 1, 'r', false },
    { "+="  , 2, 1, 'r', false },
    { "-="  , 2, 1, 'r', false },
    { "*="  , 2, 1, 'r', false },
    { "/="  , 2, 1, 'r', false },
    { "%="  , 2, 1, 'r', false },
    { "&="  , 2, 1, 'r', false },
    { "|="  , 2, 1, 'r', false },
    { "^="  , 2, 1, 'r', false },
    { "<<=" , 2, 1, 'r', false },
    { ">>=" , 2, 1, 'r', false },
    { ">>>=", 2, 1, 'r', false },

    // Comma (sequencing), least precedence
    { ",", 2, COMMA_PREC, 'l', false },
};

#define NUM_OPERATORS sizeof(operators)/sizeof(operators[0])

/**
Separator tokens
*/
std::string separators[] = {
    ",",
    ":",
    ";",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}"
};

#define NUM_SEPARATORS sizeof(separators)/sizeof(separators[0])

/**
Keyword tokens
*/
std::string keywords[] = {
    "var",
    "function",
    "if",
    "else",
    "do",
    "while",
    "for",
    "break",
    "continue",
    "return",
    "switch",
    "case",
    "default",
    "throw",
    "try",
    "catch",
    "finally",
    "true",
    "false",
    "null"
};

#define NUM_KEYWORDS sizeof(keywords)/sizeof(keywords[0])

/**
Static module constructor to initialize the
separator, keyword and op tables
*/
void init()
{
    // Sort the tables by decreasing string length
    std::sort(operators, operators + NUM_OPERATORS, [](OpInfo a, OpInfo b) { return a.str.size() > b.str.size(); });
    std::sort(separators, separators + NUM_SEPARATORS, [](std::string a, std::string b) { return a.size() > b.size(); });
    std::sort(keywords, keywords + NUM_KEYWORDS, [](std::string a, std::string b) { return a.size() > b.size(); });
}

/**
Find an op by string, arity and associativity
*/
Operator findOperator(std::string op, int arity = 0, char assoc = '\0')
{
    for (size_t i = 0; i < NUM_OPERATORS; ++i)
    {
        Operator operator_ = &operators[i];

        if (operator_->str != op)
            continue;

        if (arity != 0 && operator_->arity != arity)
            continue;

        if (assoc != '\0' && operator_->assoc != assoc)
            continue;

        return operator_;
    }

    return nullptr;
}

/**
Source code position
*/
struct SrcPos
{
    /// File name
    std::string file;

    /// Line number
    int line;

    /// Column number
    int col;

    SrcPos(std::string file_, int line_, int col_)
    {
        file = file_;
        line = line_;
        col = col_;
    }

    std::string toString()
    {
        return "TODO"; // format("\"%s\"@%d:%d", file, line, col);
    }
};

/**
String stream, used to lex from strings
*/
struct StrStream
{
    /// Input string
    char* str;
    int strLen;

    /// File name
    std::string file;

    // Current index
    int index;

    /// Current line number
    int line;

    /// Current column
    int col;

    StrStream(char* str_, std::string file_) : index(0), line(1), col(1)
    {
        str = str_;
        strLen = strlen(str);
        file = file_;
    }

    /// Read a character and advance the current index
    char readCh()
    {
        char ch = (index < strLen) ? str[index] : '\0';

        index++;

        if (ch == '\n')
        {
            line++;
            col = 1;
        }
        else if (ch != '\r')
        {
            col++;
        }

        return ch;
    }

    /// Read a character without advancing the index
    char peekCh(size_t ofs = 0)
    {
        char ch = (index + ofs < strLen) ? str[index + ofs] : '\0';
        return ch;
    }

    /// Test for a match with a given string, the string is consumed if matched
    bool match(const char *str_)
    {
        if (index + strLen > strLen)
            return false;

        if (strncmp(str_, str+index, strlen(str_)))
            return false;

        // Consume the characters
        for (int i = 0; i < strLen; ++i)
            readCh();

        return true;
    }

    /// Test for a match with a regupar expression
    std::smatch match(std::regex re)
    {
        std::smatch m;
        if (std::regex_search(std::string(str), m, re))
        {
            for (auto it : m)
            {
                for (int i = 0; i < it.length(); ++i)
                    readCh();
            }
        }
        return m;
    }

    /// Get a position object for the current index
    SrcPos* getPos()
    {
        return new SrcPos(file, line, col); // XXX leak
    }
};


bool whitespace(char ch)
{
    return (ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t');
}

bool alpha(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool digit(char ch)
{
    return (ch >= '0' && ch <= '9');
}

bool identStart(char ch)
{
    return alpha(ch) || ch == '_' || ch == '$';
}

bool identPart(char ch)
{
    return identStart(ch) || digit(ch);
}

bool ident(char* str)
{
    if (str[0] == 0 || !identStart(str[0]))
        return false;

    while (*str) {
      if (!identPart(*str))
            return false;
      str++;
    }

    return true;
}

/**
Source token value
*/
struct Token
{
    enum Type
    {
        OP,
        SEP,
        IDENT,
        KEYWORD,
        INT,
        FLOAT,
        STRING,
        REGEXP,
        EOFF,
        ERROR
    };

    /// Token type
    Type type;

    /// Token value
    union
    {
        long intVal;
        double floatVal;
        std::string stringVal;
        struct {
            std::string regexpVal;
            std::string flagsVal;
        };
    };

    bool operator=(Token& other)
    {
        if (type != other.type) return false;
        if (type == INT) return intVal == other.intVal;
        if (type == FLOAT) return floatVal == other.floatVal;
        if (type == STRING) return stringVal == other.stringVal;
        if (type == REGEXP) return regexpVal == other.regexpVal && flagsVal == other.flagsVal;
        return true;
    }

    /// Source position
    SrcPos* pos;

    Token(Type type_, long val_, SrcPos* pos_) : type(type_), intVal(val_), pos(pos_)
    {
        assert (type_ == INT);
    }

    Token(Type type_, double val_, SrcPos* pos_) : type(type_), floatVal(val_), pos(pos_)
    {
        assert (type_ == FLOAT);
    }

    Token(Type type_, std::string val_, SrcPos* pos_) : type(type_), stringVal(val_), pos(pos_)
    {
        assert (
            type_ == OP      ||
            type_ == SEP     ||
            type_ == IDENT   ||
            type_ == KEYWORD ||
            type_ == STRING  ||
            type_ == ERROR
        );
    }

    Token(Type type_, std::string re_, std::string flags_, SrcPos* pos_) : type(type_), regexpVal(re_), flagsVal(flags_), pos(pos_)
    {
        assert (type == REGEXP);
    }

    Token(Type type_, SrcPos* pos_) : type(type_), pos(pos_)
    {
        assert (type == EOFF);
    }

    std::string toString()
    {
        return "TODO: Token";
        /*
        switch (type)
        {

            case OP:        return format("op:%s"  , stringVal);
            case SEP:       return format("separator:%s" , stringVal);
            case IDENT:     return format("identifier:%s", stringVal);
            case KEYWORD:   return format("keyword:%s"   , stringVal);
            case INT:       return format("int:%s"       , intVal);
            case FLOAT:     return format("float:%f"     , floatVal);
            case STRING:    return format("string:%s"    , stringVal);
            case REGEXP:    return format("regexp:/%s/%s", regexpVal, flagsVal);
            case ERROR:     return format("error:%s"     , stringVal);
            case EOFF:       return "EOFF";

            default:
            return "token";
        }
        */
    }
};

/**
Lexer flags, used to parameterize lexical analysis
*/
typedef unsigned int LexFlags;
const LexFlags LEX_MAYBE_RE = 1 << 0;

/**
Read a character escape sequence
*/
int readEscape(StrStream& stream)
{
    assert(0);
    /*
    // Hexadecimal escape sequence regular expressions
    enum hexRegex = ctRegex!(`^x([0-9|a-f|A-F]{2})`w);
    enum uniRegex = ctRegex!(`^u([0-9|a-f|A-F]{4})`w);

    // Try to match a hexadecimal escape sequence
    auto m = stream.match(hexRegex);
    if (m.empty == true)
        m = stream.match(uniRegex);
    if (m.empty == false)
    {
        auto hexStr = m.captures[1];

        int charCode;
        formattedRead(hexStr, "%x", &charCode);

        return charCode;
    }

    // Octal escape sequence regular expression
    enum octRegex = ctRegex!(`^([0-7][0-7]?[0-7]?)`w);

    // Try to match an octal escape sequence
    m = stream.match(octRegex);
    if (m.empty == false)
    {
        auto octStr = m.captures[1];

        int charCode;
        formattedRead(octStr, "%o", &charCode);

        return charCode;
    }

    auto code = stream.readCh();

    // Switch on the escape code
    switch (code)
    {
        case 'r' : return '\r';
        case 'n' : return '\n';
        case 'v' : return '\v';
        case 't' : return '\t';
        case 'f' : return '\f';
        case 'b' : return '\b';
        case 'a' : return '\a';
        case '\\': return '\\';
        case '\"': return '\"';
        case '\'': return '\'';

        // Multiline string continuation
        case '\n': return -1;

        // By default, add the escape character as is
        default:
        return code;
    }
    */
}

/**
Get the first token from a stream
*/
Token* getToken(StrStream& stream, LexFlags flags)
{
    char ch;

    // Consume whitespace and comments
    for (;;)
    {
        ch = stream.peekCh();

        // Whitespace characters
        if (whitespace(ch))
        {
            stream.readCh();
        }

        // Single-line comment
        else if (stream.match("//"))
        {
            for (;;)
            {
                ch = stream.readCh();
                if (ch == '\n' || ch == '\0')
                    break;
            }
        }

        // Multi-line comment
        else if (stream.match("/*"))
        {
            for (;;)
            {
                if (stream.match("*/"))
                    break;
                if (stream.peekCh() == '\0')
                    return new Token(
                        Token::ERROR,
                        "end of stream in multi-line comment", 
                        stream.getPos()
                    );
                ch = stream.readCh();
            }
        }

        // Otherwise
        else
        {
            break;
        }
    }

    // Get the position at the start of the token
    SrcPos* pos = stream.getPos();

    // Number (starting with a digit or .nxx)
    if (digit(ch) || (ch == '.' && digit(stream.peekCh(1))))
    {
        // Hexadecimal number
        if (stream.match("0x"))
        {
            static std::regex hexRegex("^[0-9|a-f|A-F]+");
            auto m = stream.match(hexRegex);

            if (m.empty())
            {
                return new Token(
                    Token::ERROR,
                    "invalid hex number", 
                    pos
                );
            }

            auto hexStr = m[0];
            long val;
            scanf(hexStr.str().c_str(), "%x", &val);

            return new Token(Token::INT, val, pos);
        }

        // Octal number
        if (ch == '0')
        {
            static std::regex octRegex("^0([0-7]+)");

            auto m = stream.match(octRegex);
            if (!m.empty())
            {
                auto octStr = m[1];
                long val;
                scanf(octStr.str().c_str(), "%o", &val);
                return new Token(Token::INT, val, pos);
            }
        }

        static std::regex fpRegex("^[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?");

        auto m = stream.match(fpRegex);
        assert (m.empty() == false);
        auto numStr = m[0].str().c_str();

        // If this is a floating-point number
        if (strchr(numStr, '.') ||
            strchr(numStr, 'e') ||
            strchr(numStr, 'E'))
        {
            double val = atof(numStr);
            return new Token(Token::FLOAT, val, pos);
        }

        // Integer number
        else
        {
            long val = atoi(numStr);
            return new Token(Token::INT, val, pos);
        }
    }

    // String constant
    if (ch == '"' || ch == '\'')
    {
        auto openChar = stream.readCh();

        std::string str = "";

        // Until the end of the string
        for (;;)
        {
            ch = stream.readCh();

            if (ch == openChar)
            {
                break;
            }

            // End of file
            else if (ch == '\0')
            {
                return new Token(
                    Token::ERROR,
                    "EOF in string literal",
                    stream.getPos()
                );
            }

            // End of line
            else if (ch == '\n')
            {
                return new Token(
                    Token::ERROR,
                    "newline in string literal",
                    stream.getPos()
                );
            }

            // Escape sequence
            else if (ch == '\\')
            {
                auto escCh = readEscape(stream);
                if (escCh != -1)
                    str += escCh;
            }

            // Normal character
            else
            {
                str += ch;
            }
        }

        return new Token(Token::STRING, str, pos);
    }

    // Quasi literal
    if (ch == '`')
    {
        // TODO: full support for quasi-literals
        // for now, quasis are only multi-line strings

        // Read the opening ` character
        auto openChar = stream.readCh();

        std::string str = "";

        // Until the end of the string
        for (;;)
        {
            ch = stream.readCh();

            if (ch == openChar)
            {
                break;
            }

            // End of file
            else if (ch == '\0')
            {
                return new Token(
                    Token::ERROR,
                    "EOF in string literal",
                    stream.getPos()
                );
            }

            // Escape sequence
            else if (ch == '\\')
            {
                auto escCh = readEscape(stream);
                if (escCh != -1)
                    str += escCh;
            }

            // Normal character
            else
            {
                str += ch;
            }
        }

        return new Token(Token::STRING, str, pos);
    }

    // End of file
    if (ch == '\0')
    {
        return new Token(Token::EOFF, pos);
    }

    // Identifier or keyword
    if (identStart(ch))
    {
        stream.readCh();
        std::string identStr;
        identStr += ch;

        for (;;)
        {
            ch = stream.peekCh();
            if (identPart(ch) == false)
                break;
            stream.readCh();
            identStr += ch;
        }

        // Try matching all keywords
        for (auto keyword : keywords)
            if (identStr == keyword)
                return new Token(Token::KEYWORD, identStr, pos);

        // Try matching all ops
        for (auto op : operators)
            if (identStr == op.str)
                return new Token(Token::OP, identStr, pos);

        return new Token(Token::IDENT, identStr, pos);
    }

    // Regular expression
    if ((flags & LEX_MAYBE_RE) && ch == '/')
    {
        // Read the opening slash
        stream.readCh();

        // Read the pattern
        std::string reStr = "";
        for (;;)
        {
            ch = stream.readCh();

            // Escape sequence
            // Note: other escape sequences are
            // handled by the regexp parser
            if (ch == '\\')
            {
                if (stream.peekCh() == '/')
                {
                    stream.readCh();
                    reStr += "\\/";
                    continue;
                }

                if (stream.peekCh() == '\\')
                {
                    stream.readCh();
                    reStr += "\\\\";
                    continue;
                }
            }

            // End of regexp literal
            if (ch == '/')
            {
                break;
            }

            // End of file
            if (ch == '\0')
            {
                return new Token(Token::ERROR, "EOF in literal", stream.getPos());
            }

            reStr += ch;
        }

        // Read the flags
        std::string reFlags = "";
        for (;;)
        {
            ch = stream.peekCh();

            if (ch != 'i' && ch != 'g' && ch != 'm' && ch != 'y')
                break;

            stream.readCh();
            reFlags += ch;
        }

        //writefln("reStr: \"%s\"", reStr);

        return new Token(Token::REGEXP, reStr, reFlags, pos);
    }

    // Try matching all separators
    for (auto sep : separators)
        if (stream.match(sep.c_str()))
            return new Token(Token::SEP, sep, pos);

    // Try matching all ops
    for (auto op : operators)
        if (stream.match(op.str.c_str()))
            return new Token(Token::OP, op.str, pos);

    // Invalid character
    assert(0);
    /*
    int charVal = stream.readCh();
    std::string charStr;
    if (charVal >= 33 && charVal <= 126)
        charStr += "'"w ~ cast(char)charVal ~ "', ";
    charStr += to!std::string(format("0x%04x", charVal));
    return new Token(
        Token::ERROR,
        "unexpected character ("w ~ charStr ~ ")", 
        pos
    );
    */
}

/**
Token stream, to simplify parsing
*/
struct TokenStream
{
    /// String stream before the next token
    StrStream* preStream;

    /// String stream after the next token
    StrStream* postStream;

    /// Flag indicating a newline occurs before the next token
    bool nlPresent;

    /// Next token to be read
    Token* nextToken;

    // Next token available flag
    bool tokenAvail;

    // Lexer flags used when reading the next token
    LexFlags lexFlags;

    /**
    Constructor to tokenize a string stream
    */
    TokenStream(StrStream* strStream) : preStream(strStream), postStream(nullptr), nlPresent(false), nextToken(nullptr), tokenAvail(false) {}

    /**
    Copy constructor for this token stream. Allows for backtracking
    */
    TokenStream(TokenStream& that)
    {
        // Copy the string streams
        preStream = that.preStream;
        postStream = that.postStream;

        nlPresent = that.nlPresent;
        nextToken = that.nextToken;
        tokenAvail = that.tokenAvail;
        lexFlags = that.lexFlags;
    }

    /**
    Method to backtrack to a previous state
    */
    void backtrack(TokenStream& that)
    {
        // Copy the string streams
        preStream = that.preStream;
        postStream = that.postStream;

        nlPresent = that.nlPresent;
        nextToken = that.nextToken;
        tokenAvail = that.tokenAvail;
        lexFlags = that.lexFlags;
    }

    SrcPos* getPos()
    {
        return preStream->getPos();
    }

    Token* peek(LexFlags lexFlags_ = 0)
    {
        if (!tokenAvail || lexFlags != lexFlags_)
        {
            postStream = preStream;
            nextToken = getToken(*postStream, lexFlags_);
            tokenAvail = true;
            lexFlags = lexFlags_;
        }

        return nextToken;
    }

    Token* read(LexFlags flags = 0)
    {
        auto t = peek(flags);

        // Cannot read the last (EOF) token
        assert (t->type != Token::EOFF ); // "cannot read final EOF token"

        // Read the token
        preStream = postStream;
        tokenAvail = false;

        // Test if a newline occurs before the new front token
        nlPresent = (peek()->pos->line > t->pos->line);

        return t;
    }

    bool newline()
    {
        return nlPresent;
    }

    bool peekKw(std::string keyword)
    {
        auto t = peek();
        return (t->type == Token::KEYWORD && t->stringVal == keyword);
    }

    bool peekSep(std::string sep)
    {
        auto t = peek();
        return (t->type == Token::SEP && t->stringVal == sep);
    }

    bool matchKw(std::string keyword)
    {
        if (peekKw(keyword) == false)
            return false;
        read();

        return true;
    }

    bool matchSep(std::string sep)
    {
        if (peekSep(sep) == false)
            return false;
        read();

        return true;
    }

    bool eof()
    {
        return peek()->type == Token::EOFF;
    }
};

} // namespace almond

