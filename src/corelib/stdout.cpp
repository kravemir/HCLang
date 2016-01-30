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
SystemVtable stdout_vtable = {
        { 0, 0 },
        stdoutSystem_processMsg
};

System* stdout_new(Executor *e) {
    System *s = (System*)malloc(sizeof(System));
    system_init(s,e, &stdout_vtable);
    return s;
}

void stdinSystem_processMsg(System *s, int msg_id, void *data) {
    int len;
    char buf[512], *str;
    SlotReference ref = *(SlotReference*)data;
    char **ptr;
    switch (msg_id) {
        case 0:
            fgets(buf, 512, stdin);
            str = (char *) malloc(512);
            strcpy(str, buf);
            ptr = (char **) malloc(sizeof(char*));
            *ptr = str;
            system_putMsg(ref.system,ref.msg_id,ptr);
            break;
        default:
            printf("wrong message\n");
            break;
    }
}

static
SystemVtable stdin_vtable = {
        { 0, 0 },
        stdinSystem_processMsg
};

System* stdin_new(Executor *e) {
    System *s = (System*)malloc(sizeof(System));
    system_init(s,e, &stdin_vtable);
    return s;
}


}

#include "../ast/types/tuple.h"

void stdio_register(Context *ctx) {
    {
        // TODO: connection type, not system type
        SystemType *t = new SystemType;
        // TODO system type instance
        ctx->bindValue("StdOut", new MValue({t, (llvm::Value *) 1}));
        ctx->storage->types["StdOut"] = t;

        StructType *systemType = ctx->storage->module->getTypeByName("struct.System");
        t->_llvmType = PointerType::get(systemType, 0);
        t->fn_new = ctx->storage->module->getFunction("stdout_new");


        TupleType *argsType = TupleType::create({{"msg", StringType::create(ctx)}});
        t->slotIds["println"] = 0;
        t->slotTypes.push_back(SlotType::create(ctx, argsType, 0));
    }
    {
        // TODO: connection type, not system type
        SystemType *t = new SystemType;
        // TODO system type instance
        ctx->bindValue("StdIn", new MValue({t, (llvm::Value *) 1}));
        ctx->storage->types["StdIn"] = t;

        StructType *systemType = ctx->storage->module->getTypeByName("struct.System");
        t->_llvmType = PointerType::get(systemType, 0);
        t->fn_new = ctx->storage->module->getFunction("stdin_new");

        t->slotIds["readln"] = 0;
        SlotType *rst = SlotType::create(ctx, TupleType::create( { {"val", StringType::create(ctx)} } ), 0 );
        t->slotTypes.push_back(SlotType::create(
                ctx,
                TupleType::create({ {"return_slot", rst} }),
                StringType::create(ctx)
        ));
    }
}
