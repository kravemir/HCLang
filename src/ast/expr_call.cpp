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
#include "expr_call.h"

#include "slot.h"

using namespace llvm;

MValueType* CallExpr::calculateType(Context *ctx) {
    MValueType *ft = val->calculateType(ctx);
    return ft->callReturnType();
};
void CallExpr::preCodegen(Context *ctx) {
    val->preCodegen(ctx);
    MValueType *ft = val->calculateType(ctx);
    assert(ft);

    SlotType *st = dynamic_cast<SlotType *>(ft);
    if (st) {
        Function *TheFunction = Builder.GetInsertBlock()->getParent();
        BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "await_continue");
        int awaitId = ctx->addAwaitId(BB);
        MValue *ma = val->codegen(ctx);
        Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(lctx));
        args->preCodegen(ctx);

        MValue *v_args = args->codegen(ctx,st->argsTupleType);

        TupleType *argtype = TupleType::create({ {"val",st->returnType} });
        SlotType *type = SlotType::create(ctx,argtype,0);
        Value *self = TheFunction->arg_begin();
        Value *val_with_system = Builder.CreateInsertValue(
                UndefValue::get(type->llvmType()),
                self,
                { 0 },
                "slotref.system"
        );
        Value *val_with_msgid = Builder.CreateInsertValue(
                val_with_system,
                ConstantInt::get(lctx, APInt(32, (uint64_t) awaitId)),
                { 1 },
                "slotref.msgid"
        );
        Builder.CreateStore(
                val_with_msgid,
                Builder.CreateGEP(v_args->value(), {
                        ConstantInt::get(lctx,APInt(32,(uint64_t)0)),
                        ConstantInt::get(lctx,APInt(32,(uint64_t)args->values->size()))
                })
        );


        Function *finit = ctx->storage->module->getFunction("system_putMsg");
        ma->value()->dump();
        std::vector<llvm::Value*> aadices(
                {
                        Builder.CreateExtractValue(ma->value(), {0}, "await_send_system"),
                        Builder.CreateExtractValue(ma->value(), {1}, "await_send_slot"),
                        v_args ? v_args->value() : zero
                });
        Builder.CreateCall(finit,aadices)->dump();
        Builder.CreateRetVoid();

        TheFunction->getBasicBlockList().push_back(BB);
        Builder.SetInsertPoint(BB);

        Value *retVal = Builder.CreateLoad(
                Builder.CreateGEP(
                        Builder.CreateBitCast(++(++(TheFunction->arg_begin())), argtype->llvmType()),
                        std::vector<llvm::Value*>(2,zero)
                )
        );
        asyncCallResult = new MValue(st->returnType,retVal);

        precodegenVarId = 0;
    } else {

    }
}
MValue* CallExpr::codegen(Context *ctx, MValueType *type) {
    MValue *v = val->codegen(ctx);
    MValueType *ft = v->type;
    std::vector<Value *> argsV;
    for (int i = 0; i < args->size(); i++) {
        auto a = args->get(i);
        argsV.push_back(a->codegen(ctx)->value());
    }

    SlotType *st = dynamic_cast<SlotType *>(ft);
    if (st) {
        assert( precodegenVarId != -1 );
        return asyncCallResult;
    } else {
        if (!ft->callable) {
            std::cerr << "Can't use as function\n" << std::endl;
            assert( 0 );
        }
        MValue *val = new MValue({ft->callReturnType(), Builder.CreateCall(v->value(), argsV, "calltmp")});
        if (type)
            return type->createCast(ctx, val);
        return val;
    }
}