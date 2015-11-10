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
#include "hellosystem.h"

#include <stdlib.h>
#include <stdio.h>

extern "C" {

void helloSystem_processMsg(System *s, int msg_id, void *data) {
    printf("MSG: %d\n",msg_id);
    if( msg_id == 0) {
        system_putMsg( s, 4, 0);
        system_putMsg( s, 7, 0);
    }
}

static
SystemVtable vtable = {
    { 0, 0 },
    helloSystem_processMsg
};

HelloSystem* hellosystem_new(Executor *e) {
    HelloSystem *s = (HelloSystem*)malloc(sizeof(HelloSystem));
    system_init(&s->system,e, &vtable);
    s->counter = 0;
    return s;
}

}
