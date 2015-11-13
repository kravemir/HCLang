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
#include "procedure.h"

#include "tuple.h"

using namespace llvm;

void ProcedureDecl::codegen(Context *_ctx) {
    Context ctx (_ctx);
    MValueType *returnType = this->returnType->codegen(_ctx);
    
    std::vector<llvm::Type*> args;
    std::vector<MValueType*> types;
    for( auto &v : this->args->namedValues ) {
        auto t = v.second->codegen(&ctx);
        types.push_back(t);
        args.push_back(t->llvmType());
    }

    llvm::FunctionType *FT = llvm::FunctionType::get(returnType->llvmType(), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, ctx.storage->prefix + name, ctx.storage->module);

    auto *ft = new ProcedureType(FT,returnType);
    _ctx->bindValue(name, new MValue({ft,F}));

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);

    auto it = F->arg_begin();
    for( int i = 0; i < this->args->namedValues.size(); i++, it++ ) {
        auto v = this->args->namedValues[i];
        it->setName(v.first);
        ctx.bindValue(v.first,new MValue({ types[i], it}));
    }
    
    for(Statement *stmt : *stmts)
        stmt->codegen(&ctx);

    Builder.CreateRetVoid();

    F->dump();
}
