#ifndef HCLANG_AST_PROCEDURE_H
#define HCLANG_AST_PROCEDURE_H

#include "base.h"

class ProcedureDecl : public Statement {
public:
    ProcedureDecl( std::string name, MTupleTypeAST *args, StatementList *list ):
        name(name),
        args(args),
        stmts(list)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;

private:
    std::string name;
    MTupleTypeAST *args;
    StatementList *stmts;
};

#endif // HCLANG_AST_PROCEDURE_H
