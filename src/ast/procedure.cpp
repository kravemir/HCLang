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
    
    std::vector<std::pair<std::string,MValueType*> > args_types;
    std::vector<llvm::Type*> args_llvmtypes;
    for( auto &v : this->args->namedValues ) {
        auto t = v.second->codegen(&ctx);
        args_types.push_back( {v.first,t} );
        args_llvmtypes.push_back(t->llvmType());
    }

    if( async == false ) {
        llvm::FunctionType *FT = llvm::FunctionType::get(returnType->llvmType(), args_llvmtypes, false);
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
            ctx.bindValue(v.first,new MValue({ args_types[i].second, it}));
        }
        
        for(Statement *stmt : *stmts)
            stmt->codegen(&ctx);

        Builder.CreateRetVoid();

        F->dump();
    } else {
        TupleType *args_tuple_type = TupleType::create(args_types, name + ".args_tuple" );
        ((PointerType*)args_tuple_type->llvmType())->getTypeAtIndex((unsigned)0)->dump();
        args_tuple_type->llvmType()->dump();
        
        
        std::vector<Type*> types({ ctx.storage->module->getTypeByName("struct.System") });
        /*for( auto v : t->variables ) {
            types.push_back(v.second->llvmType());
        }*/
        StructType *systemType = StructType::create(lctx,types,name + ".instance");
        Type* systemPtrType = PointerType::get(systemType,0);
        
        // TODO native type
        ProcedureAsyncInstanceType *instType = new ProcedureAsyncInstanceType(systemPtrType);
        
        
        FunctionType *msg_ft = FunctionType::get(
            Type::getVoidTy(ctx.storage->module->getContext()), 
            {
                Type::getInt8PtrTy(getGlobalContext()),
                Type::getInt32Ty(getGlobalContext()),  
                args_tuple_type->llvmType()
            }, 
            false
        );
        Function *process_fn = Function::Create(msg_ft, Function::InternalLinkage, ctx.storage->prefix + name + ".fn", ctx.storage->module);
        
        FunctionType *create_ft = llvm::FunctionType::get(instType->llvmType(), { Type::getInt8PtrTy(lctx) }, false);
        Function *create_fn = Function::Create(create_ft, Function::ExternalLinkage, ctx.storage->prefix + name  + ".constructor", ctx.storage->module);
        
        ProcedureAsyncType *ptype = new ProcedureAsyncType(create_ft,instType);
        _ctx->bindValue(name, new MValue(ptype,create_fn));
        
        {
            auto *ft = new ProcedureType(msg_ft,returnType);

            // Create a new basic block to start insertion into.
            BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", process_fn);
            Builder.SetInsertPoint(BB);

            for( int i = 0; i < this->args->namedValues.size(); i++ ) {
                auto v = this->args->namedValues[i];
                Value *valPtr = Builder.CreateGEP(
                    ++(++(process_fn->arg_begin())),
                    std::vector<llvm::Value*>({
                        ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)0)),
                        ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)i)),
                    })
                );
                ctx.bindValue(v.first,new MValue({ args_types[i].second, Builder.CreateLoad(valPtr, v.first)}));
            }
            
            for(Statement *stmt : *stmts)
                stmt->codegen(&ctx);

            Builder.CreateRetVoid();

            process_fn->dump();
        }
        
        GlobalVariable *vtable;

        {
            Constant *contable_const = ConstantStruct::get(
                ctx.storage->module->getTypeByName("struct.ConnectionVtable"),
                Constant::getNullValue(Type::getInt8PtrTy(getGlobalContext())),
                Constant::getNullValue(Type::getInt8PtrTy(getGlobalContext())),
                0
            );
            Constant *format_const = ConstantStruct::get(
                ctx.storage->module->getTypeByName("struct.SystemVtable"),
                contable_const,
                //Constant::getNullValue(Type::getInt8PtrTy(getGlobalContext())),
                process_fn,
                0
            );
            if(ctx.storage->module->getTypeByName("struct.ConnectionVtable") == 0) return;
            vtable = new GlobalVariable(
                *ctx.storage->module, 
                ctx.storage->module->getTypeByName("struct.SystemVtable"),
                true, 
                llvm::GlobalValue::PrivateLinkage, 
                format_const, 
                ctx.storage->prefix + name  + ".vtable"
            );
            vtable->dump();
        }
        
        {
            
            // Create a new basic block to start insertion into.
            BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", create_fn);
            Builder.SetInsertPoint(BB);

            Function *fmalloc = ctx.storage->module->getFunction("malloc");
            std::vector<Value *> fmalloc_args({ConstantInt::get(lctx, APInt(64,
                (uint64_t)ctx.storage->module->getDataLayout().getTypeAllocSize(systemType)
            ))});
            Value *system_instance = Builder.CreateBitCast(
                    Builder.CreateCall(fmalloc,fmalloc_args),
                    instType->llvmType(),
                    "new_instance"
                );

            Function *finit = ctx.storage->module->getFunction("system_init");
            Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));
            std::vector<llvm::Value*> indices(2,zero);
            std::vector<Value *> init_args( {
                system_instance,
                create_fn->arg_begin(),
                Builder.CreateGEP(vtable,indices)
            });
            Builder.CreateCall(finit,init_args);
            Builder.CreateRet(system_instance);
            
            create_fn->dump();
        }
    }
}
