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
#ifndef HCLANG_AST_TUPLE_H
#define HCLANG_AST_TUPLE_H

#include "base.h"

struct TupleType : MValueType {
    std::vector<MValueType*> values;
    std::vector<std::pair<std::string,MValueType*>> namedValues;


    virtual MValue* getChild(MValue *src, std::string name);
    int getIndex(std::string name);
    virtual MValue* createConstructor(Context *ctx);
    
    static TupleType *create( std::vector<std::pair<std::string,MValueType*>> namedValues, std::string name = "tuple");
    
private:
    TupleType() = delete;
    TupleType( llvm::Type *_llvmType, std::vector<std::pair<std::string,MValueType*>> namedValues):
        MValueType(_llvmType),
        namedValues(namedValues)
    {}
};

struct MTupleTypeAST : MTypeAST {
    MTupleTypeAST(std::vector<MTypeAST*> types, std::vector<std::pair<std::string,MTypeAST*>> namedValues):
        types(types),
        namedValues(namedValues)
    {}

    TupleType* codegen(Context *ctx);

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
