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
#include "var.h"

#include "ast/declarations/system.h"

using namespace llvm;

void VarDecl::codegen(Context *ctx) {
    // TODO: QUICKFIX - don't compile as system decl
    if(dynamic_cast<SystemContext*>(ctx) != 0) {
        return;
    }
    assert(alloc);
    MValue *v = val->codegen(ctx,typeVal);
    Builder.CreateStore(v->value(), alloc);
    ctx->bindValue(name, new MValue(v->type,alloc,true));
}

void VarDecl::collectAlloc(Context *ctx) {
    if( type )
        typeVal = type->codegen(ctx);
    else
        typeVal = val->calculateType(ctx);
    alloc = Builder.CreateAlloca(typeVal->llvmType(), nullptr, "var." + name + ".alloca");
}

void VarDecl::collectSystemDecl(Context *ctx) const {
    SystemType *s = ctx->storage->system;
    s->variables.push_back(make_pair(name,type->codegen(ctx)));
}
