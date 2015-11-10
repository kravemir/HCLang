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
#include "union.h"

using namespace llvm;

Type* MUnionType::llvmType() const {
    std::vector<Type*> types({
        Type::getInt32Ty(lctx),
        ArrayType::get(Type::getInt8Ty(lctx), size)
    });
    StructType *st = StructType::create(lctx, types, "tuple");
    return PointerType::get(st,0);
}

MValue* MUnionType::createCast(Context *ctx, MValue *val ) {
    //exit(43);
    Type* t = llvmType();
    std::vector<Type*> types({
        Type::getInt32Ty(lctx),
        ArrayType::get(Type::getInt8Ty(lctx), size)
    });
    StructType *st = StructType::create(lctx, types, "tuple");
    
    Function *fmalloc = ctx->storage->module->getFunction("malloc");
    std::vector<Value *> fmalloc_args({ConstantInt::get(lctx,APInt((unsigned)64,(uint64_t)size))});
    Value *a = Builder.CreateBitCast(
            Builder.CreateCall(fmalloc,fmalloc_args),
            t
        );
    Builder.CreateStore(
        ConstantInt::get(lctx,APInt(32,(uint64_t)0)),
        Builder.CreateGEP(a, std::vector<Value*>({
            ConstantInt::get(lctx, APInt( 32, (uint64_t)0 )),
            ConstantInt::get(lctx, APInt( 32, (uint64_t)0 ))
        }))
    );
    Builder.CreateStore(
        val->value(),
        Builder.CreateGEP(a, std::vector<Value*>({
            ConstantInt::get(lctx, APInt( 32, (uint64_t)0 )),
            ConstantInt::get(lctx, APInt( 32, (uint64_t)1 ))
        }))
    );
    return new MValue(this,a);
}

std::pair<llvm::Value*,MValue*> MUnionType::matchCond(std::string targetName, MValue* src, Context *ctx) { 
    return {
        ConstantInt::get(lctx, APInt( 32, (uint64_t)1 )),
        new MValue(
            alternatives[0],
            Builder.CreateGEP(src->value(), std::vector<Value*>({
                ConstantInt::get(lctx, APInt( 32, (uint64_t)0 )),
                ConstantInt::get(lctx, APInt( 32, (uint64_t)1 ))
            }))
        )
    };
}
MValueType* MUnionTypeAST::codegen(Context *ctx) { 
    std::vector<MValueType*> values;
    int size = 0;
    for( auto tt : this->values ) {
        auto t = tt->codegen(ctx);
        int ns = ctx->storage->module->getDataLayout().getTypeAllocSize(t->llvmType());
        size = size > ns ? size : ns;
        values.push_back(t);
    }
    return new MUnionType(values, size);
};
