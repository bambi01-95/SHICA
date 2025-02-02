#ifndef EXGC_C
#define EXGC_C

#include "./msgc.c"

/*
use own memory management
in SHICA, gc_extern_alloc is used to allocate memory for unsupporting OBJECT
such as FILE, SOCKET, etc.

so, you can use malloc, calloc, realloc, free, etc.

*/
struct ExternMemory{
    void **memory;
    int size;
    int capacity;
};

struct ExternMemory *newExternMemory(int size){
    struct ExternMemory *em = gc_alloc(sizeof(struct ExternMemory));
    em->memory = gc_alloc(sizeof(void*)*size);
    em->size = size;
    return em;
}

void gc_markExternMemory(struct ExternMemory *em){
    gc_markOnly(em);
    gc_markOnly(em->memory);
    for(int i=0;i<em->size;i++){
        gc_markOnly(em->memory[i]);
    }
}

void *gc_extern_alloc(struct ExternMemory *em, int lbs)
{
    if(em->size + lbs < em->capacity){
        em->memory = gc_realloc(em->memory,em->capacity*2);
        em->capacity *= 2;
    }
    void *p = gc_alloc(lbs);
    em->memory[em->size] = p;
    return p;
}

#endif