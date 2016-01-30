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
#ifndef HCLANG_BASE_H
#define HCLANG_BASE_H

#ifndef EXPORTLL
#include <stdlib.h>
#include <stdio.h>

#ifndef MY_INLINE
#define MY_INLINE inline
#endif
#else
int printf(const char * restrict format, ... );
#define MY_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Connection Connection;
typedef struct ConnectionVtable ConnectionVtable;
typedef struct Executor Executor;
typedef struct System System;
typedef struct SystemVtable SystemVtable;
typedef struct MessageQueue MessageQueue;
typedef struct MessageQueueItem MessageQueueItem;
typedef struct SystemQueue SystemQueue;
typedef struct SystemQueueItem SystemQueueItem;

// SYSTEMS AND CONNECTIONS

struct ConnectionVtable {
    void (*destruct)(Connection *c);
    void (*putMsg)(Connection *c, int id, void *data);
};

struct Connection {
    ConnectionVtable *vtable;
    int refCounter;
};

struct MessageQueueItem {
    int msg_id;
    void *data;

    MessageQueueItem *next;
};

struct MessageQueue {
    MessageQueueItem *first, *last;
};

struct SystemVtable {
    ConnectionVtable contable;

    void (*processMsg)(System *s, int msg_id, void *msg_data);
};

struct SlotReference {
    System *system;
    int msg_id;
};

// direct connection, can be undirect - over network - not a system instance
// or undirect - horizontal inheritance of interfaces - different vtable
struct System {
    Connection connection;
    Executor *executor;
    MessageQueue queue;
};

// EXECUTOR INTERFACE
struct Executor {
    void (*run)(Executor *self);
    void (*putWork)(Executor *self, System *work);
};

// CONNECION INTERFACE
void connection_reference(Connection *c);
void connection_dereference(Connection *c);
void connection_putMsg(Connection *c, int msg_id, void *data);

void system_putMsg(System *s, int msg_id, void *data );

char messageQueue_putItem( MessageQueue *q, int msg_id, void *data );

Executor* stexecutor_new();
Executor* mtexecutor_new();

extern Executor *executor;

void system_init(
        System *s, 
        Executor *e,
        SystemVtable *vtable
    );
void system_executeWork(System *s);

#ifndef malloc
void* malloc(unsigned long s);
#endif

MY_INLINE void connection_reference(Connection *c) {
    c->refCounter += 1;
}
MY_INLINE void connection_dereference(Connection *c) {
    c->refCounter--;
    if(c->refCounter == 0)
        c->vtable->destruct(c);
}
MY_INLINE void connection_putMsg(Connection *c, int msg_id, void *data) {
    c->vtable->putMsg(c,msg_id,data);
}

MY_INLINE char messageQueue_putItem( MessageQueue *q, int msg_id, void *data ) {
    MessageQueueItem *i = (MessageQueueItem*)malloc(sizeof(MessageQueueItem));
    i->msg_id = msg_id;
    i->data = data;
    i->next = 0;
    if(q->first == 0) {
        q->first = q->last = i;
        return 1;
    } else  {
        q->last->next = i;
        q->last = i;
        return 0;
    }
}

System* stdin_new(Executor *e);
System* stdout_new(Executor *e);


#ifndef __cplusplus
char *
strcata(char *a, const char *b)
{
    int i, j;
    int len = 0;
    for (i = 0; a[i] != '\0'; i++, len++);
    for (j = 0; b[j] != '\0'; j++, len++);
    len += 1;
    char* r = (char*)malloc(len);

    for (i = 0; a[i] != '\0'; i++)
        r[i] = a[i];
    for (j = 0; b[j] != '\0'; j++)
        r[i+j] = b[j];
    r[i+j] = '\0';
    return r;
}
#endif

void __protector__(SystemVtable *vtable);
MY_INLINE void __protector__(SystemVtable *vtable) {
    vtable->processMsg(0,0,0);
    system_init(0,0,0);
    executor = 0;
    stdin_new(executor);
    stdout_new(executor);
    system_putMsg(0,0,0);
}

#ifdef __cplusplus
}
#endif

#endif // HCLANG_BASE_H

