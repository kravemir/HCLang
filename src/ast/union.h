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
