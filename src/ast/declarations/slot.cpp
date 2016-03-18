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
#include "slot.h"

#include "system.h"

using namespace llvm;

SlotType* SlotType::create ( Context *ctx, TupleType* argsType, MValueType *returnType) {
    StructType *st = StructType::create ( lctx, {
        Type::getInt8PtrTy( lctx ),
        Type::getInt32Ty ( ctx->storage->module->getContext() ),
        //argsType->llvmType()
    }, "slot" );

    // TODO debug types option: ((PointerType*)argsType->llvmType())->getElementType()->dump();
    SlotType * type = new SlotType ( st, argsType, returnType );
    type->putMsgFn = ctx->storage->module->getFunction("system_putMsg");

    return type;
}

void SlotType::codegenSendTo(MValue *value, MValue *msg) {
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));

    std::vector<llvm::Value *> aadices({
                                               Builder.CreateExtractValue(value->value(), {0}, "send.target"),
                                               Builder.CreateExtractValue(value->value(), {1}, "send.msg"),
                                               msg ? msg->value() : zero
                                       });
    Builder.CreateCall(putMsgFn, aadices);
}


MValueType* SlotTypeAST::codegen ( Context* ctx ) {
    TupleType *argsType = args->codegen ( ctx );
    MValueType *retType = 0;
    if(returnType) {
        retType = returnType->codegen(ctx);
        SlotType *rst = SlotType::create(ctx, TupleType::create( { {"val",retType} } ), 0 );
        std::vector<std::pair<std::string,MValueType*>> namedValues = argsType->namedValues;
        namedValues.push_back({"return_slot", rst});
        argsType = TupleType::create(namedValues);
    }
    return SlotType::create ( ctx, argsType, retType );
}

class SlotContext : public Context {
public:
    SlotContext(Context *parent, SlotType *type) :
        Context(parent),
        type(type)
    { }


    virtual bool doCustomReturn(MValue *value) override;

private:
    SlotType *type;
};

void SlotDecl::codegen ( Context *_ctx ) {
    SystemType *s = _ctx->storage->system;
    TupleType *slotArgsTuple = s->slotTypes[s->slotIds[name]]->argsTupleType;

    std::vector<llvm::Type*> args;
    args.push_back ( _ctx->storage->system->llvmType() );
    args.push_back ( slotArgsTuple->llvmType() );
    llvm::FunctionType *FT = llvm::FunctionType::get (
                                 llvm::Type::getVoidTy ( llvm::getGlobalContext() ), args, false );

    Function *F = Function::Create ( FT, Function::PrivateLinkage, _ctx->storage->prefix + name, _ctx->storage->module );

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create ( getGlobalContext(), "entry", F );
    Builder.SetInsertPoint ( BB );

    SlotContext ctx ( _ctx, this->type );
    ctx.bindValue ( "self", new MValue ( { ctx.storage->system, F->arg_begin() } ) );
    for ( size_t i = 0; i < this->args->namedValues.size(); i++ ) {
        auto v = this->args->namedValues[i];
        std::vector<llvm::Value*> args ( {
            ConstantInt::get ( lctx,APInt ( ( unsigned ) 32, ( uint64_t ) 0 ) ),
            ConstantInt::get ( lctx,APInt ( ( unsigned ) 32, ( uint64_t ) i ) ),
        } );
        Value *valPtr = Builder.CreateGEP ( ++ ( F->arg_begin() ), args, v.first + "_ptr" );
        auto type = v.second->codegen ( &ctx );
        if( (dynamic_cast<SlotType*>(slotArgsTuple->namedValues[i].second) == 0) || true )
            ctx.bindValue ( v.first,new MValue ( { type, Builder.CreateLoad ( valPtr, v.first ) } ) );
        else
            ctx.bindValue ( v.first,new MValue ( { type, valPtr } ) );
    }

    stmts->collectAlloc(&ctx);
    stmts->codegen(&ctx);

    Builder.CreateRetVoid();
    s->slots.push_back ( F );

    if(ctx.storage->print_llvm_ir) {
        fprintf(stderr,"DUMP: slot %s:\n",this->name.c_str());
        F->dump();
    }
}
void SlotDecl::collectSystemDecl ( Context *ctx ) const {
    SystemType *s = ctx->storage->system;
    s->slotIds[name] = s->slotCount++;


    TupleType *argsType = this->args->codegen ( ctx );
    MValueType *retType = 0;
    if(returnType) {
        retType = returnType->codegen(ctx);
        SlotType *rst = SlotType::create(ctx, TupleType::create( { {"val",retType} } ), 0 );
        std::vector<std::pair<std::string,MValueType*>> namedValues = argsType->namedValues;
        namedValues.push_back({"return_slot", rst});
        argsType = TupleType::create(namedValues);
    }
    this->type = SlotType::create(ctx,argsType,retType);
    s->slotTypes.push_back ( type );
}

bool SlotContext::doCustomReturn(MValue *value) {
    if(type->returnType) {
        TupleType *tt = TupleType::create({ {"val",type->returnType} });
        int size = storage->module->getDataLayout().getTypeAllocSize(
                ((PointerType*)tt->llvmType())->getElementType()
        );
        std::vector<Value *> fmalloc_args({ConstantInt::get(lctx,APInt(64,(uint64_t)size))});
        Function *fmalloc = storage->module->getFunction("malloc");
        Value *call = Builder.CreateCall(fmalloc, fmalloc_args, "tuple_alloc");
        Value *a = Builder.CreateBitCast(call, tt->llvmType(), "tuple");
        Value *valPtr = Builder.CreateGEP(a, {
                ConstantInt::get(lctx, APInt(32, (uint64_t) 0)),
                ConstantInt::get(lctx, APInt(32, (uint64_t) 0))
        });
        Builder.CreateStore(value->value(), valPtr);

        Function *finit = storage->module->getFunction("system_putMsg");
        Function* TheFunction = Builder.GetInsertBlock()->getParent();
        Value *ma = ++(TheFunction->arg_begin());
        std::vector<llvm::Value*> aadices(
                {
                        Builder.CreateLoad(Builder.CreateGEP(
                                ma,
                                {
                                        (Value *) ConstantInt::get(lctx, APInt(32, (uint64_t) 0)),
                                        (Value *) ConstantInt::get(lctx, APInt(32, (uint64_t) type->argsTupleType->namedValues.size() -1)),
                                        (Value *) ConstantInt::get(lctx, APInt(32, (uint64_t) 0))
                                })),
                        Builder.CreateLoad(Builder.CreateGEP(
                                ma,
                                {
                                        (Value *) ConstantInt::get(lctx, APInt(32, (uint64_t) 0)),
                                        (Value *) ConstantInt::get(lctx, APInt(32, (uint64_t) type->argsTupleType->namedValues.size() -1)),
                                        (Value *) ConstantInt::get(lctx, APInt(32, (uint64_t) 1))
                                })),
                        valPtr
                });
        Builder.CreateCall(finit,aadices);
        return true;
    }
    return false;
}