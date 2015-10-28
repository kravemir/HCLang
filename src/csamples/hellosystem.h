#ifndef HCLANG_CEXAMPLES
#define HCLANG_CEXAMPLES

#include "../base.hpp"

typedef struct HelloSystem {
    System system;
    int counter;
} HelloSystem;

extern "C" {

HelloSystem* hellosystem_new(Executor *e);

}

#endif // HCLANG_CEXAMPLES
