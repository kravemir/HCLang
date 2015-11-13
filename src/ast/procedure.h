#ifndef HCLANG_AST_PROCEDURE_H
#define HCLANG_AST_PROCEDURE_H

#include "base.h"

class ProcedureDecl : public Statement {
public:
    ProcedureDecl( std::string name, MTupleTypeAST *args, StatementList *list, bool async ):
        name(name),
        args(args),
        stmts(list),
        async(async)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;

private:
    std::string name;
    MTupleTypeAST *args;
    StatementList *stmts;
    bool async;
};

#endif // HCLANG_AST_PROCEDURE_H
