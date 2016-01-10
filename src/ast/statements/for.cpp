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
#include "for.h"

using namespace llvm;

void ForStmt::codegen(Context *ctx) {
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *condBB = BasicBlock::Create(getGlobalContext(), "cond");
    BasicBlock *forBB = BasicBlock::Create(getGlobalContext(), "for");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "endfor");

    assert( iPtr && "Probably collectAlloc wasn't called" );
    Builder.CreateStore( ConstantInt::get(lctx,APInt((unsigned)64,(uint64_t)0)), iPtr);
    MValue *val = this->inval->codegen(ctx);
    Builder.CreateBr(condBB);


    TheFunction->getBasicBlockList().push_back(condBB);
    Builder.SetInsertPoint(condBB);
    Value *cond = Builder.CreateICmpULT( Builder.CreateLoad(iPtr), val->getChild("size")->value());
    Builder.CreateCondBr(cond, forBB, endBB);

    TheFunction->getBasicBlockList().push_back(forBB);
    Builder.SetInsertPoint(forBB);
    ctx->bindValue(target_name, val->type->getArrayChild(val,Builder.CreateLoad(iPtr)));
    for( auto *s : *stmts )
        s->codegen(ctx);
    Builder.CreateStore(
            Builder.CreateAdd(Builder.CreateLoad(iPtr),ConstantInt::get(lctx,APInt((unsigned)64,(uint64_t)1))),
            iPtr
    );
    Builder.CreateBr(condBB);
    TheFunction->getBasicBlockList().push_back(endBB);
    Builder.SetInsertPoint(endBB);

}

void ForStmt::collectAlloc ( Context* ctx ) {
    iPtr = Builder.CreateAlloca(Type::getInt64Ty(lctx),0,"for.i.alloca");
    for( auto *s : *stmts )
        s->collectAlloc(ctx);
}

