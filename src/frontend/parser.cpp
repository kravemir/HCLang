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
#include "parser.h"

#include <stdio.h>

#include <ast/ast.h>

Parser::Parser(const std::vector<Token> &tokens, std::ostream *out):
    tokens(tokens),
    _current_idx(0),
    out(out)
{
}

void Parser::consume() {
    _current_idx += 1;
}

void Parser::errorExpansion(std::string what) {
    std::cerr << "Expansion error [" << current().line << ", " << current().position << "]: " << what << std::endl;
}

const Token& Parser::current(int offset) {
    return tokens[_current_idx + offset];
}
const Token& Parser::currentConsume() {
    return tokens[_current_idx++];
}

Token::Type Parser::type(int offset) {
    return tokens[_current_idx + offset].type;
}

bool Parser::expect(Token::Type type) {
    if( this->type() == type ) {
        return true;
    } else {
        errorExpansion("expected " + getTokenTypeName(type) + ", got " + getTokenTypeName(this->type()));
        return false;
    }
}
bool Parser::expectConsume(Token::Type type) {
    if( expect(type) ) {
        consume();
        return true;
    } else {
        return false;
    }
}
bool Parser::tryConsume(Token::Type type) {
    if( this->type() == type ) {
        consume();
        return true;
    } else {
        return false;
    }
}

StatementList* Parser::file() {
    Statement *s = 0;
    StatementList *stmts = new StatementList;
    while( ( s = statement()) != 0 ) {
        stmts->push_back(s);
        if( type() == Token::EOI )
            break;
    }
    return stmts;
}

Statement* Parser::statement(){
    switch(type()) {
    case Token::IDENTIFIER:
        return pathPrefixStatement();
    case Token::LET:
        return letStatement();
    case Token::SYSTEM:
        return systemDecl();
    case Token::FUNCTION:
        return functionDecl();
    case Token::PROCEDURE:
        return procedureDecl();
    case Token::SLOT:
        return slotDecl();
    case Token::VAR:
        return varStatement();
    case Token::IF:
        return condStatement();
    case Token::TYPE:
        return typeStatement();
    case Token::FOR:
        return forStatement();
    case Token::RETURN:
        return returnStatement();
    default:
        //errorExpansion("Unexpected token at expasion of statement");
        {
            MValueAST *val = value();
            if(!val || !expectConsume(Token::NEWLINE)) return 0;
            return new ExprStmt(val);
        }
        return 0;
    }
}

ReturnStmt* Parser::returnStatement() {
    if( !expectConsume(Token::RETURN) ) return 0;
    MValueAST *val = value();
    if( !expectConsume(Token::NEWLINE) ) return 0;
    return new ReturnStmt(val);
    
}

TypeDecl* Parser::typeStatement() {
    if( !expectConsume(Token::TYPE) | !expect(Token::IDENTIFIER) ) return 0;
    std::string name = currentConsume().str_val;
    expectConsume(Token::ASSIGN);
    MTypeAST *type = typeDecl();
    expectConsume(Token::NEWLINE);
    return new TypeDecl(name,type);
}

Statement* Parser::pathPrefixStatement(){
    Path p = path();
    switch(type()){
    case Token::ASSIGN:
        return bindStatement(p);
    case Token::EXCLAMATION_MARK:
        return sendStatement(p);
    case Token::OPEN_PARENTHESIS: // TODO: use atom function
        {
            TupleAST *call = tuple();
            expectConsume(Token::NEWLINE);
            MValueAST *val = new VarExpr(p[0]);
            for(size_t i = 1; i < p.size(); i++)
                val = new GetChildAST(val,p[i]);
            return new ExprStmt(new CallExpr(val,call));
        }
    default:
        errorExpansion("Unexpected token at expasion of path prefix statement");
        return 0;
    }
}

Statement* Parser::letStatement() {
    if(!expectConsume(Token::LET)) return 0;
    Path p = path();
    MValueAST*val;
    MTypeAST* letType = 0; 

    if( tryConsume(Token::COLON) ) {
        letType = typeDecl();
    }

    if(!expectConsume(Token::ASSIGN)) return 0;

    val = value();
    expectConsume(Token::NEWLINE);

    return new LetStmt(p, letType, val);
}

Statement*  Parser::bindStatement(Path target) {
    MValueAST*val;

    if(!expectConsume(Token::ASSIGN)) return 0;

    switch(type()){ 
        case Token::MATCH:
            return matchAssignStatement(target);
        default:
            val = value();
            expectConsume(Token::NEWLINE);
            return new BindStmt(target, val);
    }


}

MatchAssignStmt* Parser::matchAssignStatement(Path target) {
    if(!expectConsume(Token::MATCH)) return 0;
    MValueAST *srcVal = value();
    if( !expectConsume(Token::COLON)
            || !expectConsume(Token::NEWLINE)
            || !expectConsume(Token::INDENT) )
        return 0;

    std::vector<std::pair<std::string,MValueAST*>> cases;

    while(tryConsume(Token::CASE)) {
        if(!expect(Token::IDENTIFIER)) return 0;
        std::string match_str = currentConsume().str_val;
        if(!expectConsume(Token::SHORT_DEF)) return 0;
        MValueAST *resVal = value();
        if(!expectConsume(Token::NEWLINE)) return 0;
        cases.push_back({match_str,resVal});
    }

    if(!expectConsume(Token::DEINDENT)) return 0;

    return new MatchAssignStmt(target,srcVal,cases);
}

SystemDecl* Parser::systemDecl() {
    if(!expectConsume(Token::SYSTEM) || !expect(Token::IDENTIFIER)) return 0;

    std::string name = currentConsume().str_val;

    if( !expectConsume(Token::COLON) 
        || !expectConsume(Token::NEWLINE)
        || !expectConsume(Token::INDENT))
        return 0;

    Statement *s = 0;
    StatementList *stmts = new StatementList;
    while( ( s = statement()) != 0 ) {
        stmts->push_back(s);
        if( tryConsume(Token::DEINDENT) )
            break;
    }

    return new SystemDecl( name, stmts);
}

VarDecl*   Parser::varStatement() {
    if( !expectConsume(Token::VAR)
        || !expect(Token::IDENTIFIER) ) return 0;
    std::string name = currentConsume().str_val;
    MTypeAST *type = 0;
    MValueAST *val = 0;

    if( tryConsume( Token::COLON ))
        type = typeDecl();
    if( tryConsume(Token::ASSIGN))
        val = value();

    expectConsume(Token::NEWLINE);

    return new VarDecl(name,type,val);
}

Path Parser::path() {
    Path p;
Q_NEXT:
    expect(Token::IDENTIFIER);
    p.push_back(currentConsume().str_val);
    if(tryConsume(Token::DOT))
        goto Q_NEXT;

    return p;
}

FunctionDecl*    Parser::functionDecl() {
    if(!expectConsume(Token::FUNCTION) || !expect(Token::IDENTIFIER )) return 0;

    std::string name = currentConsume().str_val;
    MTupleTypeAST *args = argsDecl();
    MTypeAST *t = 0;

    switch(type()){
        case Token::SHORT_DEF:
            goto Q_SHORT_DEF;
        case Token::COLON:
            goto Q_BLOCK_DEF;
        default:
            t = typeDecl();
            break;
    }

    switch(type()){
        case Token::SHORT_DEF:
            goto Q_SHORT_DEF;
        case Token::COLON:
            goto Q_BLOCK_DEF;
        default:
            //error("expansion TODO");
            return 0;
    }

Q_SHORT_DEF:
    {
        std::cerr << "Short def\n";
        expectConsume(Token::SHORT_DEF);
        MValueAST* val = value();
        StatementList *stmts = new StatementList;
        stmts->push_back({new ReturnStmt(val)});
        expectConsume(Token::NEWLINE);
        
        return new FunctionDecl(name, args, stmts, t);
    }

Q_BLOCK_DEF:
    {
        if( !expectConsume(Token::COLON) 
            || !expectConsume(Token::NEWLINE)
            || !expectConsume(Token::INDENT))
            return 0;


        Statement *s = 0;
        StatementList *stmts = new StatementList;
        while( ( s = statement()) != 0 ) {
            stmts->push_back(s);
            if( tryConsume(Token::DEINDENT) )
                break;
        }

        return new FunctionDecl( name, args, stmts, t);
    }
}

SlotDecl*        Parser::slotDecl() {
    if(!expectConsume(Token::SLOT) || !expect(Token::IDENTIFIER )) return 0;

    std::string name = currentConsume().str_val;
    MTupleTypeAST *args = argsDecl();
    MTypeAST *type = 0;

    if( this->type() != Token::COLON )
        type = typeDecl();

    if( !expectConsume(Token::COLON) 
        || !expectConsume(Token::NEWLINE)
        || !expectConsume(Token::INDENT))
        return 0;


    Statement *s = 0;
    StatementList *stmts = new StatementList;
    while( ( s = statement()) != 0 ) {
        stmts->push_back(s);
        if( tryConsume(Token::DEINDENT) )
            break;
    }

    return new SlotDecl( name, args, type, stmts);
}

ProcedureDecl*        Parser::procedureDecl() {
    if(!expectConsume(Token::PROCEDURE) || !expect(Token::IDENTIFIER )) return 0;

    std::string name = currentConsume().str_val;
    MTupleTypeAST *args = argsDecl();
    MTypeAST *type = 0;

    bool async = false;
    if( this->type() != Token::COLON ) {
        if(tryConsume(Token::ASYNC)) {
            async = true;
        } else {
            type = typeDecl();
            async = tryConsume(Token::ASYNC);
        }
    }


    if( !expectConsume(Token::COLON) 
        || !expectConsume(Token::NEWLINE)
        || !expectConsume(Token::INDENT))
        return 0;


    Statement *s = 0;
    StatementList *stmts = new StatementList;
    while( ( s = statement()) != 0 ) {
        stmts->push_back(s);
        if( tryConsume(Token::DEINDENT) )
            break;
    }

    if(!type) type = new MVoidTypeAST();
    return new ProcedureDecl( name, args, type, stmts, async);
}

MTupleTypeAST*    Parser::argsDecl() {
    return tupleTypeDecl();
}

SendStmt*  Parser::sendStatement(Path target) {
    if( !expectConsume(Token::EXCLAMATION_MARK) ) return 0;

    std::string msg;
    if( type() == Token::IDENTIFIER )
        msg = currentConsume().str_val;
    
    TupleAST    *call = tuple();

    expectConsume(Token::NEWLINE);

    return new SendStmt( target, msg, call );
}

MValueAST*   Parser::value() {
    switch(type()){
        case Token::SPAWN:
            return spawnExpr();
        case Token::AWAIT:
            consume();
            // TODO
            return condExpr();
        default:
            return condExpr();
    }
}

SpawnExpr*  Parser::spawnExpr() {
    if(!expectConsume(Token::SPAWN) || !expect(Token::IDENTIFIER)) return 0;
    std::string str = currentConsume().str_val;
    TupleAST *tuple = 0;
    if(type() == Token::OPEN_PARENTHESIS)
        tuple = this->tuple();
    return new SpawnExpr(str,tuple);
}

MValueAST* Parser::condExpr() {
    if(tryConsume(Token::IF)){
        MValueAST *cond = comparison();
        expectConsume(Token::THEN);
        MValueAST * then = comparison();
        expectConsume(Token::ELSE);
        MValueAST *e = condExpr();
        return new CondExpr(cond,then,e);
    }
    return comparison();
}

MValueAST*  Parser::comparison() {
    MValueAST *v = plus();
    Token::Type t;
Q_CMP:
    switch(t = type()) {
        case Token::LESS:
        case Token::GREATER:
            consume();
            v = new BinaryOpAST(t, v, plus());
            break;
        default:
            return v;
    }
    goto Q_CMP;
}

MValueAST*   Parser::plus() {
    MValueAST* ast = mult();
Q_START:
    if(tryConsume(Token::PLUS)) {
        ast = new BinaryOpAST(Token::PLUS,ast, mult());
        goto Q_START;
    }
    if(tryConsume(Token::MINUS)) {
        ast = new BinaryOpAST(Token::MINUS,ast, mult());
        goto Q_START;
    }
    return ast;
}

MValueAST*   Parser::mult() {
    MValueAST* ast = atom_expr();
    Q_START:
    if(tryConsume(Token::MULTIPLY)) {
        ast = new BinaryOpAST(Token::MULTIPLY,ast, atom_expr());
        goto Q_START;
    }
    if(tryConsume(Token::DIVIDE)) {
        ast = new BinaryOpAST(Token::DIVIDE,ast, atom_expr());
        goto Q_START;
    }
    return ast;
}

MValueAST*  Parser::atom_expr() {
    MValueAST *val = atom();

Q_TRAILER:
    switch(type()) {
        case Token::OPEN_PARENTHESIS:
            {
                TupleAST    *call = tuple();
                val = new CallExpr(val,call);
                break;
            }
      /*  case Token::LEFT_BRACKET:
            return 0; // TODO*/
        case Token::DOT:
            consume();
            val = new GetChildAST(val,currentConsume().str_val); // TODO
            goto Q_TRAILER;
        default:
            goto Q_END;
    }
    goto Q_TRAILER;

Q_END:
    if(tryConsume(Token::EXCLAMATION_MARK)) {
        consume();
        tuple();
    }
    return val;
}

MValueAST*   Parser::atom() {
    switch(type()) {
    case Token::STRING:
        return new StringAST( currentConsume().str_val );
    case Token::INTEGER:
        return new IntegerAST( currentConsume().int_val );
    case Token::IDENTIFIER:
        return new VarExpr( currentConsume().str_val );
    case Token::OPEN_BRACKET:
        return array();
    default:;
            // TODO ERROR
    }

    return 0;
}

ArrayAST*   Parser::array() {
    if( !expectConsume(Token::OPEN_BRACKET))
        return 0;

    MValueList *values = new MValueList;
    MValueAST *val;
    if( type() != Token::CLOSE_BRACKET) 
        while( (val = value()) != 0 ) {
            values->push_back(val);
            if(!tryConsume(Token::COMMA)) break;
        }

    if( !expectConsume(Token::CLOSE_BRACKET))
        return 0;

    return new ArrayAST(values);
};

TupleAST*   Parser::tuple() {
    if( !expectConsume(Token::OPEN_PARENTHESIS))
        return 0;

    MValueList *values = new MValueList;
    MValueAST *val;
    if( type() != Token::CLOSE_PARENTHESIS) 
        while( (val = value()) != 0 ) {
            values->push_back(val);
            if( type() == Token::CLOSE_PARENTHESIS) break;
            expectConsume(Token::COMMA);
        }

    if( !expectConsume(Token::CLOSE_PARENTHESIS))
        return 0;

    return new TupleAST(values,0);
};

MTypeAST*    Parser::typeDecl() {
    return unionTypeDecl();
}
MTypeAST*   Parser::baseTypeDecl() {
    MTypeAST *type;
    switch(this->type()){
        case Token::OPEN_PARENTHESIS:
            type = tupleTypeDecl(); break;
        case Token::IDENTIFIER:
            type = new MNameTypeAST(currentConsume().str_val);break;
        case Token::SLOT:
            type = slotTypeDecl(); break;
        default:;
            // TODO error
    };

Q_TRAILER:
    switch(this->type()){
        case Token::OPEN_BRACKET:
            type = arrayTypeTrailer(type); break;
        default:
            return type;
    };
    goto Q_TRAILER;
}

SlotTypeAST* Parser::slotTypeDecl() {
    if(!expectConsume(Token::SLOT)) return 0;
    MTupleTypeAST *tuple = tupleTypeDecl();
    if(!tuple) return 0;
    return new SlotTypeAST(tuple, 0); // TODO: parse Return type
}

MTypeAST* Parser::unionTypeDecl() {
    std::string name;
    MTypeAST *type;
    //std::vector<std::pair<std::string,MTypeAST*>> values;
    std::vector<MTypeAST*> values;

    type = baseTypeDecl();

    if( tryConsume(Token::BITWISE_OR ) ) {
        values.push_back(type);
        goto Q_NEXT;
    }
    return type;

Q_NEXT:
    if( !expect(Token::IDENTIFIER) ) return 0;
    type = baseTypeDecl();
    values.push_back(type);
    if(tryConsume(Token::BITWISE_OR))goto Q_NEXT;
    return new MUnionTypeAST(values);

};

CondStmt*   Parser::condStatement() {
    if(!expectConsume(Token::IF)) return 0;

    CondStmtList l;
    MValueAST *v;
    StatementList *stmts;
    Statement *s;

Q_CASE:
    v = value();
    if( !expectConsume(Token::COLON) 
        || !expectConsume(Token::NEWLINE)
        || !expectConsume(Token::INDENT))
        return 0;

    stmts = new StatementList();
    while( ( s = statement()) != 0 ) {
        stmts->push_back(s);
        if( tryConsume(Token::DEINDENT) )
            break;
    }

    l.push_back(std::make_pair(v,stmts));
    if(tryConsume(Token::ELIF))
        goto Q_CASE;

    stmts=0;
    if(tryConsume(Token::ELSE)){
        if( !expectConsume(Token::COLON) 
            || !expectConsume(Token::NEWLINE)
            || !expectConsume(Token::INDENT))
            return 0;

        stmts = new StatementList();
        while( ( s = statement()) != 0 ) {
            stmts->push_back(s);
            if( tryConsume(Token::DEINDENT) )
                break;
        }
    }

    return new CondStmt(l,stmts);
}


MTypeAST* Parser::arrayTypeTrailer(MTypeAST *type) {
    if(!expectConsume(Token::OPEN_BRACKET)) return 0;
    if(!expectConsume(Token::CLOSE_BRACKET)) return 0;
    return new MArrayTypeAST(type);
}

MTupleTypeAST* Parser::tupleTypeDecl() {
    if(!expectConsume(Token::OPEN_PARENTHESIS)) return 0;
    std::vector<MTypeAST*> types;
    std::vector<std::pair<std::string,MTypeAST*>> namedValues;
    while( type() != Token::CLOSE_PARENTHESIS ) {
        if(!expect(Token::IDENTIFIER)) return 0;
        std::string name = currentConsume().str_val;
        if(tryConsume(Token::COLON)) {
            namedValues.push_back(make_pair(name,typeDecl()));
        } else {
            namedValues.push_back(make_pair(name,(MTypeAST*)0));
        }
        if(!tryConsume(Token::COMMA))break;
    }
    if(!expectConsume(Token::CLOSE_PARENTHESIS)) return 0;
    return new MTupleTypeAST(types,namedValues);
}

ForStmt* Parser::forStatement() {
    if( !expectConsume(Token::FOR) || !expect(Token::IDENTIFIER)) return 0;

    std::string target_name = currentConsume().str_val;

    if(!expectConsume(Token::IN)) return 0;

    MValueAST *val = value();

    if( !expectConsume(Token::COLON) 
        || !expectConsume(Token::NEWLINE)
        || !expectConsume(Token::INDENT))
        return 0;


    Statement *s = 0;
    StatementList *stmts = new StatementList;
    while( ( s = statement()) != 0 ) {
        stmts->push_back(s);
        if( tryConsume(Token::DEINDENT) )
            break;
    }

    return new ForStmt(target_name,val,stmts);
}


