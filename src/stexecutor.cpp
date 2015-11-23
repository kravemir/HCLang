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
#include "base.hpp"

#include <cstdio>
#include <queue>

// Single Threaded executor implementation

struct STExecutor : Executor {
    std::queue<System*> queue;
};

static
void stexecutor_mainloop(STExecutor *e) {
    System *s;

    while(!e->queue.empty()) {
        s = e->queue.front();
        e->queue.pop();

        system_executeWork(s);
    }
}

static
void stexecutor_putWork(STExecutor *e, System *s) {
    e->queue.push(s);
}

extern "C" {
Executor* stexecutor_new() {
    STExecutor *e = (STExecutor*)malloc(sizeof(STExecutor));
    new (e) STExecutor();
    e->run = (void(*)(Executor*))stexecutor_mainloop;
    e->putWork = (void(*)(Executor*,System*))stexecutor_putWork;
    return e;
}
}

