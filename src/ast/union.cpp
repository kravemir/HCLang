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
