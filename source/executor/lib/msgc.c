// msgc.c -- minimal mark-sweep garbage collector to illustrate the concepts
//
// (C) 2024 Ian Piumarta
//
// This Source Code is subject to the terms of the Mozilla Public License,
// version 2.0, available at: https://mozilla.org/MPL/2.0/
#include "../setting.h"

#ifndef __gc_c
#define __gc_c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fatal.c"

#if 1
# define gc_debug	if (0)
#else
# define gc_debug
#endif

typedef struct gc_header gc_header;

/*
int => 4fdasdf fdf
unsinged name:x => xbit
*/

struct gc_header
{
    unsigned int size;		// size of this block, including header
    unsigned 	 busy : 1;	// this block is busy (reachable)
    unsigned 	 mark : 1;	// this block is marked (reachable) during GC
    unsigned 	 atom : 1;	// the data stored in this block contains no pointers
    // unsigned padding : 29; // アライメント調整
};

gc_header *gc_memory  = 0;	// address of start of memory
gc_header *gc_memend  = 0;	// address of end (first byte after) memory
gc_header *gc_memnext = 0;	// next block to consider when allocating

void gc_init(int size)
{
#if SBC
    gc_memory  = malloc(size);
    gc_memend  = (void *)gc_memory + size;
#else //C++
    gc_memory  = (gc_header*)malloc(size);
    gc_memend = (gc_header*)((char*)gc_memory + size);
#endif
    gc_memnext = gc_memory;
    // memory begins as a single free block the size of the entire memory
    gc_memory->size = size;
    gc_memory->busy = 0;
    gc_memory->mark = 0;
    gc_memory->atom = 0;
}

// roots contains the addresses of all variables that hold a reference to an object

#define MAXROOTS 1024

void **roots[MAXROOTS];	// pointers to the variables pointing to objects
int   nroots = 0;	// number of variables addresses in the roots stack

void gc_pushRoot(const void *varp)	// push a new variable address onto the root stack
{
    if (nroots == MAXROOTS) {
#if SBC
        fatal("gc root table full");
#else
        SHICA_PRINTF("gc root table full");
#endif
    }
    roots[nroots++] = (void **)varp;
}

// macro to declare, initialise, and push the address of an object pointer variable on the root stack

#define GC_PUSH(TYPE, VAR, INIT)		\
    TYPE VAR = INIT;				\
    gc_pushRoot((void *)&VAR)
#ifdef NDEBUG

void gc_popRoot(void)	// remove the topmost variable address from the root stack
{
    --nroots;
}

#define GC_POP(VAR)				\
    gc_popRoot()

#else // !NDEBUG -- enforce LIFO popping of roots

void gc_popRoot(void *varp,const char *name)	// pop a variable, checking it was the topmost on the stack
{
    assert(nroots > 0);
    --nroots;
    if (varp != roots[nroots]) {
    #if SBC
        fatal("GC root '%s' popped out of order",name);
    #else //C++
        SHICA_PRINTF("GC root '%s' popped out of order",name);
    #endif
    };
}

#define GC_POP(VAR)				\
    gc_popRoot((void *)&VAR, #VAR)
#endif

void gc_popRoots(int n)	// pop several variables at once
{
    assert(nroots >= n);
    nroots -= n;
}

void gc_mark(void *ptr);	// mark an object as reachable, then call the mark function...

// the mark function is run on any object immediately after it is marked, to recursively
// mark all the objects pointed to from within it

void gc_defaultMarkFunction(void *ptr)	// recursively mark all pointers stored in the object
{
    gc_header *hdr = (gc_header *)ptr - 1;	// object address to header address
    if (hdr->atom) return;			// atomic objects do not contain pointers
    #if SBC
    void *end = (void *)hdr + hdr->size;	// first address past end of object
    while (ptr < end) gc_mark(*(void **)ptr++);	// recursively mark the object's pointers
    #else //C++
    void *end = (void*)((char*)(hdr) + hdr->size);
    while (ptr < end) {
        gc_mark(*(void**)ptr); // dereference and pass to gc_mark
        ptr = (void*)((char*)ptr + sizeof(void*)); // increment pointer
    }
    #endif
}

typedef void (*gc_markFunction_t)(void *);

// the application should provide a mark function that accurately marks only valid pointers

gc_markFunction_t gc_markFunction = gc_defaultMarkFunction;
gc_markFunction_t gc_isMarkFuction = gc_defaultMarkFunction;

// the collect function is run once at the start of GC, before the roots are marked,
// to mark any pointers that are stored outside of the normal root set or in (e.g.)
// non-object arrays

void gc_defaultCollectFunction(void) {}

typedef void (*gc_collectFunction_t)(void);

gc_collectFunction_t gc_collectFunction = gc_defaultCollectFunction;


#if SBC
#define GC_PTR(P) ((void *)gc_memory <= (P) && (P) < (void *)gc_memend)
#else
#define GC_PTR(P) ((void *)gc_memory <= (void *)(P) && (void *)(P) < (void *)gc_memend)
#endif

void gc_markOnly(void *ptr)	// mark an object reachable without recursively marking its content
{
    if (!GC_PTR(ptr)) return;			// object pointer is NULL or outside memory
    gc_header *here = (gc_header *)ptr - 1;	// object address to header address
    assert(gc_memory <= here);
    assert(here < gc_memend);
    assert(here->busy);				// object must be allocated and reachable
    if (here->mark) return;			// do not mark more than once
    here->mark = 1;
}

void gc_mark(void *ptr)
{
    if (!GC_PTR(ptr)) return;			// NULL or outside memory
    gc_header *here = (gc_header *)ptr - 1;	// object to header
    assert(gc_memory <= here);
    assert(here < gc_memend);
    assert(here->busy);
    if (here->mark) return;			// stop if already marked
    here->mark = 1;
    if (here->atom) return;			// stop if atomic (no pointers)
    gc_markFunction(ptr);			// recursively mark object contents
}

//for debug
void gc_isMark(void *ptr){
    if (!GC_PTR(ptr)) return;			// NULL or outside memory
    gc_header *here = (gc_header *)ptr - 1;	// object to header
    if (here->atom) return;			// stop if atomic (no pointers)
    gc_isMarkFuction(ptr);			// recursively mark object contents
    if (here->mark) return;			// stop if already marked
    SHICA_PRINTF("line %d this is not happen, %p is not marked\n",__LINE__, ptr);
}

#if UNMARK //unmark function that is usefull
// void gc_unmark(void *ptr)
// {
//     if (!GC_PTR(ptr)) return;			// NULL or outside memory
//     gc_header *here = (gc_header *)ptr - 1;	// object to header
//     assert(gc_memory <= here);
//     assert(here < gc_memend);
//     assert(here->busy);
//     if (here->mark) return;			// stop if already marked
//     here->mark = 0;
//     if (here->atom) return;			// stop if atomic (no pointers)
//     gc_unmarkFunction(ptr);			// recursively mark object contents   
// }
void gc_unmarkOnly(void *ptr){
    if (!GC_PTR(ptr)) return;			// NULL or outside memory
    gc_header *here = (gc_header *)ptr - 1;	// object to header
    assert(gc_memory <= here);
    assert(here < gc_memend);
    assert(here->busy);
    if (here->mark) return;			// stop if already marked
    here->mark = 0;
}
#endif

int gc_collect(void)	// collect garbage
{
    // phase one: transitively trace the object graph starting at each root variable, setting
    // the mark bit in every object visited.
    // objects with the mark bit already set can be ignored since they have already been visited.

    gc_debug SHICA_PRINTF("COLLECT\n");
    int nfree = 0, nbusy = 0;			// count memory in use and free
    gc_collectFunction();			// run pre-collection function to mark static roots
    for (int i = 0;  i < nroots;  ++i){		// mark the pointers stored in each root variable
        gc_mark(*roots[i]);
    }

    // phase two: sweep the memory looking for objects that are busy but do not have their
    // mark bit set.
    // these objects are unreachable from any of the roots, cannot ever take part in future
    // computation, and so can be collected as garbage.
#if SBC
    for ( gc_header *here = gc_memory;		// iterate over all objects in memory
	  here < gc_memend;
	  here = (gc_header*)((void *)here + here->size 
      )) 
#else //C++
    for ( gc_header *here = gc_memory;		// iterate over all objects in memory
        here < gc_memend;
        here = (gc_header *)((char *)here + here->size))
#endif
    {
    #if SBC
        gc_debug SHICA_PRINTF("%p %c%c%c %d\n", (void *)here + sizeof(*here),
                here->busy ? 'B' : '-', here->atom ? 'A' : '-', here->mark ? 'M' : '-',
                here->size);
    #else //C++
    gc_debug SHICA_PRINTF("%p %c%c%c %d\n", (void*)((char*)here + sizeof(*here)),
            here->busy ? 'B' : '-', here->atom ? 'A' : '-', here->mark ? 'M' : '-',
            here->size);
    #endif

        if (here->mark) {			// block is marked reachable: do not reclaim
            here->mark = 0;
            assert(here->busy);			// if it is not allocated, the mutator has a bug
            nbusy += here->size;
            continue;
        }
        // block is not marked, is unreachable, and is therefore garbage
    #if SBC
        gc_debug SHICA_PRINTF("%p RECLAIM\n", (void *)here + sizeof(*here));
    #else //C++
        gc_debug SHICA_PRINTF("%p RECLAIM\n", (void*)((char*)here + sizeof(*here)));
    #endif
        here->busy = 0;				// reclaim the block
        here->mark = 0;
        here->atom = 0;
        for (;;) {				// coalesce all following free blocks into this one
            #if SBC
            gc_header *next = (void *)here + here->size;
            #else
            gc_header *next = (gc_header*)((char*)here + here->size);
            #endif
            if (next == gc_memend) break;	// current block is the last
            if (next->mark) break;		// next block is reachable
            assert(gc_memory < next);
            assert(next < gc_memend);
            #if SBC
            gc_debug SHICA_PRINTF("%p EXTEND %p %d\n",
                    (void *)here + sizeof(*here),
                    (void *)next + sizeof(*next),
                    next->size);
            #else //C++
            gc_debug SHICA_PRINTF("%p EXTEND %p %d\n",
                    (void*)((char*)here + sizeof(*here)),
                    (void*)((char*)next + sizeof(*next)),
                    next->size);
            #endif
            here->size += next->size;		// absorb following block into this one
        }
        nfree += here->size;
    }
    gc_memnext = gc_memory;			// start allocating at the start of memory
# ifndef NDEBUG
    SHICA_PRINTF("\r\n[GC %d used %d free]\n\r", nbusy, nfree);
    fflush(stdout);
# endif
    return nbusy;
}

unsigned long gc_total = 0;

void *gc_alloc(int lbs)	// allocate at least lbs bytes of memory
{
    gc_total += lbs;
    //gc_collect();	// uncomment this to agressively test root pushing/popping
    // round up the allocation size to a multiple of the pointer size
    int size = (lbs + sizeof(gc_header) + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
    assert(size >= sizeof(gc_header) + lbs);
    gc_header *start = gc_memnext, *here = start;	// start looking for a free block at memnext
    for (int retries = 0;  retries < 2;  ++retries) {	// try twice, before and after collecting
        do {
            #if SBC
            gc_debug SHICA_PRINTF("%p ? %i %d\n", (void *)here + sizeof(*here), here->size, here->busy);
            #else
            gc_debug SHICA_PRINTF("%p ? %i %d\n", (void*)((char*)here + sizeof(*here)), here->size, here->busy);
            #endif
            if (!here->busy && here->size >= size) {	// this block is free and large enough
                // split this block into two if the second block is large enough to hold a pointer
                if (here->size > size + sizeof(gc_header) + sizeof(long)) { // split it
                #if SBC
                    gc_header *next = (void *)here + size;
                #else
                    gc_header *next = (gc_header*)((char*)here + size);
                #endif
                    next->size = here->size - size;
                    here->size = size;			// shrink this block
                    next->busy = 0;			// make new next block free
                    next->mark = 0;
                    next->atom = 0;
                }
                #if SBC
                gc_memnext = (void *)here + here->size;			// next block to allocate,
                #else //C++
                gc_memnext = (gc_header*)((char*)here + here->size);
                #endif
                if (gc_memnext == gc_memend) gc_memnext = gc_memory;	// wraps back to start at the end
                assert(gc_memory  <= gc_memnext);
                assert(gc_memnext <  gc_memend);
                here->busy = 1;				// this block is now allocated
                #if SBC
                gc_debug SHICA_PRINTF("%p ALLOC %d %d\n", (void *)here + sizeof(*here), lbs, here->size);
                #else //C++
                gc_debug SHICA_PRINTF("%p ALLOC %d %d\n", (void*)((char*)here + sizeof(*here)), lbs, here->size);
                #endif
                return (void *)(here + 1);		// header address to object address
            }
            #if SBC
            here = (void *)here + here->size;		// block not free: advance to next block and
            #else //C++
            here = (gc_header*)((char*)here + here->size);
            #endif
            if (here == gc_memend) here = gc_memory;	// wrap back to start at the end of memory
            assert(gc_memory <= here);
            assert(here < gc_memend);
        } while (here != start);		// until we come back to where we started and
        if (retries) break;			// stop if we are on the second attempt
        gc_collect();				// otherwise collect garbage and try again
    }
#if SBC
    fatal("out of memory");	// could not allocate after collecting -- memory is full
#else //C++
    SHICA_PRINTF("out of memory");	// could not allocate after collecting -- memory is full
#endif
    return 0;
}

void *gc_beAtomic(void *p)	// note that the object is atomic (no pointers inside)
{
    ((gc_header *)p)[-1].atom = 1;
    return p;
}

void *gc_alloc_atomic(int size)	// allocate a block and make it atomic (no pointers inside)
{
    return gc_beAtomic(gc_alloc(size));
}

char *gc_strdup(char *s)	// allocate memory for and copy a string
{
    int len = strlen(s);
#if SBC
    char *mem = gc_alloc_atomic(len + 1);
#else
    char *mem = (char*)(gc_alloc_atomic(len + 1));
#endif
    gc_debug SHICA_PRINTF("%p:%s [%d]\n", mem, s, len);
    memcpy(mem, s, len);
    mem[len] = 0;
    return mem;
}

void *gc_realloc(void *oldptr, int newsize)	// grow or shrink an allocated block
{
    if (!oldptr) return gc_alloc(newsize);		// realloc(0) == alloc()
    gc_header *oldhdr = (gc_header *)oldptr - 1;	// object to header
    int oldsize = oldhdr->size - sizeof(gc_header);	// original size of object
    if ( oldsize >= newsize		// object will fit into original block and
	 && oldsize < newsize * 2	// fills at least half of the newly requested size
       )
 	return oldptr;			// don't change the size of the block
    gc_pushRoot(&oldptr);		// protect the original pointer from collection
    void *newptr = oldhdr->atom ? gc_alloc_atomic(newsize) : gc_alloc(newsize);
    --nroots;				// unprotect original pointer
    int len = newsize < oldsize ? newsize : oldsize;
    memcpy(newptr, oldptr, len);	// copy old object content into new block
    return newptr;			// object has moved
}


#if TEST_GC	// trivial test with linked lists -- see mstest.c for a real stress test

typedef struct Link Link;

struct Link {
    int  data;
    Link *next;
};

struct Link *newLink(int data, Link *next)
{
    Link *link = gc_alloc(sizeof(*link));
    link->data = data;
    link->next = next;
    return link;
}

void markLink(Link *ptr)
{
    gc_mark(ptr->next);
}

int main()
{
    gc_init(4096);
    gc_markFunction = (gc_markFunction_t)markLink;

    Link *list = 0;

    gc_pushRoot(&list);

    for (;;) {
	list = 0;
	for (int i = 0;  i < 10;  ++i) {
	    list = newLink(i, list);
	    static int cycle = 0;
	    if ((random() & 1023) < ((cycle = cycle + 1) & 1023)) gc_collect();
	}
	for (Link *l = list;  l;  l = l->next)
	    SHICA_PRINTF("%d ", l->data);
	SHICA_PRINTF("\n");
    }

    return 0;
}

#endif // TEST_GC

#endif // __gc_c
