/*
 * HCLang - Highly Concurrent Language
 * Copyright (c) 2015 Miroslav Kravec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef HCLANG_AST_ARRAY_H
#define HCLANG_AST_ARRAY_H

#include "ast/base.h"

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

    static MArrayType* create( MValueType *elementType );
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

    virtual MValueType* calculateType(Context *ctx);
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
