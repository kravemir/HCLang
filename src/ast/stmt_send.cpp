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
#include "stmt_send.h"

using namespace llvm;

#include "tuple.h"
#include "system.h"
#include "slot.h"

#include "llvm/ADT/ArrayRef.h"

void SendStmt::codegen(Context *ctx) {
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));

    if(target[0] == "stdout"  ) { // TODO
        Function *printf_func = ctx->storage->module->getFunction("printf");
        Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));
        if( msg == "println" ) {
            MValue *val = args->get(0)->codegen(ctx);
            Builder.CreateCall(printf_func, val->value());
        } else {
            std::vector<Value*> argsv;
            for( int i = 0; i < args->size(); i++ ) {
                args->get(i)->preCodegen(ctx);
            }
            for( int i = 0; i < args->size(); i++ ) {
                argsv.push_back(args->get(i)->codegen(ctx)->value());
            }
            Builder.CreateCall(printf_func,argsv);
        }


        Constant *format_const = ConstantDataArray::getString(lctx, "\n" );
        GlobalVariable *var = new GlobalVariable(
                *ctx->storage->module,
                llvm::ArrayType::get(llvm::IntegerType::get(lctx, 8), 2),
                true,
                llvm::GlobalValue::PrivateLinkage,
                format_const,
                ".str"
            );
        std::vector<llvm::Constant*> indices(2,zero);
        Builder.CreateCall(printf_func, ConstantExpr::getGetElementPtr(
                    ArrayType::get(llvm::IntegerType::get(lctx, 8), 2),
                    var,
                    indices));
    } else {
        MValue *ma = ctx->getVariable(target[0]); // TODO
        for(int i = 1; i < target.size(); i++ ) {
            ma = ma->type->getChild(ma,target[i]);
        }
        if(!ma) {
            std::cerr << "Can't find connection of `" << target[0] << "`\n";
        } else {
            SystemType *system_type = dynamic_cast<SystemType*>(ma->type);
            SlotType *slot_type = dynamic_cast<SlotType*>(ma->type);

            if(system_type) {
                args->preCodegen(ctx);
                MValue *v_args = args->codegen(ctx);

                Function *finit = ctx->storage->module->getFunction("system_putMsg");
                std::vector<llvm::Value*> aadices({
                    ma->value(),
                    ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)system_type->slotIds[msg])),
                    v_args ? v_args->value() : zero
                });
                Builder.CreateCall(finit,aadices);
            } else {
                args->preCodegen(ctx);
                MValue *v_args = args->codegen(ctx);

                ma->value()->dump();
                Function *finit = ctx->storage->module->getFunction("system_putMsg");
                std::vector<llvm::Value*> aadices({
                    Builder.CreateLoad(Builder.CreateGEP(ma->value(), { (Value*)ConstantInt::get(lctx,APInt(32,(uint64_t)0)), (Value*)ConstantInt::get(lctx,APInt(32,(uint64_t)0))})),
                    Builder.CreateLoad(Builder.CreateGEP(ma->value(), { (Value*)ConstantInt::get(lctx,APInt(32,(uint64_t)0)), (Value*)ConstantInt::get(lctx,APInt(32,(uint64_t)1))})),
                    v_args ? v_args->value() : zero
                });
                Builder.CreateCall(finit,aadices);
            }
        }
    }
}

void SendStmt::collectAlloc(Context* ctx) {

}

