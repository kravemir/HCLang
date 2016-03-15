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
#include "function.h"

#include <ast/types/tuple.h>

using namespace llvm;

void FunctionDecl::codegen(Context *_ctx) {
    Context ctx (_ctx);
    std::vector<llvm::Type*> args;
    std::vector<MValueType*> types;
    for( size_t i = 0; i < this->args->namedValues.size(); i++ ) {
        auto v = this->args->namedValues[i];
        auto type = v.second->codegen(&ctx);
        args.push_back(type->llvmType());
        types.push_back(type);
    }

    MValueType *rt = retType->codegen(&ctx);
    llvm::FunctionType *FT = llvm::FunctionType::get(rt->llvmType(), args, false);

    Function *F = Function::Create(FT, Function::PrivateLinkage, ctx.storage->prefix + name, ctx.storage->module);

    auto *ft = new MFunctionType(rt);
    ft->retType = retType->codegen(&ctx);
    _ctx->bindValue(name, new MValue({
                                             ft,
                                             F
                                     }));

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);

    Function::arg_iterator arg = F->arg_begin();
    for( size_t i = 0; i < this->args->namedValues.size(); i++, arg++ ) {
        auto v = this->args->namedValues[i];
        ctx.bindValue(v.first, new MValue({types[i],arg}));
    }

    stmts->collectAlloc(&ctx);
    stmts->codegen(&ctx);
}