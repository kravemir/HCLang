#ifndef HCLANG_AST_EXPRBINOP_H
#define HCLANG_AST_EXPRBINOP_H

#include "base.h"

class BinaryOpAST : public MValueAST {
public:
    BinaryOpAST(Token::Type op, MValueAST *left, MValueAST *right);

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    Token::Type op;
    MValueAST *left, *right;
};

#endif // HCLANG_AST_EXPRBINOP_H
