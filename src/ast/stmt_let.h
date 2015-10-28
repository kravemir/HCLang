#ifndef HCLANG_AST_STMT_LET_H
#define HCLANG_AST_STMT_LET_H

#include "base.h"

class LetStmt : public Statement {
public:
    LetStmt(Path target, MTypeAST *letType, MValueAST *value):
        target(target),
        letType(letType),
        value(value)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;

private:
    Path target;
    MTypeAST *letType;
    MValueAST *value;
};


#endif // HCLANG_AST_STMT_LET_H
