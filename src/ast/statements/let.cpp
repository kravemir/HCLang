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
#include "let.h"

#include "printer.h"

void LetStmt::codegen(Context *ctx) {
    MValueType *t = 0;
    if( letType )
        t = letType->codegen(ctx);

    llvm::Value* alloca = ctx->getAlloc(this->allocId);
    MValue *val;
    value->preCodegen(ctx);
    val = value->codegen(ctx, t);
    Builder.CreateStore(val->value(), alloca);
    ctx->bindValue(target[0], new MValue(val->type,alloca,true)); // TODO
}

void LetStmt::collectAlloc(Context *ctx) {
    MValueType *t = letType ? letType->codegen(ctx) : value->calculateType(ctx);
    ctx->bindValueType(this->target[0],t);
    assert(t);
    this->allocId = ctx->createAlloc(t);
}

void LetStmt::print(Printer &p) const {
    p.println( target[0] + " = " + value->toString()); // TODO
}
