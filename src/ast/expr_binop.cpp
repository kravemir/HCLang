#include "expr_binop.h"

using namespace llvm;

MValue* BinaryOpAST::codegen(Context *ctx, MValueType *type) {
    MValue *l = left->codegen(ctx);
    MValue *r = right->codegen(ctx);
    if( dynamic_cast<IntType*>(l->type) && dynamic_cast<IntType*>(r->type) ) {
        Value *res;
        switch(op) {
        case Token::PLUS:
            res = Builder.CreateAdd(l->value(), r->value()); break;
        case Token::MINUS:
            res = Builder.CreateSub(l->value(), r->value()); break;
        case Token::LESS:
            res = Builder.CreateICmpULE(l->value(), r->value()); break;
        case Token::GREATER:
            res = Builder.CreateICmpUGE(l->value(), r->value()); break;
        default:
            assert(0);
            return 0;
        }
        return new MValue({new IntType(), res});
    }

    if( dynamic_cast<StringType*>(l->type) && dynamic_cast<StringType*>(r->type) ) {
        Function *fconcat = ctx->storage->module->getFunction("strcata");
        if( fconcat == 0 )
            std::cerr << "concant not found\n";

        std::vector<llvm::Value*> aadices({
            l->value(), r->value()
        });
        return new MValue({new StringType(),Builder.CreateCall(fconcat,aadices)});
    }
    assert(0);
    return 0;
}

