#ifndef HCLANG_AST_TUPLE_H
#define HCLANG_AST_TUPLE_H

#include "base.h"

struct TupleType : MValueType {
    std::vector<MValueType*> values;
    std::vector<std::pair<std::string,MValueType*>> namedValues;

    TupleType( std::vector<std::pair<std::string,MValueType*>> namedValues):
        namedValues(namedValues)
    {}

    virtual MValue* getChild(MValue *src, std::string name);
    virtual llvm::Type* llvmType() const;
    int getIndex(std::string name);
    virtual MValue* createConstructor(Context *ctx);
};

struct MTupleTypeAST : MTypeAST {
    MTupleTypeAST(std::vector<MTypeAST*> types, std::vector<std::pair<std::string,MTypeAST*>> namedValues):
        types(types),
        namedValues(namedValues)
    {}

    MValueType* codegen(Context *ctx);

    std::vector<MTypeAST*> types;
    std::vector<std::pair<std::string,MTypeAST*>> namedValues;
};

struct TupleAST : MValueAST {
    TupleAST( MValueList *values, MValueMap *namedMValues );

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    std::string toString() const;

    MValueAST* get(size_t idx) {
        return (*values)[idx];
    }

    size_t size() const {
        return values->size();
    }

    MValueList *values;
    MValueMap *namedMValues;
};

#endif // HCLANG_AST_TUPLE_H
