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
#include "array.h"

using namespace llvm;

MValue* MArrayType::getChild(MValue *src, std::string name) {
    Value * val = Builder.CreateLoad(
        Builder.CreateGEP(
            //Type::getInt64Ty(lctx),
            src->value(),
            std::vector<llvm::Value*>({
                ConstantInt::get(lctx,APInt(32,(uint64_t)0)),
                ConstantInt::get(lctx,APInt(32,(uint64_t)0))
            })
        )
    );
    return new MValue(elementType, val); 
}
MValue* MArrayType::getArrayChild(MValue *src, llvm::Value *idx) { 
    Value * val = Builder.CreateLoad(
        Builder.CreateGEP(
            src->value(),
            std::vector<llvm::Value*>({
                ConstantInt::get(lctx,APInt(32,(uint64_t)0)),
                ConstantInt::get(lctx,APInt(32,(uint64_t)1)),
                //Builder.CreateSExt(idx,Type::getInt64Ty(lctx))
                idx
            })
        )
    );
    return new MValue(elementType, val); 
}


MArrayType *MArrayType::create(MValueType *elementType) {
    return new MArrayType(
            elementType,
            PointerType::getUnqual(
                StructType::get(lctx,{
                        Type::getInt64Ty(lctx),
                        ArrayType::get(elementType->llvmType(), 1)
                })
            )
    );
}

MValueType* MArrayTypeAST::codegen(Context *ctx) {
    MValueType *elementType = element->codegen(ctx);
    return MArrayType::create(elementType);
}

#include <cstdlib>

MValueType* ArrayAST::calculateType(Context* ctx) {
    return MArrayType::create((*this->values)[0]->calculateType(ctx));
}

MValue* ArrayAST::codegen(Context *ctx, MValueType *type) {
    MValueType *childType = 0;
    if(type != 0) {
        MArrayType *atype = dynamic_cast<MArrayType*>(type);
        if(atype == 0) exit(43);
        childType = atype->elementType;
    }

    LLVMContext &lctx = getGlobalContext();

    std::vector<Value*> values;
    std::vector<MValueType*> types;
    for( MValueAST *vast : *this->values ) {
        MValue *v = vast->codegen(ctx,childType);
        values.push_back(v->value());
        types.push_back(v->type);
    }

    ArrayType *aType = ArrayType::get(types[0]->llvmType(),types.size());
    StructType *lType = StructType::get(lctx, std::vector<Type*>({
        Type::getInt64Ty(lctx),
        aType}));
    std::vector<llvm::Constant*> indices({
            ConstantInt::get(lctx, APInt((unsigned)32,(uint64_t)1))
    });
    PointerType *ptrType = PointerType::get(lType,0);
    Value *size = ConstantInt::get(lctx, APInt(
                (unsigned)64,
                (uint64_t)ctx->storage->module->getDataLayout().getTypeAllocSize(lType)
            ));
    Function *fmalloc = ctx->storage->module->getFunction("malloc");
    std::vector<Value *> fmalloc_args( {size} );
    Value *call = Builder.CreateCall(fmalloc,fmalloc_args);
    Value *a = Builder.CreateBitCast(call, ptrType, "array");

    Builder.CreateStore(
        ConstantInt::get(lctx,APInt((unsigned)64,(uint64_t)values.size())),
        Builder.CreateGEP(
            a,
            std::vector<llvm::Value*>({
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)0)),
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)0))
            })
        )
    );

    for( size_t i = 0; i < values.size(); i++ ) {
        Value *valPtr = Builder.CreateGEP(
            a,
            std::vector<llvm::Value*>({
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)0)),
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)1)),
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)i))
            }),
            std::string("array_eptr_") + std::to_string(i)
        );
        Builder.CreateStore(
                values[i],
                valPtr
            );

    }

    return new MValue({new MArrayType(types[0],lType),a});
}
