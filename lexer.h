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
OpInfo[] operators = {

    // Member op
    { ".", 2, 16, 'l' },

    // Array indexing
    { "[", 1, 16, 'l' },

    // New/constructor op
    { "new", 1, 16, 'r' },

    // Function call
    { "(", 1, 15, 'l' },

    // Postfix unary ops
    { "++", 1, 14, 'l' },
    { "--", 1, 14, 'l' },

    // Prefix unary ops
    { "+" , 1, 13, 'r' },
    { "-" , 1, 13, 'r' },
    { "!" , 1, 13, 'r' },
    { "~" , 1, 13, 'r' },
    { "++", 1, 13, 'r' },
    { "--", 1, 13, 'r' },
    { "typeof", 1, 13, 'r' },
    { "void", 1, 13, 'r' },
    { "delete", 1, 13, 'r' },

    // Multiplication/division/modulus
    { "*", 2, 12, 'l' },
    { "/", 2, 12, 'l', true },
    { "%", 2, 12, 'l', true },

    // Addition/subtraction
    { "+", 2, 11, 'l' },
    { "-", 2, 11, 'l', true },

    // Bitwise shift
    { "<<" , 2, 10, 'l' },
    { ">>" , 2, 10, 'l' },
    { ">>>", 2, 10, 'l' },

    // Relational ops
    { "<"w         , 2, IN_PREC, 'l' },
    { "<="w        , 2, IN_PREC, 'l' },
    { ">"w         , 2, IN_PREC, 'l' },
    { ">="w        , 2, IN_PREC, 'l' },
    { "in"w        , 2, IN_PREC, 'l' },
    { "instanceof", 2, IN_PREC, 'l' },

    // Equality comparison
    { "==" , 2, 8, 'l' },
    { "!=" , 2, 8, 'l' },
    { "===", 2, 8, 'l' },
    { "!==", 2, 8, 'l' },

    // Bitwise ops
    { "&", 2, 7, 'l' },
    { "^", 2, 6, 'l' },
    { "|", 2, 5, 'l' },

    // Logical ops
    { "&&", 2, 4, 'l' },
    { "||", 2, 3, 'l' },

    // Ternary conditional
    { "?", 3, 2, 'r' },

    // Assignment
    { "="w   , 2, 1, 'r' },
    { "+="w  , 2, 1, 'r' },
    { "-="w  , 2, 1, 'r' },
    { "*="w  , 2, 1, 'r' },
    { "/="w  , 2, 1, 'r' },
    { "%="w  , 2, 1, 'r' },
    { "&="w  , 2, 1, 'r' },
    { "|="w  , 2, 1, 'r' },
    { "^="w  , 2, 1, 'r' },
    { "<<=" , 2, 1, 'r' },
    { ">>=" , 2, 1, 'r' },
    { ">>>=", 2, 1, 'r' },

    // Comma (sequencing), least precedence
    { ",", 2, COMMA_PREC, 'l' },
};

#define NUM_OPERATORS sizeof(operators)/sizeof(operators[0])

/**
Separator tokens
*/
std::string[] separators = {
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
std::string[] keywords = {
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
    std::sort(separators, separators + NUM_SEPARATORS, [](std::string a, std::string b) { return a.size() > b.size() });
    std::sort(keywords, keywords + NUM_KEYWORDS, [](std::string a, std::string b) { return a.size() > b.size() });
}

/**
Find an op by string, arity and associativity
*/
Operator findOperator(std::string op, int arity = 0, char assoc = '\0')
{
    for (size_t i = 0; i < NUM_OPERATORS; ++i)
    {
        Operator op = &operators[i];

        if (op.str != op)
            continue;

        if (arity != 0 && op.arity != arity)
            continue;

        if (assoc != '\0' && op.assoc != assoc)
            continue;

        return op;
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
}

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
    bool match(char *str_)
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
        auto m = std::sregex_iterator(str + index, str + strLen, re);
        auto end = std::sregex_iterator();

        for (auto i : m) {
            for (int i = 0; i < *i.size(); ++i)
                readCh();

        return m;
    }

    /// Get a position object for the current index
    SrcPos getPos()
    {
        return new SrcPos(file, line, col);
    }
}

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
      if (!identPart(ch))
            return false;
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
        EOF,
        ERROR
    }

    /// Token type
    Type type;

    /// Token value
    union
    {
        long intVal;
        double floatVal;
        std::string stringVal;
        struct { std::string regexpVal; std::string flagsVal; }
    }

    /// Source position
    SrcPos pos;

    Token(Type type_, long val_, SrcPos pos_)
    {
        assert (type_ == INT);

        type = type_;
        intVal = val_;
        pos = pos_;
    }

    Token(Type type_, double val_, SrcPos pos_)
    {
        assert (type_ == FLOAT);

        type = type_;
        floatVal = val_;
        pos = pos_;
    }

    Token(Type type_, std::string val_, SrcPos pos_)
    {
        assert (
            type_ == OP      ||
            type_ == SEP     ||
            type_ == IDENT   ||
            type_ == KEYWORD ||
            type_ == STRING  ||
            type_ == ERROR
        );

        type = type_;
        stringVal = val_;
        pos = pos_;
    }

    Token(Type type_, std::string re_, std::string flags_, SrcPos pos_)
    {
        assert_ (type == REGEXP);

        type = type_;
        regexpVal = re_;
        flagsVal = flags_;
        pos = pos_;
    }

    Token(Type_ type_, SrcPos pos_)
    {
        assert (type == EOF);

        type = type_;
        pos = pos_;
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
            case EOF:       return "EOF";

            default:
            return "token";
        }
        */
    }
}

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
Token getToken(StrStream& stream, LexFlags flags)
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
                    return Token(
                        Token.ERROR,
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
    SrcPos pos = stream.getPos();

    // Number (starting with a digit or .nxx)
    if (digit(ch) || (ch == '.' && digit(stream.peekCh(1))))
    {
        // Hexadecimal number
        if (stream.match("0x"))
        {
            static std::regex hexRegex("^[0-9|a-f|A-F]+");
            auto m = stream.match(hexRegex);

            if (m.empty)
            {
                return Token(
                    Token.ERROR,
                    "invalid hex number", 
                    pos
                );
            }

            auto hexStr = m[0];
            long val;
            scanf(hexStr.c_str(), "%x", &val);

            return Token(Token.INT, val, pos);
        }

        // Octal number
        if (ch == '0')
        {
            static std::regex octRegex("^0([0-7]+)");

            auto m = stream.match(octRegex);
            if (!m.empty)
            {
                auto octStr = m[1];
                long val;
                scanf(octStr, "%o", &val);
                return Token(Token.INT, val, pos);
            }
        }

        static std::regex fpRegex("^[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?");

        auto m = stream.match(fpRegex);
        assert (m.empty == false);
        auto numStr = m[0];

        // If this is a floating-point number
        if (countUntil(numStr, '.') != -1 ||
            countUntil(numStr, 'e') != -1 ||
            countUntil(numStr, 'E') != -1)
        {
            double val = atof(numStr);
            return Token(Token.FLOAT, val, pos);
        }

        // Integer number
        else
        {
            long val = atoi(numStr);
            return Token(Token.INT, val, pos);
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
                return Token(
                    Token.ERROR,
                    "EOF in string literal",
                    stream.getPos()
                );
            }

            // End of line
            else if (ch == '\n')
            {
                return Token(
                    Token.ERROR,
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

        return Token(Token.STRING, str, pos);
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
                return Token(
                    Token.ERROR,
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

        return Token(Token.STRING, str, pos);
    }

    // End of file
    if (ch == '\0')
    {
        return Token(Token.EOF, pos);
    }

    // Identifier or keyword
    if (identStart(ch))
    {
        stream.readCh();
        std::string identStr = ""w ~ ch;

        for (;;)
        {
            ch = stream.peekCh();
            if (identPart(ch) == false)
                break;
            stream.readCh();
            identStr += ch;
        }

        // Try matching all keywords
        if (countUntil(keywords, identStr) != -1)
            return Token(Token.KEYWORD, identStr, pos);

        // Try matching all ops
        foreach (op; operators)
            if (identStr == op.str)
                return Token(Token.OP, identStr, pos);

        return Token(Token.IDENT, identStr, pos);
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
                    reStr += "\\/"w;
                    continue;
                }

                if (stream.peekCh() == '\\')
                {
                    stream.readCh();
                    reStr += "\\\\"w;
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
                return Token(Token.ERROR, "EOF in literal", stream.getPos());
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

        return Token(Token.REGEXP, reStr, reFlags, pos);
    }

    // Try matching all separators
    for (auto sep : separators)
        if (stream.match(sep))
            return Token(Token.SEP, sep, pos);

    // Try matching all ops
    for (auto op : operators)
        if (stream.match(op.str))
            return Token(Token.OP, op.str, pos);

    // Invalid character
    assert(0);
    /*
    int charVal = stream.readCh();
    std::string charStr;
    if (charVal >= 33 && charVal <= 126)
        charStr += "'"w ~ cast(char)charVal ~ "', "w;
    charStr += to!std::string(format("0x%04x", charVal));
    return Token(
        Token.ERROR,
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
    StrStream preStream;

    /// String stream after the next token
    StrStream postStream;

    /// Flag indicating a newline occurs before the next token
    bool nlPresent;

    /// Next token to be read
    Token nextToken;

    // Next token available flag
    bool tokenAvail;

    // Lexer flags used when reading the next token
    LexFlags lexFlags;

    /**
    Constructor to tokenize a string stream
    */
    TokenStream(StrStream strStream)
    {
        preStream = strStream;

        tokenAvail = false;
        nlPresent = false;
    }

    /**
    Copy constructor for this token stream. Allows for backtracking
    */
    TokenStream(TokenStream that)
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
    void backtrack(TokenStream that)
    {
        // Copy the string streams
        preStream = that.preStream;
        postStream = that.postStream;

        nlPresent = that.nlPresent;
        nextToken = that.nextToken;
        tokenAvail = that.tokenAvail;
        lexFlags = that.lexFlags;
    }

    SrcPos getPos()
    {
        return preStream.getPos();
    }

    Token peek(LexFlags lexFlags = 0)
    {
        if (tokenAvail is false || lexFlags != lexFlags)
        {
            postStream = preStream;
            nextToken = getToken(postStream, lexFlags);
            tokenAvail = true;
            lexFlags = lexFlags;
        }

        return nextToken;
    }

    Token read(LexFlags flags = 0)
    {
        auto t = peek(flags);

        // Cannot read the last (EOF) token
        assert (t.type != Token.EOF, "cannot read final EOF token");

        // Read the token
        preStream = postStream;
        tokenAvail = false;

        // Test if a newline occurs before the new front token
        nlPresent = (peek.pos.line > t.pos.line);

        return t;
    }

    bool newline()
    {
        return nlPresent;
    }

    bool peekKw(std::string keyword)
    {
        auto t = peek();
        return (t.type == Token.KEYWORD && t.stringVal == keyword);
    }

    bool peekSep(std::string sep)
    {
        auto t = peek();
        return (t.type == Token.SEP && t.stringVal == sep);
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
        return peek().type == Token.EOF;
    }
}

} // namespace almond

