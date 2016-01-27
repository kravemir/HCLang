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

MValueType* GetChildAST::calculateType(Context *ctx) {
    MValueType *t = val->calculateType(ctx);
    assert(t);
    return t->getChildType(name);
};
MValue* GetChildAST::codegen(Context *ctx, MValueType *type) {
    MValue *target = val->codegen(ctx);
    MValue *child = target->getChild(name);
    assert(child);
    return child;
}
MValueType* VarExpr::calculateType(Context *ctx) {
    MValue *val = ctx->getValue(str);
    assert(val);
    return val->type;
};
MValue* VarExpr::codegen(Context *ctx, MValueType *type) {
    MValue *v = ctx->getValue(str); // TODO
    assert(v);
    return v;
}
MValueType* SpawnExpr::calculateType(Context *ctx) {
    MValue* v = ctx->getValue(name);
    assert(v);
    return v->type;
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
    for( size_t i = 0; i < stmts.size(); i++ ) {
        MValue *cond = stmts[i].first->codegen(ctx);
        BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
        BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");

        Builder.CreateCondBr(cond->value(), ThenBB, ElseBB);

        // Emit then value.
        Builder.SetInsertPoint(ThenBB);
        // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
        ThenBB = Builder.GetInsertBlock();


        stmts[i].second->codegen(ctx);

        Builder.CreateBr(MergeBB);

        // Emit else block.
        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);

        if( i == stmts.size() - 1 ) {
            if( elStmt )
                elStmt->codegen(ctx);

            Builder.CreateBr(MergeBB);
            // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
            ElseBB = Builder.GetInsertBlock();
        }

    }
    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
}

void CondStmt::collectAlloc(Context *ctx) {
    for( size_t i = 0; i < stmts.size(); i++ )
        stmts[i].second->collectAlloc(ctx);

    if( elStmt )
        elStmt->collectAlloc(ctx);
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
    for( size_t i = 0; i < valuesl.size(); i++ ) {
        phi->addIncoming(valuesl[i],blocks[i]);
    }
    ctx->bindValue(target[0], new MValue{ 0, phi });
    return;
}

void StatementList::codegen(Context *ctx) {
    for( auto *s : *this )
        s->codegen(ctx);
}

void StatementList::collectAlloc(Context *ctx) {
    for( auto *s : *this )
        s->collectAlloc(ctx);
}