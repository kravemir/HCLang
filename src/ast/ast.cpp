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
#include "ast.h"

#include <iostream>
#include "printer.h"

using namespace llvm;

IRBuilder<> Builder(getGlobalContext());
LLVMContext &lctx = getGlobalContext();

MValue* GetChildAST::codegen(Context *ctx, MValueType *type) {
    MValue *target = val->codegen(ctx);
    MValue *child = target->getChild(name);
    assert(child);
    return child;
}
MValue* VarExpr::codegen(Context *ctx, MValueType *type) {
    MValue *v = ctx->getValue(str); // TODO
    assert(v);
    return v;
}
MValue* CallExpr::codegen(Context *ctx, MValueType *type) {
    MValue *v = val->codegen(ctx);
    MValueType *ft = v->type;
    if( !ft->callable ) { 
        std::cerr << "Can't use as function\n" << std::endl;
    }
    std::vector<Value *> argsV;
    for(int i = 0; i < args->size(); i++) {
        auto a = args->get(i);
        argsV.push_back(a->codegen(ctx)->value());
    }

    MValue * val = new MValue({ft->callReturnType(),Builder.CreateCall(v->value(), argsV, "calltmp")});
    if( type )
        return type->createCast(ctx,val);
    return val;

}
MValue* SpawnExpr::codegen(Context *ctx, MValueType *type) {
    MValue* v = ctx->getValue(name);
    assert(v);
    
    SystemType *system_type = dynamic_cast<SystemType*>(v->type);
    ProcedureAsyncType *procedure_type = dynamic_cast<ProcedureAsyncType*>(v->type);
    if( system_type ) {
        Function *create_fn = system_type->fn_new;
        GlobalValue* executor = ctx->storage->module->getNamedValue("executor");
        Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));
        std::vector<llvm::Value*> indices(1,zero);
        std::vector<Value *> ArgsV({Builder.CreateLoad(Builder.CreateGEP(executor,indices,"global_executor"))});

        return new MValue({system_type, Builder.CreateCall(create_fn, ArgsV)});
    } else if(procedure_type) {
        GlobalValue* executor = ctx->storage->module->getNamedValue("executor");
        Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(getGlobalContext()));
        std::vector<llvm::Value*> indices(1,zero);
        std::vector<Value *> ArgsV({Builder.CreateLoad(Builder.CreateGEP(executor,indices,"global_executor"))});
        
        Value *procinst = Builder.CreateCall(v->value(), ArgsV, "procedure_instance");
        Function *finit = ctx->storage->module->getFunction("system_putMsg");
        std::vector<llvm::Value*> aadices({
            procinst,
            ConstantInt::get(lctx,APInt((unsigned)32,0)),
            spawnArgs->codegen(ctx)->value()
        });
        Builder.CreateCall(finit,aadices);
        return 0;
    }
    assert(0);
}
MValue* StringAST::codegen(Context *ctx, MValueType *type) {
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));
    Constant *format_const = ConstantDataArray::getString(lctx, val.c_str() );
    Type* ltype = ArrayType::get(IntegerType::get(lctx, 8), val.size() + 1);
    GlobalVariable *var = new GlobalVariable(
        *ctx->storage->module, 
        ltype,
        true, 
        llvm::GlobalValue::PrivateLinkage, 
        format_const, 
        ".str"
    );


    std::vector<llvm::Constant*> indices(2,zero);
    MValue *vval = new MValue({new StringType(llvm::Type::getInt8PtrTy(ctx->storage->module->getContext())),ConstantExpr::getGetElementPtr(ltype, var, indices)});
    if(type)
        vval = type->createCast(ctx,vval);
    return vval;

}
MValue* IntegerAST::codegen(Context *ctx, MValueType *type) {
    return new MValue({
        new IntType(llvm::Type::getInt64Ty(ctx->storage->module->getContext())),
        ConstantInt::get(getGlobalContext(),APInt((unsigned)64,val))
    });
}

#include <llvm/IR/Verifier.h>

void BindStmt::codegen(Context *ctx) {
    if(target[0] == "self") {
        MValue *val = value->codegen(ctx);
        MValue *target = ctx->getValue(this->target[0]);
        MValue *var = target->getChild(this->target[1]);
        var->store(val);
    } else {
        MValue *newValue = value->codegen(ctx);
        MValue *varValue = ctx->getValue(target[0]);
        varValue->store(newValue);
    }
}


void VarDecl::codegen(Context *ctx) {
    MValueType *t = 0;
    if( type )
        t = type->codegen(ctx);
    MValue *v = val->codegen(ctx,t);
    AllocaInst *alloc = Builder.CreateAlloca(v->type->llvmType(), nullptr, "var." + name + ".alloca");
    Builder.CreateStore(v->value(), alloc);
    ctx->bindValue(name, new MValue(v->type,alloc,true));
}
void VarDecl::collectSystemDecl(Context *ctx) const {
    SystemType *s = ctx->storage->system;
    s->variables.push_back(make_pair(name,type->codegen(ctx)));
}

Value* MValue::value() {
    if(variable)
        return Builder.CreateLoad(_value);
    else
        return _value;
}

void MValue::store(MValue *v) {
    // TODO nice error
    assert(variable && "Must be variable");

    Builder.CreateStore(v->value(),_value);
}
void CondStmt::codegen(Context *ctx) {
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
    for( int i = 0; i < stmts.size(); i++ ) {
        MValue *cond = stmts[i].first->codegen(ctx);
        BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
        BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");

        Builder.CreateCondBr(cond->value(), ThenBB, ElseBB);

        // Emit then value.
        Builder.SetInsertPoint(ThenBB);
        // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
        ThenBB = Builder.GetInsertBlock();
        for( auto *s : *stmts[i].second )
            s->codegen(ctx);
        Builder.CreateBr(MergeBB);

        // Emit else block.
        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);

        if( i == stmts.size() - 1 ) {
            if( elStmt ) {
                for( auto *s : *elStmt )
                    s->codegen(ctx);
            }
            Builder.CreateBr(MergeBB);
            // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
            ElseBB = Builder.GetInsertBlock();
        }

    }
    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
}
void ForStmt::codegen(Context *ctx) {
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *condBB = BasicBlock::Create(getGlobalContext(), "cond");
    BasicBlock *forBB = BasicBlock::Create(getGlobalContext(), "for");
    BasicBlock *endBB = BasicBlock::Create(getGlobalContext(), "endfor");

    AllocaInst* iPtr = Builder.CreateAlloca(Type::getInt64Ty(lctx));
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
void ReturnStmt::codegen(Context *ctx) {
    MValue *value = val->codegen(ctx);
    Builder.CreateRet(value->value());
}
void TypeDecl::codegen(Context *ctx) {
    auto t = type->codegen(ctx);
    ctx->storage->types[name] = t;
    ctx->bindValue(name,t->createConstructor(ctx));
}
void FunctionDecl::codegen(Context *_ctx) {
    Context ctx (_ctx);
    std::vector<llvm::Type*> args;
    std::vector<MValueType*> types;
    for( int i = 0; i < this->args->namedValues.size(); i++ ) {
        auto v = this->args->namedValues[i];
        auto type = v.second->codegen(&ctx);
        args.push_back(type->llvmType());
        types.push_back(type);
    }
    llvm::FunctionType *FT = llvm::FunctionType::get(retType->codegen(&ctx)->llvmType(), args, false);

    Function *F = Function::Create(FT, Function::PrivateLinkage, ctx.storage->prefix + name, ctx.storage->module);

    auto *ft = new MFunctionType();
    ft->retType = retType->codegen(&ctx);
    _ctx->bindValue(name, new MValue({
                ft,
                F
        }));

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);

    Function::arg_iterator arg = F->arg_begin();
    for( int i = 0; i < this->args->namedValues.size(); i++, arg++ ) {
        auto v = this->args->namedValues[i];
        ctx.bindValue(v.first, new MValue({types[i],arg}));
    }
    
    for(Statement *stmt : *stmts)
        stmt->codegen(&ctx);
}
MValue* CondExpr::codegen(Context *ctx, MValueType *type) {
    Value *CondV = cond->codegen(ctx)->value();

    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

    Builder.CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then value.
    Builder.SetInsertPoint(ThenBB);
    MValue *MThenV = thenVal->codegen(ctx);
    Value *ThenV = MThenV->value();
    Builder.CreateBr(MergeBB);

    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = Builder.GetInsertBlock();

    // Emit else block.
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    Value *ElseV = elseVal->codegen(ctx)->value();

    Builder.CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = Builder.GetInsertBlock();

    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    PHINode *PN =
        Builder.CreatePHI(MThenV->type->llvmType(), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return new MValue({MThenV->type,PN});
}
void MatchAssignStmt::codegen(Context *ctx) {
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    MValue* incoming = src->codegen(ctx);

    std::vector<BasicBlock*> blocks;
    std::vector<MValue*> values;
    std::vector<llvm::Value*> valuesl;
    BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "match_merge");

    for( auto c : cases ) {
        std::pair<llvm::Value*,MValue*> cv = incoming->type->matchCond(c.first,incoming,ctx);
        
        BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "match_case", TheFunction);
        BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "match_nextcase");

        Builder.CreateCondBr(cv.first, ThenBB, ElseBB);
        Builder.SetInsertPoint(ThenBB);
        MValue *v = c.second->codegen(ctx);
        values.push_back(v);
        valuesl.push_back(Builder.CreateBitCast(v->value(),Type::getInt8PtrTy(lctx)));
        Builder.CreateBr(MergeBB);

        blocks.push_back(Builder.GetInsertBlock());

        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);


        ElseBB = Builder.GetInsertBlock();
        if( values.size() == cases.size() ) {
            // not match!
            valuesl.push_back(Constant::getNullValue(Type::getInt8PtrTy(lctx)));
            blocks.push_back(ElseBB);
            Builder.CreateBr(MergeBB);
        }
    }

    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);

    PHINode* phi = Builder.CreatePHI(Type::getInt8PtrTy(lctx),values.size(), "matchresult" );
    for( int i = 0; i < valuesl.size(); i++ ) {
        phi->addIncoming(valuesl[i],blocks[i]);
    }
    ctx->bindValue(target[0], new MValue{ 0, phi });
    return;
}
