#include "procedure.h"

using namespace llvm;

void ProcedureDecl::codegen(Context *_ctx) {
    Context ctx (_ctx);
    std::vector<llvm::Type*> args;
    llvm::FunctionType *FT = llvm::FunctionType::get(
            llvm::Type::getVoidTy(llvm::getGlobalContext()), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, ctx.storage->prefix + name, ctx.storage->module);

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    Builder.SetInsertPoint(BB);
    
    for(Statement *stmt : *stmts)
        stmt->codegen(&ctx);

    Builder.CreateRetVoid();
}
