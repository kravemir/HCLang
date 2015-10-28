#include "stmt_send.h"

using namespace llvm;

#include "tuple.h"
#include "system.h"

#include "llvm/ADT/ArrayRef.h"

void SendStmt::codegen(Context *ctx) {
    LLVMContext &lctx = getGlobalContext();
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
            SystemType *t = dynamic_cast<SystemType*>(ma->type);
            MValue *v_args = args->codegen(ctx);

            if( !t ) {
                std::cerr << "Can't use as system\n" << std::endl;
                return;
            }

            Function *finit = ctx->storage->module->getFunction("system_putMsg");
            std::vector<llvm::Value*> aadices({
                ma->value(),
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)t->slotIds[msg])),
                v_args ? v_args->value() : zero
            });
            Builder.CreateCall(finit,aadices);
        }
    }
}
