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
        printf("processing %p\n", s);
        while( messagequeue_executeAndPop( s, &s->queue ));
    }
}

}

