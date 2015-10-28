#include "tuple.h"

using namespace llvm;

Type* TupleType::llvmType() const {
    std::vector<Type*> types;
    for( MValueType *vast : this->values ) {
        types.push_back(vast->llvmType());
    }
    for( auto vast : this->namedValues ) {
        types.push_back(vast.second->llvmType());
    }
    StructType *st = StructType::create(lctx, types, "tuple");
    return PointerType::get(st,0);
}
MValue* TupleType::createConstructor(Context *ctx) {
    std::vector<Type*> types;
    for( MValueType *vast : this->values ) {
        types.push_back(vast->llvmType());
    }
    for( auto vast : this->namedValues ) {
        types.push_back(vast.second->llvmType());
    }
    StructType *st = StructType::create(lctx, types, "tuple");
    Type* ptrType = PointerType::get(st,0);
    llvm::FunctionType *FT = llvm::FunctionType::get(ptrType, types, false);

    Function *F = Function::Create(FT, Function::PrivateLinkage, "tuple_constructor", ctx->storage->module);
    auto *ft = new MFunctionType();
    ft->retType = this;

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);

    std::vector<llvm::Value*> iindices({
            ConstantInt::get(lctx, APInt((unsigned)32,(uint64_t)1))
    });
    Value *sizeptr = Builder.CreateGEP(
            Constant::getNullValue(ptrType),
            iindices
        );
    Value *size = Builder.CreatePtrToInt(sizeptr, Type::getInt64Ty(lctx));
    Function *fmalloc = ctx->storage->module->getFunction("malloc");
    std::vector<Value *> fmalloc_args({size});
    Value *a = Builder.CreateBitCast(
            Builder.CreateCall(fmalloc,fmalloc_args),
            ptrType
        );

    auto arg = F->arg_begin();
    for( int i = 0; i < namedValues.size(); i++ ) {
        auto v = namedValues[i];
        Value *valPtr = Builder.CreateGEP(
            //v.second->llvmType(),
            a,
            std::vector<llvm::Value*>({
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)0)),
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)i)),
            })
        );
        Builder.CreateStore(arg,valPtr);
        ++arg;
    }

    Builder.CreateRet(a);
    return new MValue({ft,F});
}
MValue* TupleType::getChild(MValue *src, std::string name) { 
    int i = 0;
    for(; i < namedValues.size(); i++ )
        if( namedValues[i].first == name) break;
    Value *valPtr = Builder.CreateGEP(src->value(),{
            ConstantInt::get(lctx,APInt(32,(uint64_t)0)),
            ConstantInt::get(lctx,APInt(32,(uint64_t)i)),
    });
    return new MValue(
            namedValues[i].second, 
            valPtr,
            true
        );
}

MValueType* MTupleTypeAST::codegen(Context *ctx) {
    std::vector<std::pair<std::string,MValueType*>> namedValues;
    for( auto tt : this->namedValues )
        namedValues.push_back(std::make_pair(tt.first,tt.second->codegen(ctx)));
    return new TupleType(namedValues);
};

MValue* TupleAST::codegen(Context *ctx, MValueType *type) {
    LLVMContext &lctx = getGlobalContext();
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));


    std::vector<Value*> values;
    std::vector<Type*> types;
    std::vector<std::pair<std::string,MValueType*>> namedValues;
    for( MValueAST *vast : *this->values ) {
        MValue *v = vast->codegen(ctx);
        values.push_back(v->value());
        types.push_back(v->type->llvmType());
        namedValues.push_back(std::make_pair("",v->type));
    }
    TupleType *ttt = new TupleType(namedValues);
    StructType *st = StructType::create(lctx, types, "tuple");
    std::vector<llvm::Constant*> indices({
            ConstantInt::get(lctx, APInt((unsigned)32,(uint64_t)1))
    });
    PointerType *stPtr = PointerType::get(st,0);
    Value *sizeptr = ConstantExpr::getGetElementPtr(
            PointerType::get(st,0),
            Constant::getNullValue(PointerType::get(st,0)),
            indices
        );
    Value *size = Builder.CreatePtrToInt(sizeptr, Type::getInt64Ty(lctx));
    //Value *size = ConstantInt::get(lctx, APInt((unsigned)64,(uint64_t)32));
    Function *fmalloc = ctx->storage->module->getFunction("malloc");
    std::vector<Value *> fmalloc_args( {size} );
    Value *call = Builder.CreateCall(fmalloc,fmalloc_args, "tuple_alloc");
    Value *a = Builder.CreateBitCast(call, stPtr, "tuple");
    for( int i = 0; i < values.size(); i++ ) {
        Value *valPtr = Builder.CreateGEP( a, {
            ConstantInt::get(lctx,APInt(32,(uint64_t)0)),
            ConstantInt::get(lctx,APInt(32,(uint64_t)i))
        });
        Builder.CreateStore(values[i],valPtr);
    }

    return new MValue({ttt,a});
}
