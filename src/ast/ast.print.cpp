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
 */
#include "ast.h"

#include <iostream>
#include "printer.h"

using namespace llvm;


VarExpr::VarExpr(std::string str):
    str(str)
{}

std::string VarExpr::toString() const {
    return str;
}
std::string CallExpr::toString() const {
    return val->toString() + "( ... )"; // TODO
}
SpawnExpr::SpawnExpr(std::string val,TupleAST *spawnArgs):
    name(val),
    spawnArgs(spawnArgs)
{}

std::string SpawnExpr::toString() const {
    return "spawn " + name;
}
std::string CondExpr::toString() const {
    return "if " + cond->toString() + 
        " then " + thenVal->toString() + 
        " else " + elseVal->toString();
}

StringAST::StringAST(std::string val):
    val(val)
{}

std::string StringAST::toString() const {
    return '"' + val + '"';
}

IntegerAST::IntegerAST(uint64_t val):
    val(val)
{}

std::string IntegerAST::toString() const {
    return std::to_string(val);
}

Printer& operator<<(Printer &p, const Statement &s) {
    s.print(p);
    return p;
}

void TypeDecl::print(Printer &p) const {
    p.println("type TODO");
}

void SystemDecl::print(Printer &p) const {
    p.beginBlock("system " + name);
    for( Statement* s : *stmts )
        p << *s;
    p.endBlock();
}
void SlotDecl::print(Printer &p) const {
    std::string argss = "(";
    //bool first = true;
    /*for( auto a : *args ) {
        if(first)
            first = false;
        else 
            argss += ", ";
        argss += a.first;
    }
    argss += ")";*/
    p.beginBlock("slot " + name + argss);
    for( Statement* s : *stmts )
        p << *s;
    p.endBlock();
}
void FunctionDecl::print(Printer &p) const {
    std::string argss = "(";
    //bool first = true;
    /*for( auto a : *args ) {
        if(first)
            first = false;
        else 
            argss += ", ";
        argss += a.first;
    }
    argss += ")";*/
    p.beginBlock("fn " + name + argss);
    for( Statement* s : *stmts )
        p << *s;
    p.endBlock();
}

void ProcedureDecl::print(Printer &p) const {
    std::string argss = "(";
    //bool first = true;
    /*for( auto a : *args ) {
        if(first)
            first = false;
        else 
            argss += ", ";
        argss += a.first;
    }
    argss += ")";*/
    p.beginBlock("procedure " + name + argss);
    for( Statement* s : *stmts )
        p << *s;
    p.endBlock();
}
void ForStmt::print(Printer &p) const {
    p.beginBlock("for ... ");
    for( Statement* s : *stmts )
        p << *s;
    p.endBlock();
}

SendStmt::SendStmt(Path target, std::string msg, TupleAST *args):
target(target),
msg(msg),
args(args) {

}

void SendStmt::print(Printer &p) const {
    p.println( target[0] + " ! " + msg + args->toString()); // TODO
}

void BindStmt::print(Printer &p) const {
    p.println( target[0] + " = " + value->toString()); // TODO
}
void MatchAssignStmt::print(Printer &p) const {
    p.beginBlock( target[0] + " = " + "match TODO"); // TODO
    for( auto c : cases )
        p.println(c.first);
    p.endBlock();
}

TupleAST::TupleAST(MValueList *values, MValueMap *namedMValues ):
    values(values),
    namedMValues(namedMValues)
{}
std::string TupleAST::toString() const {
    std::string str = "(";
    for(MValueAST *v : *values)
        str += v->toString();
    str += ")";
    return str;
}
std::string ArrayAST::toString() const {
    std::string str = "[";
    for(MValueAST *v : *values)
        str += v->toString();
    str += "]";
    return str;
}

BinaryOpAST::BinaryOpAST(Token::Type op, MValueAST *left, MValueAST *right):
    op(op),
    left(left),
    right(right)
{}

std::string BinaryOpAST::toString() const {
    std::map<Token::Type,std::string> ops;
    ops[Token::PLUS] = " + ";
    ops[Token::MINUS] = " - ";
    ops[Token::LESS] = " < ";
    return left->toString() + ops[op] + right->toString();
}

VarDecl::VarDecl(std::string name, MTypeAST *type, MValueAST *val):
    name(name),
    type(type),
    val(val)
{}


void VarDecl::print(Printer &p) const {
    p.println("var " + name + " : " /*+ type->toString()*/);
}
CondStmt::CondStmt(CondStmtList stmts, StatementList *elStmt):
    stmts(stmts),
    elStmt(elStmt)
{}

void CondStmt::print(Printer &p) const {
    p.println("TODO if");
}

ReturnStmt::ReturnStmt(MValueAST *val):
    val(val)
{}

void ReturnStmt::print(Printer &p) const {
    p.println("return " + val->toString());
}
