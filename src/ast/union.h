#ifndef HCLANG_AST_UNION_H
#define HCLANG_AST_UNION_H

#include "base.h"

struct MUnionType : MValueType {
    MUnionType(std::vector<MValueType*> alternatives, int size):
        alternatives(alternatives),
        size(size)
    {}


    void add(MValueType* t) {}; // TODO
    virtual llvm::Type* llvmType() const;

    virtual std::pair<llvm::Value*,MValue*> matchCond(std::string targetName, MValue* src, Context *ctx);
    virtual MValue* createCast(Context *ctx, MValue *src);

private:
    std::vector<MValueType*> alternatives;
    int size;

    llvm::Type *_llvmType = 0;
};

class MUnionTypeAST : public MTypeAST {
public:
    MUnionTypeAST(std::vector<MTypeAST*> values):
        values(values)
    {}

    virtual MValueType* codegen(Context *ctx);

private:
    std::vector<MTypeAST*> values;
};

#endif // HCLANG_AST_UNION_H
