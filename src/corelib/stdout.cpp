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
 *
 */
#include "stdout.h"

#include <stdlib.h>
#include <stdio.h>
#include <ast/declarations/slot.h>

#include "../ast/declarations/system.h"

using namespace llvm;

extern "C" {

void stdoutSystem_processMsg(System *s, int msg_id, void *data) {
    switch (msg_id) {
        case 0:
            printf("%s\n",*(char**)data);
            break;
        case 1:
            printf("%s\n", *(char**)data);
            break;
        default:
            printf("wrong message\n");
            break;
    }
}

static
SystemVtable vtable = {
        { 0, 0 },
        stdoutSystem_processMsg
};

System* stdout_new(Executor *e) {
    StdOutSystem *s = (StdOutSystem*)malloc(sizeof(StdOutSystem));
    system_init(&s->system,e, &vtable);
    s->counter = 0;
    return &s->system;
}

}

#include "../ast/types/tuple.h"

void stdout_register(Context *ctx) {
    SystemType *t = new SystemType;
    // TODO system type instance
    ctx->bindValue("StdOut",new MValue( {t, (llvm::Value*)1}));
    ctx->storage->types["StdOut"] = t;

    StructType *systemType = ctx->storage->module->getTypeByName("struct.StdOutSystem");
    t->_llvmType = PointerType::get(systemType,0);
    t->fn_new = ctx->storage->module->getFunction("stdout_new");


    TupleType *argsType = TupleType::create({{"msg", StringType::create(ctx)}});
    t->slotIds["println"] = 0;
    t->slotTypes.push_back(SlotType::create(ctx, argsType, 0));
}
