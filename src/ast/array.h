#ifndef HCLANG_AST_ARRAY_H
#define HCLANG_AST_ARRAY_H

#include "base.h"

struct MArrayType : MValueType {
    MValueType *elementType;
    llvm::Type *type;
    
    MArrayType ( MValueType *elementType, llvm::Type *type):
        elementType(elementType),
        type(type) 
    {
        assert(elementType);
        assert(type);
    }

    virtual MValue* getChild(MValue *src, std::string name);
    virtual MValue* getArrayChild(MValue *src, llvm::Value *index);
    virtual llvm::Type* llvmType() const {
        return type;
    }
};

struct MArrayTypeAST : MTypeAST {
    MArrayTypeAST(MTypeAST* element):
        element(element)
    {
        assert(element != 0);
    }

    virtual MValueType* codegen(Context *ctx);

private:
    MTypeAST* element;
};

struct ArrayAST : MValueAST {
    ArrayAST( MValueList *values ): values(values)
    {}

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    std::string toString() const;

    MValueAST* get(size_t idx) {
        return (*values)[idx];
    }

    size_t size() const {
        return values->size();
    }

    MValueList *values;
};

#endif // HCLANG_AST_ARRAY_H
