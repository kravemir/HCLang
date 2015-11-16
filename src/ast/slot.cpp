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

#include "tuple.h"
#include "system.h"

using namespace llvm;

SlotType* SlotType::create ( Context *ctx, TupleType* argsType ) {
    StructType *st = StructType::create ( lctx, {
        Type::getInt32Ty ( ctx->storage->module->getContext() ),
        argsType->llvmType()
    } );
    SlotType * type = new SlotType ( st, argsType );
    return type;
}

MValueType* SlotTypeAST::codegen ( Context* ctx ) {
    return SlotType::create ( ctx, args->codegen ( ctx ) );
}

void SlotDecl::codegen ( Context *_ctx ) {
    Context ctx ( _ctx );
    std::vector<llvm::Type*> args;
    args.push_back ( ctx.storage->system->llvmType() );
    args.push_back ( this->args->codegen ( &ctx )->llvmType() );
    llvm::FunctionType *FT = llvm::FunctionType::get (
                                 llvm::Type::getVoidTy ( llvm::getGlobalContext() ), args, false );

    Function *F = Function::Create ( FT, Function::PrivateLinkage, ctx.storage->prefix + name, ctx.storage->module );

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create ( getGlobalContext(), "entry", F );
    Builder.SetInsertPoint ( BB );

    ctx.bindValue ( "self", new MValue ( { ctx.storage->system, F->arg_begin() } ) );
    for ( int i = 0; i < this->args->namedValues.size(); i++ ) {
        auto v = this->args->namedValues[i];
        std::vector<llvm::Value*> args ( {
            ConstantInt::get ( lctx,APInt ( ( unsigned ) 32, ( uint64_t ) 0 ) ),
            ConstantInt::get ( lctx,APInt ( ( unsigned ) 32, ( uint64_t ) i ) ),
        } );
        Value *valPtr = Builder.CreateGEP ( ++ ( F->arg_begin() ), args );
        auto type = v.second->codegen ( &ctx );
        ctx.bindValue ( v.first,new MValue ( { type, Builder.CreateLoad ( valPtr ) } ) );
    }


    for ( Statement *stmt : *stmts )
        stmt->codegen ( &ctx );

    Builder.CreateRetVoid();
    SystemType *s = ctx.storage->system;
    s->slots.push_back ( F );
}
void SlotDecl::collectSystemDecl ( Context *ctx ) const {
    SystemType *s = ctx->storage->system;
    s->slotIds[name] = s->slotCount++;
}
