// rcgc.c -- minimal reference counting garbage collector
//
// (C) 2024 Ian Piumarta
//
// This Source Code is subject to the terms of the Mozilla Public License,
// version 2.0, available at: https://mozilla.org/MPL/2.0/

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

struct gc_header
{
    unsigned int  size;      	// size of object, including header
    unsigned int  refs;		// reference count
    unsigned int  gcrc;		// cycle detection reference count
    unsigned      atom : 1;	// contains no pointers
    gc_header    *next;		// next header on the same list as this object
};

// macros to access header fields from an object pointer

#define gc_refcnt(P)	(((gc_header *)(P))[-1].refs)
#define gc_gcrcnt(P)	(((gc_header *)(P))[-1].gcrc)

gc_header *memory  = 0;	// the memory
gc_header *memend  = 0;	// the end (address of byte after) memory
gc_header *memnext = 0;	// the next block to consider for allocation

int gc_nused = 0;	// running total of bytes in allocated blocks
int gc_nfree = 0;	// running total of bytes in free blocks

void gc_init(int size)
{
    gc_nfree = size;
    memory  = malloc(size);
    memend  = (void *)memory + size;
    memnext = memory;
    memory->size = size;
    memory->refs = 0;
}

// the clear function dereferences every pointer in the given object

void gc_defaultClearFunction(void *ptr);

typedef void (*gc_clearFunction_t)(void *ptr);

gc_clearFunction_t gc_clearFunction = gc_defaultClearFunction;

static inline int GC_PTR(void *p)	// return non-zero if address p is inside object memory
{
    return (void *)memory <= p && p <= (void *)memend;
}

static inline void *REF(void *p)	// increase the reference count of the object at p
{
    if (GC_PTR(p)) ++gc_refcnt(p);
    return p;
}

// garbage collection happens incrementally, as the program is running.
// every time a pointer variable is modified, or goes out of scope, the pointer value it
// contains loses a reference; the object it points to is `dereferenced' by reducing its reference
// count by one.
// if the reference count frops to zero then there must be no remaining references to the object
// in any pointer variable, the object cannot take part in any future computation, it therefore
// garbage and can be collected immmediately.
// when an object is collected all the pointers in it go out of scope, and so every pointer variable
// in the object should be dereferenced too.
// the recursive dereferencing is accomplished by the gc_clearFunction.

static inline void DEREF(void *p)	// decrease the reference count of p, deallocate if zero
{
    if (GC_PTR(p)) {
	gc_debug {
	    extern void println();
	    printf("DEREF "); println(p); fflush(stdout);
	}
	assert(gc_refcnt(p));	// should never deref an unreachable object
	if (!--gc_refcnt(p)) {
	    int size = ((gc_header *)p)[-1].size;
	    gc_nused -= size;
	    gc_nfree += size;
	    gc_clearFunction(p);	// recursively deref each of p's pointers
	}
    }
}

// helper function: increase refs to new then decrease refs to old

static inline void *gc_set(void *old, void *new)
{
    REF(new);
    DEREF(old);
    return new;
}

// set a variable: increment new pointer, decrement old pointer, assign new pointer to variable

#define SET(VAR, VAL)		(VAR = gc_set(VAR, VAL))

// default clear function derefs every potential pointer in an object

void gc_defaultClearFunction(void *ptr)
{
    gc_header *hdr = ptr;
    if (hdr[-1].atom) return;	// atomic objects have no pointers
    for (void **p = ptr, **lim = ptr + hdr[-1].size;  p < lim;  ++p) {
	gc_header *q = *p;
	if (q < memory || q >= memend) continue;	// ignore pointers outside of memory
	SET(*p, 0);					// clear the pointer (implicit deref)
    }
}

// the traverse function applies a callback to every pointer stored in an object

typedef void (*gc_callback_t)(void *p, void *cookie);

typedef void (*gc_traverseFunction_t)(void *p, gc_callback_t fn, void *cookie);

// there is no default: the client must provide a means to identify the pointers accurately

void gc_defaultTraverseFunction(void *p, gc_callback_t fn, void *cookie)
{
    fatal("no traverse function has been set");
}

gc_traverseFunction_t gc_traverseFunction = gc_defaultTraverseFunction;

// cycle detection and reclamation keeps a list of reachable blocks

struct gc_list {
    gc_header *head;	// the first block in the list
    gc_header *tail;	// the last block in the list
};

// callback function that decrements the cycle reference count of every pointer in an object

void gc_rcdec(void *p, void *cookie)
{						assert(gc_gcrcnt(p) >= 0);
    gc_debug printf("%p decrement gcrc\n", p);
    gc_gcrcnt(p)--;
}

// callback function that increments the cycle reference count of every pointer in an object
// and moves it onto the list of reachable objects (which is passed as the cookie)

void gc_rcinc(void *p, void *cookie)
{
    gc_debug printf("%p increment gcrc\n", p);
    gc_header *h = ((gc_header *)p) - 1;
    if (h->gcrc == 0) {	// block was potentially unreachable, but is now reachable
	gc_debug printf("%p moved back to reachable set\n", p);
	h->gcrc = 1;				// mark block as reachable
	struct gc_list *reachable = cookie;	// recover the reachable list via the cookie
	assert(!reachable->tail->next);
	reachable->tail->next = h;		// append block to reachable list
	reachable->tail = h;			// and make it the last reachable block
	h->next = 0;				// and make it the last one in the list
    }
}

// clear an unreachable object during cycle reclamation
// the gc_refcnt is set to 1 so that each unreachable object is traversed only once
// to avoid deref'ing the pointers in objects that are part of a cycle more than once

void gc_clearUnreachable(void *p, void *cookie)
{
    if (gc_refcnt(p) && !--gc_refcnt(p)) {
	int size = ((gc_header *)p)[-1].size;
	gc_nused -= size;
	gc_nfree += size;
	gc_traverseFunction(p, gc_clearUnreachable, 0);
    }
}

// when memory is full the following function identifies and collect cycles of garbage.
//
// a cycle of garbage contains only internal references from within the cycle, but every
// object in the cycle therefore appears to have a reference count of 1.
//
//     | external ref (e.g., from inside the interpreter)
//     v
//    +----------+   +----------+   +----------+      +----------+   +----------+
//    | refcnt 2 |   | refcnt 1 |   | refcnt 1 |      | refcnt 1 |   | refcnt 1 |
//    |          |   |          |   |          |      |          |   |          |
//    |       O--+-->|       O--+-->|       O--+--+   |       O--+-->|       O--+--+
//    +----------+   +----------+   +----------+  |   +----------+   +----------+  |
//     ^                                          |    ^                           |
//     |        cycle that is NOT garbage         |    |   cycle that IS garbage   |
//     +------------------------------------------+    +---------------------------+
//
// to identify and reclaim cyclic garbage:
//
// 1. for all live objects, copy refs to gcrc and add the object to a "reachable" list
//
//     | external ref
//     v
//    +----------+   +----------+   +----------+      +----------+   +----------+
//    | refcnt 2 |   | refcnt 1 |   | refcnt 1 |      | refcnt 1 |   | refcnt 1 |
//    | gcrc   2 |   | gcrc   1 |   | gcrc   1 |      | gcrc   1 |   | gcrc   1 |
//    | reach    |   | reach    |   | reach    |      | reach    |   | reach    |
//    |       O--+-->|       O--+-->|       O--+--+   |       O--+-->|       O--+--+
//    +----------+   +----------+   +----------+  |   +----------+   +----------+  |
//     ^                                          |    ^                           |
//     |        cycle that is NOT garbage         |    |   cycle that IS garbage   |
//     +------------------------------------------+    +---------------------------+
//
// 2. for all reachable objects, decrement the gcrc of every object referenced from it
//    (this reduces gcrc to 0 for objects that are only reachable from within a cycle)
//
//     | external ref
//     v
//    +----------+   +----------+   +----------+      +----------+   +----------+
//    | refcnt 2 |   | refcnt 1 |   | refcnt 1 |      | refcnt 1 |   | refcnt 1 |
//    | gcrc   1 |   | gcrc   0 |   | gcrc   0 |      | gcrc   0 |   | gcrc   0 |
//    | reach    |   | reach    |   | reach    |      | reach    |   | reach    |
//    |       O--+-->|       O--+-->|       O--+--+   |       O--+-->|       O--+--+
//    +----------+   +----------+   +----------+  |   +----------+   +----------+  |
//     ^                                          |    ^                           |
//     |        cycle that is NOT garbage         |    |   cycle that IS garbage   |
//     +------------------------------------------+    +---------------------------+
//
// 3. remove all objects with gcrc == 0 from the list of reachable objects
//
//     | external ref
//     v
//    +----------+   +----------+   +----------+      +----------+   +----------+
//    | refcnt 2 |   | refcnt 1 |   | refcnt 1 |      | refcnt 1 |   | refcnt 1 |
//    | gcrc   1 |   | gcrc   0 |   | gcrc   0 |      | gcrc   0 |   | gcrc   0 |
//    | reach    |   |          |   |          |      |          |   |          |
//    |       O--+-->|       O--+-->|       O--+--+   |       O--+-->|       O--+--+
//    +----------+   +----------+   +----------+  |   +----------+   +----------+  |
//     ^                                          |    ^                           |
//     |        cycle that is NOT garbage         |    |   cycle that IS garbage   |
//     +------------------------------------------+    +---------------------------+
//
// 4. for all objects on the reachable list, increment the gcrc for every object it references
//    and append that object to the reachable list iff the gcrc changed from 0 -> 1
//    (this finds all objects [in]directly reachable from some reference outside their cycle)
//    (in the following diagram:
//       from A: B->gcrc is incremented and B is added to the reachable list
//       from B: C->gcrc is incremented and C is added to the reachable list
//       from C: A->gcrc is incremented but not added to the list since it was already there)
//
//     | external ref
//     v    A              B              C                D              E
//    +----------+   +----------+   +----------+      +----------+   +----------+
//    | refcnt 2 |   | refcnt 1 |   | refcnt 1 |      | refcnt 1 |   | refcnt 1 |
//    | gcrc   2 |   | gcrc   1 |   | gcrc   1 |      | gcrc   0 |   | gcrc   0 |
//    | reach    |   |          |   |          |      |          |   |          |
//    |       O--+-->|       O--+-->|       O--+--+   |       O--+-->|       O--+--+
//    +----------+   +----------+   +----------+  |   +----------+   +----------+  |
//     ^                                          |    ^                           |
//     |        cycle that is NOT garbage         |    |   cycle that IS garbage   |
//     +------------------------------------------+    +---------------------------+
//
// 5. any objects that have refs > 0 and gcrc == 0 are cyclic garbage and can be reclaimed
//    (in the above diagram, objects D and E are reclaimed and any objects they refer to
//    have their refcnt reduced by 1, potentially reaching 0 and reclaiming them in turn)

void gc_collect(void) // find and reclaim cyclic garbage
{
    struct gc_list reachable  = { 0, 0 };	// list of reachable objects (not isolated cycles)

    // macro to append a pointer to a list

# define append(list, value)				\
    do {						\
	if (!list.head) list.head = list.tail = value;	\
	else {						\
	    assert(!list.tail->next);			\
	    list.tail->next = value;			\
	    list.tail = value;				\
	}						\
	value->next = 0;				\
    } while (0)

    // add all objects to reachable set and init gcrc = refs

    for (gc_header *h = memory;  h < memend;  h = (void *)h + h->size) {
	if (h->refs) {
	    h->gcrc = h->refs;
	    append(reachable, h);		gc_debug printf("%p added to reachable set\n", h+1);
	}
    }

    // decrement gcrc for all objects referenced by reachable objects

    for (gc_header *h = reachable.head;  h;  h = h->next)
	gc_traverseFunction((void *)(h+1), gc_rcdec, 0);

    // remove all objects with gcrc == 0 from the reachable set

    for (gc_header **hp = &reachable.head;  *hp;) {
	gc_header *h = *hp;
	if (!h->gcrc) {
	    gc_debug printf("%p removed from reachable set\n", h+1);
	    *hp = h->next;  	// hp stays still, but the object it references changes
	}
	else
	    hp = &h->next;	// hp moves to ref of next object in list
    }

    // increment gcrc for all objects referenced by reachable objects and
    // append them to the reachable list (to be included in the same scan later)

    for (gc_header *h = reachable.head;  h;  h = h->next)
	gc_traverseFunction((void *)(h+1), gc_rcinc, &reachable);

    // objects with refs > 0 && gcrc == 0 are garbage

    for (gc_header *h = memory;  h < memend;  h = (void *)h + h->size)
	if (h->refs && !h->gcrc) {
	    gc_debug {
		extern void println();
		printf("%p [%d] is garbage ", h+1, h->refs);  println(h+1);
	    }
	    h->refs = 1; // clear only once in...
	    gc_clearUnreachable((void *)(h+1), 0);	// sets refs to 0 and recursively clears
	}

# undef append
}

int gc_size(void *p)	// return the size of an object (not including its header)
{
    return ((gc_header *)p)[-1].size - sizeof(gc_header);
}

int gc_used(void)	// explicitly count the number of bytes used in allocated blocks
{
    int used = 0;
    for (gc_header *h = memory;  h < memend;  h = (void *)h + h->size)
	if (h->refs) used += h->size;
    return used;
}

unsigned long gc_total = 0;

void *gc_alloc(int lbs)	// allocate a block big enough to store an object of lbs bytes
{
    gc_total += lbs;
    // round up to multiple of pointer size
    int size = (lbs + sizeof(gc_header) + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
    assert(size >= sizeof(gc_header) + lbs);
    gc_header *start = memnext, *here = start;		// the block following the last allocation
    for (int retries = 0;  retries < 2;  ++retries) {	// try to allocate twice
	do {
	    gc_debug printf("%p ? %i %i\n", (void *)here + sizeof(*here), here->size, here->refs);
	    gc_header *next = (void *)here + here->size;
	    if (here->refs) {				// block is in use
		here = next;				// skip it
		if (here == memend) here = memory;	// wrapping around to start of memory
		continue;
	    }
	    // block is free
	    for (;;) { // coalesce all following free blocks into one single free block
		if (next >= memend || next->refs) break;
		here->size += next->size;
		next = (void *)here + here->size;
	    }
	    if (here->size < size) {			// free block is too small
		here = next;				// skip it
		if (here == memend) here = memory;	// wrapping around to start of memory
		continue;
	    }
	    // split the free block if the remainder is large enough to store at least a pointer
	    if (here->size > size + sizeof(gc_header) + sizeof(void *)) { // split
		next = (void *)here + size;
		next->size = here->size - size;
		next->refs = 0;
		here->size = size;
	    }
	    memnext = next;				// where to start the next allocation
	    if (memnext == memend) memnext = memory;	// wrapping if necessary
	    assert(memory <= memnext);  assert(memnext < memend);
	    gc_debug printf("%p ALLOC %d %d\n", here + 1, lbs, here->size);
	    here->atom = 0;	// by default assume this object will contain pointers
	    here->refs = 1;	// new objects are born with a single reference
	    gc_nused += here->size;
	    gc_nfree -= here->size;
#         ifndef NDEBUG
	    static int stats = 0;
	    if (++stats > 100000) {	// print some pretty stats every 100,000 allocations
		stats = 0;
		printf("\r\t\t\t\t\t\t[GC %d used %d free]\r", gc_nused, gc_nfree);
		fflush(stdout);
	    }
#         endif
	    return here + 1;		// block address to object address
	} while (here != start);
	if (retries) break;		// could not allocate; give up if this is second attempt
	gc_collect();			// otherwise reclaim any cycles and try one more time
    }
    fatal("out of memory");		// memory really is full (or too fragmented)
    return 0;
}

void *gc_beAtomic(void *p)	// make p be an atomic object (no pointers)
{
    ((gc_header *)p)[-1].atom = 1;
    return p;
}

int gc_isAtomic(void *p)	// answer non-zero if p is atomic
{
    return ((gc_header *)p)[-1].atom;
}

void *gc_alloc_atomic(int lbs)	// allocate an atomic object
{
    return gc_beAtomic(gc_alloc(lbs));
}

char *gc_strdup(char *s)	// duplicate a string into a collectible atomic object
{
    int len = strlen(s);
    char *mem = gc_alloc_atomic(len + 1);
    gc_debug printf("%p:%s [%d]\n", mem, s, len);
    memcpy(mem, s, len);
    mem[len] = 0;
    return mem;			// REFERENCE COUNT IS 1
}

void *gc_realloc(void *oldptr, int newsize)	// change the size of a block
{
    if (!oldptr) return gc_alloc(newsize);	// realloc(0) => alloc()
    gc_header *oldhdr = (gc_header *)oldptr - 1;
    assert(oldhdr->refs);
    int oldsize = oldhdr->size - sizeof(gc_header);
    if ( oldsize >= newsize         	// object will fit into old memory and
	 && oldsize < newsize * 2   	// new object fills at least half old memory then
       )
 	return oldptr;			// don't bother resizing
    void *newptr = gc_alloc(newsize);
    int len = newsize < oldsize ? newsize : oldsize;
    memcpy(newptr, oldptr, len);	// copy max possible bytes from old to new
    return newptr;			// note that ref count is already 1
}

#if TEST_GC	// trivial test with linked lists -- see rctest.c for a real stress test

typedef struct Link Link;

struct Link {
    int  data;
    Link *next;
};

struct Link *newLink(int data, Link *next)
{
    REF(next);
    Link *link = gc_alloc(sizeof(*link));
    link->data = data;
    link->next = 0;  SET(link->next, next);
    DEREF(next); // out of scope
    return link;
}

void clearLink(Link *link)
{
    SET(link->next, 0);
}

int main()
{
    gc_init(256);
    gc_clearFunction = clearLink;

    Link *list = 0;

    for (;;) {
	SET(list, 0);
	for (int i = 0;  i < 10;  ++i) {
	    Link *new = newLink(i, list);
	    SET(list, new);
	    DEREF(new); // out of scope
	}
	for (Link *l = list;  l;  l = l->next) {
	    assert(gc_refcnt(l) == 1);
	    printf("%d ", l->data);
	}
	printf("\n");
    }

    return 0;
}

#endif // TEST_GC

#endif // __gc_c
