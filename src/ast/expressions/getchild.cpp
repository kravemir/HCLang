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
#include "getchild.h"

MValueType* GetChildAST::calculateType(Context *ctx) {
    MValueType *t = val->calculateType(ctx);
    assert(t);
    return t->getChildType(name);
};
MValue* GetChildAST::codegen(Context *ctx, MValueType *type) {
    MValue *target = val->codegen(ctx);
    MValue *child = target->getChild(name);
    assert(child);
    return child;
}

MValueType* GetIndexChildAST::calculateType(Context *ctx) {
    MValueType *t = val->calculateType(ctx);
    assert(t);
    // TODO: check whether index is constant then use it, otherwise exception
    return t->getIndexChildType(0);
};
MValue* GetIndexChildAST::codegen(Context *ctx, MValueType *type) {
    MValue *target = val->codegen(ctx);
    MValue *child = target->type->getIndexChild(target,index->codegen(ctx)->value());
    assert(child);
    return child;
}