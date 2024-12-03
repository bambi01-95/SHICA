// mstest.c -- stress-test for msgc.c
//
// (C) 2024 Ian Piumarta
//
// This Source Code is subject to the terms of the Mozilla Public License,
// version 2.0, available at: https://mozilla.org/MPL/2.0/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#include "fatal.c"

#if !defined(SYSGC)	// define SYSGC to test this file's correctness, rather than msgc.c
# include "msgc.c"
#else			// use a conservative collector masquerading as msgc.c
# include <gc.h>
# define gc_debug if (0)
# define gc_init(S) GC_INIT()
# define gc_pushRoot(P)
# define gc_popRoots(N)
# define GC_PUSH(TYPE, VAR, INIT) TYPE VAR = INIT
# define GC_POP(VAR)
# define gc_mark(P)
unsigned long gc_total = 0;
# define gc_alloc(S) GC_malloc(gc_total += (S))
# define gc_alloc_atomic(S) GC_malloc_atomic(S)
# define gc_beAtomic(P) (P)
# define gc_realloc(P, S) GC_realloc(P, S)
# define gc_strdup(S) GC_strdup(S)
# define gc_markOnly(P)
# define gc_collect() 0
typedef void (*gc_collectFunction_t)(void);
gc_collectFunction_t gc_collectFunction = 0;
typedef void (*gc_markFunction_t)(void *ptr);
gc_markFunction_t gc_markFunction = 0;
#endif

typedef union Object Object;
typedef Object *oop;

typedef enum type_t type_t;
enum type_t { Undefined, Integer, Symbol, Pair, Closure };

char *type_n[] = { "Undefined", "Integer", "Symbol", "Pair", "Closure" };

typedef oop (*prim_t)(oop fn, oop args, oop env);

#define OBJ	type_t _type	// common header for all objects

union Object {	// note that String.name is allocated in collectible memory
    struct           { OBJ;                	       	     	       		 };
    struct Undefined { OBJ;                	       	     	       		 } Undefined;
    struct Integer   { OBJ;  long _value;  	       	     	       		 } Integer;
    struct Symbol    { OBJ;  char *_name;  prim_t _form, _primitive;  oop value; } Symbol;
    struct Pair      { OBJ;  oop a, d;                                 		 } Pair;
    struct Closure   { OBJ;  oop parameters, body, environment;       		 } Closure;
};

#undef OBJ

char  *getTypeName(type_t type)	{ return type_n[type]; }
type_t getType(oop obj)		{ assert(obj);  return obj->_type; }
int    is(type_t type, oop obj) { return getType(obj) == type; }

void markObject(oop obj)	// mark function for GC: recursively mark each object pointer
{
    switch (getType(obj)) {
	case Symbol:
	    gc_mark(obj->Symbol._name);	// this string is allocated in collectible memory
	    gc_mark(obj->Symbol.value);
	    break;
	case Pair:
	    gc_mark(obj->Pair.a);
	    gc_mark(obj->Pair.d);
	    break;
	case Closure:
	    gc_mark(obj->Closure.parameters);
	    gc_mark(obj->Closure.body);
	    gc_mark(obj->Closure.environment);
	    break;
	default:
	    break;
    }
}

oop _new(int size, type_t type)	// create new, empty object of given size and type
{
    oop obj = gc_alloc(size);		gc_debug printf("%p=%s\n", obj, getTypeName(type));
    obj->_type = type;
    return obj;
}

#define new(TYPE)	_new(sizeof(struct TYPE), TYPE)

oop _typeCheck(oop obj, type_t type, char *file, int line)
{
    if (getType(obj) != type)
	fatal("%s:%d: expected %s got %s ",
	      file, line, getTypeName(type), getTypeName(getType(obj)));
    return obj;
}

#define  get(OBJ, TYPE,FIELD)	_typeCheck(OBJ, TYPE, __FILE__, __LINE__)->TYPE.FIELD

struct Undefined _nil = { Undefined };	// singleton object allocated outside the GC memory

oop nil = (oop)&_nil;

oop sym_t      = 0;	// the symbol "t"
oop sym_quote  = 0;	// the symbol "quote"

oop newInteger(long value)
{
    oop obj = gc_beAtomic(new(Integer));	// contains no pointers
    get(obj, Integer,_value) = value;
    return obj;
}

#define  _integerValue(obj)	get(obj, Integer,_value)

long integerValue(oop obj, char *who)
{
    if (!is(Integer, obj)) fatal("%s: expected Integer, found %s", who, getTypeName(getType(obj)));
    return _integerValue(obj);
}

oop newSymbol(char *name)
{
    GC_PUSH(oop, obj, new(Symbol));			// allocate and protect a symbol object
    get(obj, Symbol,_name)      = 0;   			// must be valid at next GC...
    get(obj, Symbol,_form)      = 0;
    get(obj, Symbol,_primitive) = 0;
    get(obj, Symbol,value)      = nil; 			// must be valid at next GC...
    get(obj, Symbol,_name)      = gc_strdup(name);	// because this might cause a GC
    GC_POP(obj);
    return obj;
}

#define _symbolName(obj)	get(obj, Symbol,_name)

char *symbolName(oop obj, char *who)
{
    if (!is(Symbol, obj)) fatal("%s: Symbol expected, found %s", who, getTypeName(getType(obj)));
    return _symbolName(obj);
}

oop   *symbols = 0;	// table of pointers to all symbols allocated inside GC memory
int numsymbols = 0;	// how many symbols are in the table
int maxsymbols = 0;	// current maximum capacity of the table

oop intern(char *string)	// convert a string to a unique Symbol
{
    int lo = 0, hi = numsymbols - 1;
    while (lo <= hi) {
	int mid = (lo + hi) / 2;
	oop obj = symbols[mid];
	int cmp = strcmp(string, _symbolName(obj));
	if      (cmp < 0) lo = mid + 1;
	else if (cmp > 0) hi = mid - 1;
	else              return obj;	// symbol for this string already exists
    }
    if (numsymbols >= maxsymbols)	// grow table by 32 symbols when it is full
	symbols = gc_realloc(symbols, sizeof(*symbols) * (maxsymbols += 32));
    memmove(symbols + lo + 1, symbols + lo, sizeof(*symbols) * (numsymbols - lo));
    ++numsymbols;
    return symbols[lo] = newSymbol(string);
}

void collectObjects(void)	// pre-collection funciton to mark all the symbols
{
    gc_markOnly(symbols);			// mark the table itself
    for (int i = 0;  i < numsymbols;  ++i)
	gc_mark(symbols[i]);			// and every symbol in the table
}

// a Pair is a link in a linked list (or an association between a key and a value):
// the "a" part stores data (or a key) and
// the "d" part stores the rest of the list (or the value associated with the key)

oop newPair(oop a, oop d)
{
    gc_pushRoot(&a);	// protect both arguments while allocating the object
    gc_pushRoot(&d);
    oop obj = new(Pair);
    gc_popRoots(2);
    get(obj, Pair,a) = a;
    get(obj, Pair,d) = d;
    return obj;
}

#define cons(a, d) newPair(a, d)

// accessors for the a and d parts of a Pair, or of a pair further down a list
// note car/cdr of a non-list (including of nil) is nil, not an error

oop car(oop obj) { return is(Pair, obj) ? get(obj, Pair,a) : nil; }
oop cdr(oop obj) { return is(Pair, obj) ? get(obj, Pair,d) : nil; }

oop caar(oop obj) { return car(car(obj)); }
oop cadr(oop obj) { return car(cdr(obj)); }
oop cdar(oop obj) { return cdr(car(obj)); }

oop caddr(oop obj) { return car(cdr(cdr(obj))); }

// a closure is an unevaluated function that captures its defining environment
// so that local variables from lexically enclosed scopes are visiible when it executes

oop newClosure(oop parameters, oop body, oop environment)
{
    gc_pushRoot(&parameters);	// all three parameters must be protected because
    gc_pushRoot(&body);
    gc_pushRoot(&environment);
    oop obj = new(Closure);	// this might cause GC
    gc_popRoots(3);
    get(obj, Closure,parameters)  = parameters;		// a list of formal parameter names
    get(obj, Closure,body)        = body;		// a list of expressions in the body
    get(obj, Closure,environment) = environment;	// the captured lexical environment
    return obj;
}

oop eq(oop a, oop b)	// compare two objects, return symbol t if equal otherwise nil
{
    if (a == b) return sym_t;
    type_t ta = getType(a), tb = getType(b);
    if (ta != tb) return nil;
    switch (getType(a)) {
	case Integer: return integerValue(a, "eq") == integerValue(b, "eq") ? sym_t : nil;
	default:      break;
    }
    return nil;
}

void print(oop obj)	// print an object
{
    static int recursion = 0;

    if (recursion > 50) {
	printf("..."); return;
    }

    ++recursion;

    switch (getType(obj)) {
	case Undefined: printf("()");					break;
	case Integer:	printf("%ld", integerValue(obj, "print"));	break;
	case Symbol:	printf("%s",  symbolName(obj, "print"));	break;
	case Pair: {
	    printf("(");	// print pairs as lists
	    int n = 0;
	    for (;;) {
		print(car(obj));
		obj = cdr(obj);
		if (!is(Pair, obj)) break;
		if (++n > 50) {
		    printf(" ...");
		    obj = nil;
		    break;
		}
		printf(" ");
	    }
	    if (nil != obj) {	// final element was not nil => "dotted list"
		printf(" . ");
		print(obj);
	    }
	    printf(")");
	    break;
	}
	case Closure:		// print the expression that creates this kind of Closure
	    printf("(lambda ");
	    print(get(obj, Closure,parameters));
	    for (oop body = get(obj, Closure,body);  is(Pair, body);  body = get(body, Pair,d)) {
		printf(" ");
		print(car(body));
	    }
	    printf(")");
	    break;
    }
    --recursion;
}

void println(oop obj)	// print an object then a newline
{
    print(obj);
    printf("\n");
}

oop revlist(oop list, oop last)	// reverse list in-place with zero allocations
{				// appending last to the end of it
    while (is(Pair, list)) {
	oop pair = list;		// the first pair in the list
	list = get(list, Pair,d);	// the rest of the list after the first pair
	get(pair, Pair,d) = last;	// move the first pair to the front of last
	last = pair;			// and then make it the new last part of the list
    }
    return last;			// when list runs out, last contains the reversed list
}

int pushedchar = -1;	// single character of pushback for the expression reader

int pushchar(int c)
{
    assert(pushedchar < 0);
    return pushedchar = c;
}

int nextchar(void)	// read one character from the input, accounting for pushed-back character
{
    if (pushedchar < 0) return getchar();
    int c = pushedchar;
    pushedchar = -1;
    return c;
}

int peekchar(void)	// peek one character ahead in the input
{
    return pushchar(nextchar());
}

#define END ((oop)(intptr_t)(-1))	// illegal sentinel object pointer meaning end-of-file

int skipspace(void)	// skip over spaces and comments (from ';' to end of line) in the input
{
    int c = nextchar();
    for (;;) {
	if (c == ';') do c = nextchar(); while (c >= ' ' || c == '\t');
	if (!isspace(c)) break;
	c = nextchar();
    }
    return c;	// the next non-space character on the input
}

oop read(void)	// read and return one object (atom or list)
{
    int c = skipspace();	// the next non-space character on the input
    if (EOF == c) return END;
    switch (c) {
	case '0'...'9': {		// read a number
	    long value = 0;
	    do {
		value = value * 10 + c - '0';
		c = nextchar();
	    } while (isdigit(c));
	    pushchar(c);
	    return newInteger(value);
	}
	case '(': {			// read a list
	    GC_PUSH(oop, list, nil);	// the list read so far (accumulated in reverse)
	    GC_PUSH(oop, obj,  nil);	// temporary holder for items in the list
	    for (;;) {
		c = skipspace();
		if (EOF == c) fatal("EOF while reading list");
		if ('.' == c) break;	// '.' or ')' signals the end of the list
		if (')' == c) break;
		pushchar(c);
		obj = read();		// recursively read one element of the list
		assert(END != obj);
		list = newPair(obj, list);	// prepend it to the list being constructed
	    }
	    obj = nil;			// normal lists end in nil
	    if ('.' == c) {		// dotted lists have a non-nil final "d" field
		obj = read();
		if (END == obj) fatal("EOF while reading dotted list");
		c = skipspace();	// this must be a ')' to close the list
	    }
	    if (')' != c) fatal("expected ')' at end of list");
	    GC_POP(obj);
	    GC_POP(list);
	    return revlist(list, obj);	// list was constructed backwards, so reverse it in-place
	}
	case '.': 	fatal("'.' outside list");	// only valid inside a list
	case ')':	fatal("')' outside list");	// only valid inside a list
	case '\'': {			// 'x is short-hand for the list (quote x)
	    GC_PUSH(oop, obj, read());	// read the object after the '
	    if (END == obj) fatal("EOF while reading quoted value");
	    obj = newPair(obj, nil);	// create (quote obj)
	    obj = newPair(sym_quote, obj);
	    GC_POP(obj);
	    return obj;
	}
	default: {			// must be an identifier
	    int len = 0, lim = 8;
	    GC_PUSH(char *, buf, gc_alloc_atomic(lim));	// temporary buffer in collectible memory
	    gc_debug printf("%p ALLOC BUF\n", buf);
	    for (;;) {
		buf[len++] = c;		// append next character in identifier
		c = nextchar();
		// spaces, list delimiters, quotation, or comment ends an identifier
		if (isspace(c) || EOF == c) break;
		if ('\'' == c || '(' == c || '.' == c || ')' == c || ';' == c) {
		    pushchar(c);	// read one character too much
		    break;		// end of identifier
		}
		if (len == lim - 1) {	// make sure there is always space for a terminating NUL
		    buf = gc_realloc(buf, lim += 8);
		    gc_debug printf("%p REALLOC\n", buf);
		}
	    }
	    assert(len < lim);
	    buf[len] = 0;		// terminate the string
	    oop obj = intern(buf);	// turn it into a Symbol
	    GC_POP(buf);		// allow the string buffer to be garbage collected
	    return obj;
	}
    }
    assert(!"this cannot happen");
    return 0;
}

oop eval(oop exp, oop env);	// evaluate an expression in an environment

// an association is a Pair containing (key . value);
// an association list is a list of associations forming a dictionary:
// the alist ((a . 3) (b . 4)) is a dictionary mapping a -> 32 and b -> 4

oop assoc(oop key, oop alist)	// lookup key in the association list, return the association or nil
{
    if (!is(Pair, alist)) return nil;		// not found
    if (key == caar(alist)) return car(alist);	// key of first association matches the target key
    return assoc(key, cdr(alist));		// try the rest of the list
}

// zip a list of keys with a list of values making ((key1 . val1) (key2 . val2) ... . tail)
// values beyond the end of the keys are ignored
// keys beyond the end of the values are associated with nil (because cdr(nil) == nil)

oop pairlis(oop keys, oop vals, oop tail)
{
    if (!is(Pair, keys)) return tail;				// end of the keys
    GC_PUSH(oop, head, cons(car(keys), car(vals)));		// head is (key1 . val1)
    GC_PUSH(oop, rest, pairlis(cdr(keys), cdr(vals), tail));	// rest is pairlis of tails
    head = cons(head, rest);					// head is ((key1 . val1) ... tail)
    GC_POP(rest);
    GC_POP(head);
    return head;
}

// evaluate each element in an list as an expression and return a list of the results

oop evlis(oop list, oop env)
{
    if (!is(Pair, list)) return nil;	// no more expressions
    GC_PUSH(oop, head, car(list));	// head is first element
    GC_PUSH(oop, tail, cdr(list));	// tail is remainder of list
    head = eval (head, env);		// evaluate the head
    tail = evlis(tail, env);		// evaluate everything in the tail
    head = cons(head, tail);		// prepend evaluated head to the tail
    GC_POP(tail);
    GC_POP(head);
    return head;
}

// apply a function to zero or more arguments in a given environment

oop apply(oop fn, oop args, oop env)	// all three arguments are GC protected (see eval())
{
    switch (getType(fn)) {
	case Undefined:
	case Integer:
	    fatal("cannot apply %s", getTypeName(getType(fn)));
	    break;
	case Symbol: {
	    prim_t p = get(fn, Symbol,_primitive);
	    if (!p) break;
	    return p(fn, args, env);	// fn is a Symbol that names a primitive function
	}
	case Pair:	// ((expression) args...)
	    break;
	case Closure: {	// user-defined function, produced by (lambda ...)
	    gc_pushRoot(&env);	// so we can assign to it...
	    env = pairlis(get(fn, Closure,parameters), args, env);	// create parameters
	    oop result = nil;
	    oop body   = get(fn, Closure,body);
	    while (is(Pair, body)) {			// evaluate each expression in the body
		result = eval(car(body), env);
		body   = cdr(body);
	    }
	    gc_popRoots(1);
	    return result;
	}
    }
    // fn is either a non-primitive Symbol or an expression (a list)
    gc_pushRoot(&fn);		// so we can assign to it...
    fn = eval(fn, env);		// evaluate Symbol -> value, or list -> value, and
    fn = apply(fn, args, env);	// apply it to the arguments
    gc_popRoots(1);
    return fn;
}

oop eval(oop exp, oop env)	// evaluate expression in environment
{
    gc_debug { printf("@ ");  println(exp); }
    switch (getType(exp)) {
	case Undefined:
	case Integer:
	case Closure: {
	    return exp;		// evaluate to themselves
	}
	case Symbol: {			// lookup symbol in environment alist
	    oop val = assoc(exp, env);
	    if (nil != val) return cdr(val);	// association found: use its value
	    return get(exp, Symbol,value);	// otherwise, return global value of symbol
	}
	case Pair: {			// apply a function to some arguments
	    oop func   = car(exp);	// head of list is the function
	    oop args   = cdr(exp);	// rest of list are the arguments
	    oop result = nil;
	    if (is(Symbol, func) && get(func, Symbol,_form))	 // symbol names a special form
		return get(func, Symbol,_form)(func, args, env); // which takes unevaluated arguments
	    gc_pushRoot(&func);
	    gc_pushRoot(&args);
	    gc_pushRoot(&env);
	    args = evlis(args, env);		// evaluate all the arguments
	    result = apply(func, args, env);	// apply the function to them
	    gc_popRoots(3);
	    return result;
	}
    }
}

// special forms: arguments must be GC protected if the form allocates memory

oop form_quote (oop fn, oop args, oop env)	// (quote x) -> x
{
    return car(args);
}

oop form_lambda(oop fn, oop args, oop env)	// (lambda (params...) exprs...) -> Closure
{
    return newClosure(car(args), cdr(args), env);
}

oop form_let(oop fn, oop args, oop env)		// (let ((k1 v1) (k2 v2) ...) exprs...)
{
    gc_pushRoot(&args);			// fn is the Symbol "let" which we will ignore
    gc_pushRoot(&env);
    GC_PUSH(oop, val, nil);		// temporary GC-protected variable
    oop binds = car(args);		// local variable bindings: ((k1 v1) (k2 v2) ...)
    oop body  = cdr(args);		// exprs in the body of the form
    while (is(Pair, binds)) {		// more local variables
	oop bind = car(binds);		// first (key value) in the binding list
	oop name = car(bind);		// the key
	val      = cadr(bind);		// the value
	val   	 = eval(val, env);	// which is evaluated
	val   	 = cons(name, val);	// and then paired with its name as an association
	env   	 = cons(val, env);	// and prepended to the environment
	binds 	 = cdr(binds);
    }
    oop result = nil;
    while (is(Pair, body)) {		// evaluate each expression in the extended environment
	result = eval(car(body), env);
	body   = cdr(body);
    }
    GC_POP(val);
    gc_popRoots(2);	// env, args
    return result;
}

oop form_if(oop fn, oop args, oop env)	// (if condition consequent alternate)
{
    gc_pushRoot(&args);
    gc_pushRoot(&env);
    oop val = eval(car(args), env);			// condition
    val = (nil != val) ? cadr(args) : caddr(args);	// choose consequent or alternate
    val = eval(val, env);				// and evaluate it
    gc_popRoots(2);
    return val;
}

oop form_while(oop fn, oop args, oop env)	// (while condition exprs...)
{
    gc_pushRoot(&args);
    gc_pushRoot(&env);
    oop cond   = car(args);
    oop body   = cdr(args);
    oop result = nil;
    while (nil != eval(cond, env)) {		// condition is true
	oop prog = body;
	while (is(Pair, prog)) {		// evaluate each expression in the body
	    result = eval(car(prog), env);
	    prog   = cdr(prog);
	}
    }
    gc_popRoots(2);
    return result;
}

// built-in primitive operations

oop prim_set(oop fn, oop args, oop env)	// (set symbol value)
{
    oop s = car(args), v = cadr(args);
    oop a = assoc(car(args), env);		// look up the symbol in the environment
    if (nil != a) get(a, Pair,d) = v;		// if found, change its value
    else   {					// otherwise
	if (!is(Symbol, s)) fatal("set: non-symbol name");
	get(s, Symbol,value) = v;		// set its global value
    }
    return v;
}

oop prim_cons  (oop fn, oop args, oop env)	{ return cons(car(args), cadr(args)); }
oop prim_car   (oop fn, oop args, oop env)	{ return caar(args); }
oop prim_cdr   (oop fn, oop args, oop env)	{ return cdar(args); }

oop prim_set_car(oop fn, oop args, oop env)	// modify the 'a' part of a Pair
{
    oop pair = car(args);
    if (!is(Pair, pair)) return nil;
    oop val  = cadr(args);
    return get(pair, Pair,a) = val;
}

oop prim_set_cdr(oop fn, oop args, oop env)	// modify the 'a' part of a Pair
{
    oop pair = car(args);
    if (!is(Pair, pair)) return nil;
    oop val  = cadr(args);
    return get(pair, Pair,d) = val;
}

oop prim_eq  (oop fn, oop args, oop env)	{ return eq(car(args), cadr(args)); }

oop prim_add(oop fn, oop args, oop env)	// add two integers
{
    return newInteger(integerValue(car(args), "+") + integerValue(cadr(args), "+"));
}

oop prim_sub(oop fn, oop args, oop env)	// subtract two integers
{
    return newInteger(integerValue(car(args), "-") - integerValue(cadr(args), "-"));
}

oop prim_less(oop fn, oop args, oop env) // compare two integers
{
    return integerValue(car(args), "<") < integerValue(cadr(args), "<") ? sym_t : nil;
}

oop prim_gc(oop fn, oop args, oop env) // compare two integers
{
    return newInteger(gc_collect());
}

#include <sys/resource.h>

int main(int argc, char *argv[])
{
    int memSize = 32*1024; // default memory size

    for (int n = 1;  n < argc;) {
	char *arg = argv[n++];
	if   (!strcmp(arg, "-m") && n < argc) {	// change memory size
	    char *end = 0;
	    memSize = strtol(argv[n++], &end, 10);
	    while (*end) {
		switch (*end++) {
		    case 'k': case 'K':  memSize *= 1024;  		continue;
		    case 'm': case 'M':  memSize *= 1024*1024;		continue;
		    case 'g': case 'G':  memSize *= 1024*1024*1024;	continue;
		    default: fatal("unknown memory size multiplier '%c'", *end);
		}
	    }
	}
	else {
	    fprintf(stderr, "usage: %s [-m memsize]", argv[0]);
	    exit(1);
	}
    }

    gc_init(memSize);
    gc_collectFunction = (gc_collectFunction_t)collectObjects;
    gc_markFunction    = (gc_markFunction_t)markObject;

    sym_t      = intern("t");		// these are protected by symbols[]
    sym_quote  = intern("quote");

    intern("quote")  ->Symbol._form = form_quote;	// the special forms
    intern("lambda") ->Symbol._form = form_lambda;
    intern("let")    ->Symbol._form = form_let;
    intern("if")     ->Symbol._form = form_if;
    intern("while")  ->Symbol._form = form_while;

    intern("cons")   ->Symbol._primitive = prim_cons;	// the primitive functions
    intern("car")    ->Symbol._primitive = prim_car;
    intern("cdr")    ->Symbol._primitive = prim_cdr;
    intern("set-car")->Symbol._primitive = prim_set_car;
    intern("set-cdr")->Symbol._primitive = prim_set_cdr;
    intern("eq")     ->Symbol._primitive = prim_eq;
    intern("+")      ->Symbol._primitive = prim_add;
    intern("-")      ->Symbol._primitive = prim_sub;
    intern("<")      ->Symbol._primitive = prim_less;
    intern("set")    ->Symbol._primitive = prim_set;
    intern("gc")     ->Symbol._primitive = prim_gc;

    oop obj = 0;
    while (END != (obj = read())) {	// read an object from the input
	printf("   ");
	println(obj);
	obj = eval(obj, nil);		// evaluate it
	printf("=> ");
	println(obj);			// print it
    }

    symbols = 0;	// discard the symbol table (so nothing should be reachable)
    numsymbols = 0;

    int used = gc_collect();	// should report 0 bytes used

    struct rusage ru;

    getrusage(RUSAGE_SELF, &ru);
    double time = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.;

    printf("%lu bytes allocated in a heap of size %d\n", gc_total, memSize);
    printf("%lu bytes/second allocated in %g seconds\n", (long)(gc_total / time), time);
    printf("%d used %d free\n", used, memSize - used); // should report 0 used

    return 0;
}
