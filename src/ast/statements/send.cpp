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
#include "send.h"

using namespace llvm;

#include "ast/types/tuple.h"
#include "ast/declarations/system.h"
#include "ast/declarations/slot.h"

#include "llvm/ADT/ArrayRef.h"

void SendStmt::codegen(Context *ctx) {
    args->preCodegen(ctx);
    MValue *v_args = args->codegen(ctx);
    MValue *ma = ctx->getVariable(target[0]); // TODO
    for(size_t i = 1; i < target.size(); i++ ) {
        assert(ma || "Error ma");
        ma = ma->type->getChild(ma,target[i]);
    }
    if(!ma) {
        std::cerr << "Can't find connection of `" << target[0] << "`\n";
    } else {
        ma->type->codegenSendTo(ma, v_args);
    }
}

void SendStmt::collectAlloc(Context* ctx) {

}

