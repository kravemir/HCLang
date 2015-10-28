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
