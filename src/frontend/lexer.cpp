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
#include <cstdlib>
#include <cstdio>
#include <assert.h>

#include "lexer.h"

using namespace std;

struct State {
    bool new_line = true;
    int indent = 0;
    int bracket_depth = 0;
    int remaining_deindent = 0;
    int line = 0;
    int position = 0;
};


inline
Token symbolToken(State &state, Token::Type type)
{
    return { state.line, state.position, type, "", 0 };
}

inline
Token symbolToken(State &state, int type)
{
    return { state.line, state.position, (Token::Type)type, "", 0 };
}

static
void addSymbolToken(map<Token::Type, const char *> &n, const char *symbol)
{
    n[(Token::Type)symbol[0]] = symbol;
}

static
map<Token::Type, const char *> _tokenNames()
{
    map<Token::Type, const char *> n;
    n[Token::AND] = "AND";
    n[Token::MATCH] = "match";
    n[Token::CASE] = "case";

    n[Token::PLUS] = "+";
    n[Token::MINUS] = "-";
    n[Token::MULTIPLY] = "*";
    n[Token::DIVIDE] = "/";
    n[Token::COLON] = ":";
    n[Token::EQUAL] = "==";
    n[Token::NEQUAL] = "<>";
    n[Token::LESS] = "<";
    n[Token::BITWISE_OR] = "<";

    n[Token::VAR] = "VAR";
    n[Token::IF] = "IF";
    n[Token::ELSE] = "ELSE";
    n[Token::THEN] = "THEN";
    n[Token::ASSIGN] = "=";
    n[Token::PROCEDURE] = "procedure";
    n[Token::FUNCTION] = "fn";
    n[Token::IDENTIFIER] = "identifier";
    n[Token::INDENT] = "indent";
    n[Token::DEINDENT] = "deindent";
    n[Token::FROM] = "from";
    n[Token::IMPORT] = "import";
    n[Token::NEWLINE] = "\\n";
    n[Token::USING] = "using";
    n[Token::AS] = "as";
    n[Token::SLOT] = "slot";
    n[Token::RETURN] = "return";
    n[Token::IN] = "in";
    n[Token::SYSTEM] = "system";
    n[Token::SPAWN] = "spawn";
    n[Token::SHORT_DEF] = "=>";
    n[Token::TYPE] = "type";
    n[Token::LET] = "let";
    n[Token::ASYNC] = "async";
    n[Token::AWAIT] = "await";


    addSymbolToken(n,";");
    addSymbolToken(n,",");
    addSymbolToken(n,"[");
    addSymbolToken(n,"]");
    addSymbolToken(n,"(");
    addSymbolToken(n,")");
    addSymbolToken(n,"!");

    return n;
};

std::map<Token::Type, const char *> tokenNames = _tokenNames();

std::string getTokenTypeName(Token::Type t) {
    auto it = tokenNames.find(t);
    
    if( t == Token::IDENTIFIER ) {
        return "ID";
    } else if( t == Token::INTEGER ) {
        return "INTEGER";
    } else if( it != tokenNames.end() ) {
        return std::string(it->second);
    }
    return std::string("");
}

void printToken(const Token &t) {
    auto it = tokenNames.find(t.type);
    
    if( t.type == Token::IDENTIFIER ) {
        cout << "[ID]" << t.str_val;
    } else if( t.type == Token::INTEGER ) {
        cout << "[INTEGER]" << t.int_val;
    } else if( it != tokenNames.end() ) {
        cout << "[" << it->second << "]";
    }
}

static
map<string,Token::Type> stringTokens()
{
    map<string,Token::Type> m;
    m["procedure"] = Token::PROCEDURE;
    m["match"] = Token::MATCH;
    m["var"] = Token::VAR;
    m["if"] = Token::IF;
    m["else"] = Token::ELSE;
    m["then"] = Token::THEN;
    m["integer"] = Token::INTEGER;
    m["for"] = Token::FOR;
    m["mod"] = Token::MOD;
    m["fn"] = Token::FUNCTION;
    m["from"] = Token::FROM;
    m["import"] = Token::IMPORT;
    m["using"] = Token::USING;
    m["as"] = Token::AS;
    m["slot"] = Token::SLOT;
    m["return"] = Token::RETURN;
    m["in"] = Token::IN;
    m["system"] = Token::SYSTEM;
    m["spawn"] = Token::SPAWN;
    m["elif"] = Token::ELIF;
    m["type"] = Token::TYPE;
    m["case"] = Token::CASE;
    m["let"] = Token::LET;
    m["async"] = Token::ASYNC;
    m["await"] = Token::AWAIT;

    return m;
}
static Token nextToken(State &state, const char *&ch)
{
    std::string str;
    int integer, indent = 0;
    const char *start = ch;

    static auto str_tokens = stringTokens();

Q_START:
    if(state.remaining_deindent) {
        state.remaining_deindent -= 1;
        return { state.line, state.position, Token::DEINDENT};
    }
    if(state.new_line) {
        state.new_line = false;
        goto Q_NEWLINE;
    }

    while( *ch==' ' || (state.bracket_depth > 0 && *ch == '\n') ) {
        if( *ch == '\n' ) {
            state.line += 1;
            state.position = 0;
        } else {
            state.position += 1;
        }
        ch++;
    }

    if(*ch == '\n') {
        ch++; 
        state.line += 1;
        state.position = 0;
        state.new_line = true;
        return { state.line, state.position, Token::NEWLINE };
    }
    goto Q_CONTINUE;

Q_NEWLINE:
    indent = 0;
    while( *ch==' ' ) {
        indent++;
        state.position += 1;
        ch++;
    }
    if( *ch == '\n' ) {
        ch++;
        state.line += 1;
        state.position = 0;
        goto Q_NEWLINE;
    }
    if(indent < state.indent) {
        state.remaining_deindent = (state.indent - indent)/4 - 1;
        state.indent = indent;
        return { state.line, state.position, Token::DEINDENT };
    }
    if( indent > state.indent ) {
        state.indent = indent;
        return { state.line, state.position, Token::INDENT };
    }

Q_CONTINUE:
    if( !*ch ) {
        state.remaining_deindent = state.indent/4;
        state.indent = 0;
        if(state.remaining_deindent > 0)
            goto Q_START;
        return { state.line, state.position, Token::EOI };
    }

    switch( *ch ) {
    case '#':
        while(*(++ch) != '\n' && *ch );
        ch++;
        goto Q_NEWLINE;
    case '<':
        ch++;
        if( *ch == '>') {
            ch++;
            state.position += ch - start;
            return { state.line, state.position, Token::NEQUAL };
        }
        if( *ch == '=') {
            ch++;
            state.position += ch - start;
            return { state.line, state.position,Token::LEQUAL };
        }
        state.position += ch - start;
        return symbolToken( state, '<' );
    case '{':
        goto Q_COMMENT;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        goto Q_INTEGER;
    case '0':
        goto Q_INTEGER8;
    case ':':
        ch++;
        return symbolToken( state, ':');
    case '>':
        ch++;
        if( *ch == '=') {
            ch++;
            state.position += ch - start;
            return { state.line, state.position, Token::GEQUAL };
        }
        state.position += ch - start;
        return { state.line, state.position, Token::GREATER };
    case '/':
        if( *(ch+1) == '/' ) goto Q_COMMENT;
    case '+':
    case '-':
    case '*':
    case ',':
    case '!':
    case '|':
        state.position += ch - start + 1;
        return symbolToken ( state, (Token::Type) *(ch++) );
    case '=':
        if( *(ch+1) == '>'){
            ch+=2;
            return { state.line, state.position, Token::SHORT_DEF };
        }
        state.position += ch - start + 1;
        return symbolToken ( state, (Token::Type) *(ch++) );
    case ')':
    case ']':
        state.bracket_depth -= 1;
        state.position += ch - start + 1;
        return symbolToken ( state, (Token::Type) *(ch++) );
    case '(':
    case '[':
        state.bracket_depth += 1;
        state.position += ch - start + 1;
        return symbolToken ( state, *(ch++) );
    case ';':
        //ch++;
        state.position += ch - start;
        return symbolToken( state, Token::SEMICOLON );
    case '.':
        ch++;
        if(*ch == '.') {
            ch++;
            state.position += ch - start;
            return symbolToken( state, Token::DOUBLE_DOT );
        }
        state.position += ch - start;
        return symbolToken( state, '.');
    case '"':
        goto Q_STRING;
    default:
        goto Q_TRY_INDENTIFIER;
    }

Q_COMMENT:
    ch += 2;
    while( *ch != '\n')ch++;
    goto Q_START;

Q_INTEGER:
    integer = *ch - '0';
    ch++;
    while( *ch >= '0' && *ch <= '9' ) {
        integer = integer * 10 + *ch - '0';
        ch++;
    }
    state.position += ch - start;
    return { state.line, state.position, Token::INTEGER, "", integer };

Q_INTEGER8:
    integer = *ch - '0';
    ch++;
    if( *ch == 'x' )
        goto Q_INTEGER16;
    while( *ch >= '0' && *ch <= '9' ) {
        integer = integer * 8 + *ch - '0';
        ch++;
    }
    state.position += ch - start;
    return { state.line, state.position, Token::INTEGER, "", integer };

Q_INTEGER16:
    integer = 0;
Q_INTEGER16_CONTINUE:
    ch++;
    if( *ch >= '0' && *ch <= '9' ) {
        integer = integer * 16 + *ch - '0';
        goto Q_INTEGER16_CONTINUE;
    }
    if( *ch >= 'a' && *ch <= 'f' ) {
        integer = integer * 16 + *ch - 'a' + 10;
        goto Q_INTEGER16_CONTINUE;
    }
    if( *ch >= 'A' && *ch <= 'F' ) {
        integer = integer * 16 + *ch - 'A' + 10;
        goto Q_INTEGER16_CONTINUE;
    }
    state.position += ch - start;
    return { state.line, state.position, Token::INTEGER, "", integer };

Q_STRING:
    str = "";
    ch++;
    while( *ch && *ch != '"' ) {
        str += *ch;
        ch++;
    }
    ch++;
    state.position += ch - start;
    return { state.line, state.position, Token::STRING, str };

Q_TRY_INDENTIFIER:
    if(isalpha(*ch) || *ch == '_')
        str += *ch;

Q_IDENTIFIER:
    ch++;
    if( isalnum(*ch) || *ch == '_' ) {
        str += *ch;
        goto Q_IDENTIFIER;
    }

    {
        auto it = str_tokens.find(str);
        state.position += ch - start;
        if( it == str_tokens.end() )
            return { state.line, state.position, Token::IDENTIFIER, str, 0 };
        else
            return { state.line, state.position, it->second, "", 0 };
    }
}



std::vector<Token> lexerBuf(const char *data, int len)
{
    State state;
    std::vector<Token> tokens;
    const char *end = data + len;
    const char *prev = data - 1;
    while(tokens.empty() || tokens.back().type != Token::EOI) {
        if( data > end ) {
            exit(1);
        }
        if( prev == data && tokens.back().type != Token::DEINDENT ) {
            //exit(2);
            fprintf(stderr,"Not consumed\n");
            break; // TODO
        }
        prev = data;
        tokens.push_back(nextToken(state,data));
    }
    //tokens.push_back({Token::EOI});
    return tokens;
}


std::vector<Token> lexerFile(const char *fname)
{
    FILE *f = fopen(fname, "rb");

    assert(f && "Can't open file");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = new char[fsize + 1];
    fread(data, fsize, 1, f);
    fclose(f);
    data[fsize] = 0;

    std::vector<Token> tokens = lexerBuf(data,fsize);

    delete []data;
    return tokens;
}
