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
SpawnExpr::SpawnExpr(std::string val):
    name(val)
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
    bool first = true;
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
    bool first = true;
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
    bool first = true;
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

VarDecl::VarDecl(std::string name, MTypeAST *type):
    name(name),
    type(type)
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
