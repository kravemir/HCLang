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
#include "system.h"

#include "slot.h"

using namespace llvm;

MValueType *SystemType::getChildType(std::string name) {
    int i = 0;
    for(; i < variables.size(); i++)
        if(variables[i].first == name) break;
    if(i != variables.size())
        return variables[i].second;

    i = -1;
    for(auto v : slotIds)
        if(v.first == name) {
            i = v.second;
            break;
        }
    if(i != -1)
        return slotTypes[i];
    assert(0);
    return 0;
}

MValue* SystemType::getChild(MValue *src, std::string name) {
    int i = 0;
    for(; i < variables.size(); i++)
        if(variables[i].first == name) break;
    if(i != variables.size()) {
        Value* valPtr = Builder.CreateGEP(
                            //src->type->llvmType(),
                            src->value(),
        std::vector<llvm::Value*> ( {
            Constant::getNullValue(IntegerType::getInt64Ty(lctx)),
            ConstantInt::get(lctx,APInt((unsigned) 32, (uint64_t) i+1)),
        })
                        );
        return new MValue(variables[i].second,valPtr,true);
    }

    i = -1;
    for(auto v : slotIds)
        if(v.first == name) {
            i = v.second;
            break;
        }
    if(i != -1) {
        Value *val_with_system = Builder.CreateInsertValue(
            UndefValue::get(slotTypes[i]->llvmType()),
            src->value(),
            { 0 },
            "slotref.system"
        );
        Value *val_with_msgid = Builder.CreateInsertValue(
            val_with_system,
            ConstantInt::get(lctx, APInt(32, (uint64_t) i)),
            { 1 },
            "slotref.msgid"
        );
        val_with_msgid->dump();
        return new MValue(slotTypes[i],val_with_msgid,false);
    }

    assert(0 && "Can't find");

    return 0;
}

Function* SystemDecl::codegen_msghandler(Context *ctx) {
    std::vector<llvm::Type*> aargs;
    aargs.push_back(Type::getInt8PtrTy(getGlobalContext()));
    aargs.push_back(Type::getInt32Ty(getGlobalContext()));
    aargs.push_back(Type::getInt8PtrTy(getGlobalContext()));
    FunctionType *FFT = FunctionType::get(Type::getVoidTy(getGlobalContext()), aargs, false);

    std::string full_name = ctx->storage->prefix + "_msg__";
    Function *F = Function::Create(FFT,Function::PrivateLinkage,full_name, ctx->storage->module);

    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);
    LLVMContext &lctx = getGlobalContext();

    GlobalVariable *msg_functions;
    {
        std::vector<llvm::Type*> args;
        args.push_back(Type::getInt8PtrTy(getGlobalContext()));
        llvm::FunctionType *FT = llvm::FunctionType::get(
                                     llvm::Type::getVoidTy(llvm::getGlobalContext()), args, false);

        std::vector<Constant*> ccc;
        for(auto f : ctx->storage->system->slots)
            ccc.push_back(f);


        ArrayType *at = ArrayType::get(PointerType::get(FT,0),ccc.size());
        Constant *msg_const = ConstantArray::get(at,ccc);

        msg_functions = new GlobalVariable(
            *ctx->storage->module,
            at,
            true,
            llvm::GlobalValue::PrivateLinkage,
            msg_const,
            "." + name + ".msg_funcs"
        );

        Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));
        std::vector<llvm::Value*> indices(1,zero);
        indices.push_back(++ (F->arg_begin()));
        Value *fn_ref = Builder.CreateGEP(/*msg_functions->getType(),*/msg_functions, indices);
        Value *fn_load = Builder.CreateLoad(fn_ref);
        Builder.CreateCall(fn_load, std::vector<Value*> ( {F->arg_begin(), ++(++(F->arg_begin())) }));
    }

    Builder.CreateRetVoid();

    return F;
}
void SystemDecl::codegen(Context *_ctx) {
    SystemType *t = new SystemType;
    // TODO system type instance
    _ctx->bindValue(name,new MValue( {t, (llvm::Value*)1}));
    _ctx->storage->types[name] = t;

    SystemContext ctx(_ctx, name, t);


    for(Statement *s : *stmts)
        s->collectSystemDecl(&ctx);

    std::vector<Type*> types( { ctx.storage->module->getTypeByName("struct.System") });
    for(auto v : t->variables) {
        types.push_back(v.second->llvmType());
    }
    StructType *systemType = StructType::create(lctx,types,"system." + name + ".Instance");
    Type* systemPtrType = PointerType::get(systemType,0);

    t->_llvmType = systemPtrType;
    systemType->dump();
    systemPtrType->dump();

    for(Statement *s : *stmts)
        s->codegen(&ctx);

    Function *FF = codegen_msghandler(&ctx);

    GlobalVariable *vtable;

    {
        Constant *contable_const = ConstantStruct::get(
                                       ctx.storage->module->getTypeByName("struct.ConnectionVtable"),
                                       Constant::getNullValue(Type::getInt8PtrTy(getGlobalContext())),
                                       Constant::getNullValue(Type::getInt8PtrTy(getGlobalContext())),
                                       nullptr
                                   );
        Constant *format_const = ConstantStruct::get(
                                     ctx.storage->module->getTypeByName("struct.SystemVtable"),
                                     contable_const,
                                     //Constant::getNullValue(Type::getInt8PtrTy(getGlobalContext())),
                                     FF,
                                     nullptr
                                 );
        if(ctx.storage->module->getTypeByName("struct.ConnectionVtable") == 0) return;
        vtable = new GlobalVariable(
            *ctx.storage->module,
            ctx.storage->module->getTypeByName("struct.SystemVtable"),
            true,
            llvm::GlobalValue::PrivateLinkage,
            format_const,
            "system." + name + ".vtable"
        );
    }



    FunctionType *FT_system_new = FunctionType::get(systemPtrType, { Type::getInt8PtrTy(lctx) }, false);
    Function *F = Function::Create(FT_system_new, Function::ExternalLinkage, ctx.storage->prefix + "_new__", ctx.storage->module);
    t->fn_new = F;

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);

    Function *fmalloc = ctx.storage->module->getFunction("malloc");
    std::vector<Value *> fmalloc_args( {ConstantInt::get(lctx, APInt(64,
                                        (uint64_t) ctx.storage->module->getDataLayout().getTypeAllocSize(systemType)
                                                                    ))
                                       });
    Value *system_instance = Builder.CreateBitCast(
                                 Builder.CreateCall(fmalloc,fmalloc_args),
                                 systemPtrType,
                                 "new_instance"
                             );

    Function *finit = ctx.storage->module->getFunction("system_init");
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));
    std::vector<llvm::Value*> indices(2,zero);
    std::vector<Value *> init_args( {
        system_instance,
        F->arg_begin(),
        Builder.CreateGEP(vtable,indices)
    });
    Builder.CreateCall(finit,init_args);
    Builder.CreateRet(system_instance);
}

