/*
 * HCLang - Highly Concurrent Language
 * Copyright (c) 2015 Miroslav Kravec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
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
        
        SHORT_DEF, LET, ASYNC, AWAIT,



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
