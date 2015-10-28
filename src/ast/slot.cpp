#include "slot.h"

#include "tuple.h"
#include "system.h"

using namespace llvm;

void SlotDecl::codegen(Context *_ctx) {
    Context ctx (_ctx);
    std::vector<llvm::Type*> args;
    args.push_back(ctx.storage->system->llvmType());
    args.push_back(this->args->codegen(&ctx)->llvmType());
    llvm::FunctionType *FT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvm::getGlobalContext()), args, false);

    Function *F = Function::Create(FT, Function::PrivateLinkage, ctx.storage->prefix + name, ctx.storage->module);

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);

    ctx.bindValue("self", new MValue( { ctx.storage->system, F->arg_begin()} ) );
    for( int i = 0; i < this->args->namedValues.size(); i++ ) {
        auto v = this->args->namedValues[i];
        Value *valPtr = Builder.CreateGEP(
            //ctx.storage->system->llvmType(),
            ++(F->arg_begin()),
            std::vector<llvm::Value*>({
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)0)),
                ConstantInt::get(lctx,APInt((unsigned)32,(uint64_t)i)),
            })
        );
        auto type = v.second->codegen(&ctx);
        ctx.bindValue(v.first,new MValue({ type, Builder.CreateLoad(valPtr)}));
    }

    
    for(Statement *stmt : *stmts)
        stmt->codegen(&ctx);

    Builder.CreateRetVoid();
    SystemType *s = ctx.storage->system;
    s->slots.push_back(F);
}
void SlotDecl::collectSystemDecl(Context *ctx) const {
    SystemType *s = ctx->storage->system;
    s->slotIds[name] = s->slotCount++;
}
