#ifndef HCLANG_AST_STMTSEND_H
#define HCLANG_AST_STMTSEND_H

#include "base.h"

class SendStmt : public Statement {
public:
    SendStmt(Path target, std::string msg, TupleAST *args);

    virtual void codegen(Context *ctx);

    virtual void print(Printer &p) const;
private:
    Path target;
    std::string msg;
    TupleAST *args;
};


#endif // HCLANG_AST_STMTSEND_H
