#ifndef HCLANG_AST_SLOT_H
#define HCLANG_AST_SLOT_H

#include "base.h"

class SlotDecl : public Statement {
public:
    SlotDecl( std::string name, MTupleTypeAST *args, StatementList *list ):
        name(name),
        args(args),
        stmts(list)
    {}

    virtual void codegen(Context *ctx);

    virtual void print(Printer &p) const;
    virtual void collectSystemDecl(Context *ctx) const;
private:
    std::string name;
    MTupleTypeAST *args;
    StatementList *stmts;
};

#endif // HCLANG_AST_SLOT_H
