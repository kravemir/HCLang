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

#include <stdio.h>

extern "C" {

inline
bool messagequeue_executeAndPop(System *s, MessageQueue *q) {
    fflush(stdout);
    if( q->first == 0 ) 
        return 0;

    MessageQueueItem *i = q->first;
    SystemVtable *vtable = (SystemVtable*)s->connection.vtable;
    vtable->processMsg(s, i->msg_id, i->data);
    if(i->next) {
        q->first = i->next;
        return true;
    } else {
        q->first = q->last = 0;
        return false;
    }
}

inline 
System* systemqueue_pop(SystemQueue *q) {
    if( q->first == 0 ) 
        return 0;

    SystemQueueItem *i = q->first;
    if(i->next)
        q->first = i->next;
    else
        q->first = q->last = 0;

    return i->system;
}


void system_init(
        System *s, 
        Executor *e,
        SystemVtable *vtable ){
    s->executor = e;
    s->queue.first = s->queue.last = 0;
    s->connection.vtable = (ConnectionVtable*)vtable;
}

Executor* executor_new() {
    Executor *e = (Executor*)malloc(sizeof(Executor*));
    e->queue.first = e->queue.last = 0;
    return e;
}
void executor_mainloop(Executor *e) {
    System *s;
    
    while( (s = systemqueue_pop( &e->queue )) != 0 ) {
        //printf("processing %p\n", s);
        while( messagequeue_executeAndPop( s, &s->queue ));
    }
}


void system_putMsg(System *s, int msg_id, void *data ) {
    //printf("putting msg %p %d\n", s, msg_id);
    //fflush(stdout);
    if(messageQueue_putItem(&s->queue, msg_id, data)) {
        systemQueue_putItem(&s->executor->queue, s);
    }
}

}

