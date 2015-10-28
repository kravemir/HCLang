#ifndef HCLANG_LEXER
#define HCLANG_LEXER

#include <iostream>
#include <vector>
#include <map>

struct Token {
    enum Type {
        // syntactic tokens
        COLON = ':', SEMICOLON = ';', COMMA = ',', DOT = '.',
        OPEN_BRACKET = '[', CLOSE_BRACKET = ']',
        OPEN_PARENTHESIS = '(', CLOSE_PARENTHESIS = ')',
        EXCLAMATION_MARK = '!', BITWISE_OR = '|',

        // expresions / binary operations
        PLUS = '+', MINUS = '-', MULTIPLY = '*', DIVIDE = '/',

        // expressions / statements
        ASSIGN = '=', 

        // comparison
        LESS = '<', GREATER = '>', 

        // base structural elements
        EOI = 256, UNKNOWN, 
        NEWLINE, INDENT, DEINDENT,

        IDENTIFIER,

        DOUBLE_DOT, CASE,



        // manipulation elements
        FROM, IMPORT, AS, MATCH,

        // declaration elements
        FUNCTION, SYSTEM, GROUP, SLOT, SPAWN, TYPE,

        // logical
        AND, 
        
        // operations / commands
        RETURN, 
        
        SHORT_DEF, LET,



        // comparison
        EQUAL, NEQUAL, LEQUAL, GEQUAL,

        IF, ELSE, THEN, ELIF,

        FOR, MOD, IN, VAR, PROCEDURE,
        USING, 

        // constants
        INTEGER, STRING
    };

    int line, position;
    Type type;
    std::string str_val;
    int int_val;
};


std::vector<Token> lexerBuf(const char *data, int len);
std::vector<Token> lexerFile(const char *fname);
void printToken(const Token &t);
std::string getTokenTypeName(Token::Type t);


#endif // HCLANG_LEXER
