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
#include "binop.h"

using namespace llvm;

MValueType* BinaryOpAST::calculateType(Context* ctx) {
    // TODO: calculateType, check for type...
    return left->calculateType(ctx);
}

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
            res = Builder.CreateICmpULT(l->value(), r->value()); break;
        case Token::GREATER:
            res = Builder.CreateICmpUGT(l->value(), r->value()); break;
        default:
            assert(0);
            return 0;
        }
        return new MValue({new IntType(llvm::Type::getInt64Ty(ctx->storage->module->getContext())), res});
    }

    if( dynamic_cast<StringType*>(l->type) && dynamic_cast<StringType*>(r->type) ) {
        Function *fconcat = ctx->storage->module->getFunction("strcata");
        if( fconcat == 0 )
            std::cerr << "concant not found\n";

        std::vector<llvm::Value*> aadices({
            l->value(), r->value()
        });
        return new MValue({new StringType(llvm::Type::getInt8PtrTy(ctx->storage->module->getContext())),Builder.CreateCall(fconcat,aadices)});
    }
    assert(0);
    return 0;
}

