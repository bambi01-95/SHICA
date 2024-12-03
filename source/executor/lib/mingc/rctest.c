// rctest.c -- stress-test for rcgc.c
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

#if !defined(SYSGC)	// define SYSGC to test this file's correctness, rather than rcgc.c
# include "rcgc.c"
#else			// use the system's conservative collector masquerading as rcgc.c
# include <gc.h>
  int  gc_nused = 0;
  int  gc_nfree = 0;
  long gc_total = 0;
  typedef void (*gc_callback_t)(void *, void *);
  typedef void (*gc_clearFunction_t)(void *);
  gc_clearFunction_t gc_clearFunction = 0;
  typedef void (*gc_traverseFunction_t)(void *obj, gc_callback_t callback, void *cookie);
  gc_traverseFunction_t gc_traverseFunction = 0;
  int gc_refcnt = 0;
# define gc_debug	if (0)
# define gc_init(N)	GC_INIT()
# define gc_alloc(N) 	GC_malloc(gc_total += (N))
# define gc_beAtomic(N) (N)
# define gc_isAtomic(N) 0
# define gc_collect(N) 	0
# define gc_refcnt(P)	gc_refcnt
# define gc_size(P)	GC_size(P)
# define REF(A)		(A)
# define DEREF(A)	(void)0
# define SET(A, B)	(A = (B))
#endif // SYSGC

// macros to reference or dereference multiple pointers

#define REF2(A, B)		(REF(A), REF(B))
#define REF3(A, B, C)		(REF(A), REF(B), REF(C))
#define REF4(A, B, C, D)	(REF(A), REF(B), REF(C), REF(D))
#define DEREF2(A, B)		(DEREF(A), DEREF(B))
#define DEREF3(A, B, C)		(DEREF(A), DEREF(B), DEREF(C))
#define DEREF4(A, B, C, D)	(DEREF(A), DEREF(B), DEREF(C), DEREF(D))

typedef union Object Object;
typedef Object *oop;

typedef enum type_t type_t;
enum type_t { Undefined, Integer, Symbol, Pair, Closure };

char *type_n[] = { "Undefined", "Integer", "Symbol", "Pair", "Closure" };

typedef oop (*prim_t)(oop fn, oop args, oop env);

#define OBJ	type_t _type

// pointers and bytes are dummy types for storing non-object arrays of pointers or chars
// they have type Undefined but a size larger than sizeof(struct Undefined)

union Object {
    struct           { OBJ;  							 };
    struct pointers  { OBJ;  int size;  oop  at[0];                              } pointers;
    struct bytes     { OBJ;  char at[0];                                         } bytes;
    struct Undefined { OBJ;                	       	     	       		 } Undefined;
    struct Integer   { OBJ;  long _value;  	       	     	       		 } Integer;
    struct Symbol    { OBJ;  oop name;  prim_t _form, _primitive;  oop value;    } Symbol;
    struct Pair      { OBJ;  oop a, d;                       		 	 } Pair;
    struct Closure   { OBJ;  oop parameters, body, environment;	 	         } Closure;
};

#undef OBJ

char  *getTypeName(type_t type)	{ return type_n[type]; }
type_t getType(oop obj)		{ assert(obj);  return obj->_type; }
int    is(type_t type, oop obj) { return getType(obj) == type; }

oop _new(int size, type_t type)	// create a new object of the given size and type
{
    oop obj = gc_alloc(size);
    gc_debug printf("%p=%s\n", obj, getTypeName(type));
    obj->_type = type;
    return obj;
}

#define new(TYPE)	_new(sizeof(struct TYPE), TYPE)

oop newPointers(int size)	// create a new dummy object for storing a pointer array
{
    oop obj = _new(sizeof(struct pointers) + sizeof(oop) * size, Undefined);
    obj->pointers.size = size;
    return obj;
}

oop newBytes(int size)		// create a new dummy object for storing a character array
{
    return gc_beAtomic(_new(sizeof(struct Undefined) + sizeof(char) * size, Undefined));
}

oop _typeCheck(oop obj, type_t type, char *file, int line)
{
    if (getType(obj) != type)
	fatal("%s:%d: expected %s got %s ",
	      file, line, getTypeName(type), getTypeName(getType(obj)));
    return obj;
}

#define get(OBJ, TYPE,FIELD)	  _typeCheck(OBJ, TYPE, __FILE__, __LINE__)->TYPE.FIELD

// set a pointer value of an object, modifying reference counts appropriately

#define set(OBJ, TYPE,FIELD, VAL) SET(get(OBJ, TYPE,FIELD), VAL)

oop nil       = 0;	// the undefined object (also used for false)
oop sym_t     = 0;	// the symbol "t"
oop sym_quote = 0;	// the symbol "quote"

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

oop newSymbol(char *string)
{
    int len  = strlen(string);
    oop name = newBytes(len + 1);			assert(1 == gc_refcnt(name));
    gc_debug printf("%p NAME %s\n", name, string);
    memcpy(name->bytes.at, string, len);
    name->bytes.at[len] = 0;
    oop obj  = new(Symbol);				assert(1 == gc_refcnt(obj));
    get(obj, Symbol,name)       = name;			assert(1 == gc_refcnt(name));
    get(obj, Symbol,_form)      = 0;
    get(obj, Symbol,_primitive) = 0;
    get(obj, Symbol,value)      = REF(nil);		// add a reference to nil
    return obj;
}

#define _symbolName(obj)	get(obj, Symbol,name->bytes.at)

char *symbolName(oop obj, char *who)
{
    if (!is(Symbol, obj)) fatal("%s: Symbol expected, found %s", who, getTypeName(getType(obj)));
    return _symbolName(obj);
}

oop    symbols =  0;	// table of pointers to all symbols allocated inside GC memory
int numsymbols =  0;	// how many symbols are in the table
int maxsymbols = 32;	// initial maximum capacity of the table

void nullClearFunction(void *p) {}

oop intern(char *string)	// convert a string to a unique Symboloop intern(char *string)
{
    int lo = 0, hi = numsymbols - 1;
    while (lo <= hi) {
	int mid = (lo + hi) / 2;
	oop obj = symbols->pointers.at[mid];
	int cmp = strcmp(string, _symbolName(obj));
	if      (cmp < 0) lo = mid + 1;
	else if (cmp > 0) hi = mid - 1;
	else              return obj;	// symbol for this string already exists
    }
    if (numsymbols >= maxsymbols) {					assert(1 == gc_refcnt(symbols));
	// grow table by 32 symbols when full
	oop new = newPointers(maxsymbols += 32);			assert(1 == gc_refcnt(new));
	gc_debug printf("%p = SYMBOLS REALLOC %p\n", new, symbols);
	// copy the old pointers to the new array
	memcpy(new->pointers.at, symbols->pointers.at, sizeof(oop) * numsymbols);
	// unused entries must be 0 to keep the GC clear functions happy
	memset(new->pointers.at + numsymbols, 0, sizeof(oop) * (maxsymbols - numsymbols));
	// throw away the old table
	gc_refcnt(symbols)--;						assert(0 == gc_refcnt(symbols));
	// install the new one
	symbols = new;
    }									assert(1 == gc_refcnt(symbols));
    memmove(symbols->pointers.at + lo + 1,
	    symbols->pointers.at + lo,
	    sizeof(oop) * (numsymbols - lo));
    ++numsymbols;
    oop sym = newSymbol(string);	assert(1 == gc_refcnt(sym));
    symbols->pointers.at[lo] = sym;
    return sym;
}

// a Pair is a link in a linked list (or an association between a key and a value):
// the "a" part stores data (or a key) and
// the "d" part stores the rest of the list (or the value associated with the key)

oop newPair(oop a, oop d)
{
    oop obj = new(Pair);		assert(1 == gc_refcnt(obj));
    get(obj, Pair,a) = REF(a);
    get(obj, Pair,d) = REF(d);
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
    oop obj = new(Closure);				assert(1 == gc_refcnt(obj));
    get(obj, Closure,parameters)  = REF(parameters);	// a list of formal parameter names
    get(obj, Closure,body)        = REF(body);	    	// a list of expressions in the body
    get(obj, Closure,environment) = REF(environment);	// the captured lexical environment
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

    assert(obj);
    gc_debug printf("\033[31m%d.\033[0m", gc_refcnt(obj));
    switch (getType(obj)) {
	case Undefined: {
	    if (nil == obj) {
		printf("()");
		break;
	    }
	    if (gc_isAtomic(obj)) {
		printf("#[%s]", obj->bytes.at);
		break;
	    }
	    printf("#(%d)", obj->pointers.size);
	    break;
	}
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
	    break;
	default: {
	    fatal("illegal object type at %p", obj);
	    break;
	}
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

oop read(void)	// read and return one object (atom or list); new object has 1 ref
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
	    oop list = nil; // sentinel: not included in result so no new ref
	    for (;;) {
		c = skipspace();
		if (EOF == c) fatal("EOF while reading list");
		if ('.' == c) break;	// '.' or ')' signals the end of the list
		if (')' == c) break;
		pushchar(c);
		oop tmp  = read();			assert(END != tmp);  assert(1 <= gc_refcnt(tmp));
		oop pair = newPair(tmp, list);		assert(1 == gc_refcnt(pair));
		DEREF2(tmp, list);	// refs transerred to pairx
		list = pair;
	    }
	    oop last = REF(nil);	// normal list ends in nil (included in result, creates a ref)
	    if ('.' == c) {		// dotted lists have a non-nil final "d" field
		DEREF(last);
		last = read();		// born with 1 ref
		if (END == last) fatal("EOF while reading dotted list");
		assert(1 <= gc_refcnt(last));
		c = skipspace();	// this must be a ')' to close the list
	    }
	    if (')' != c) fatal("expected ')' at end of list");
	    list = revlist(list, last);
	    return list;
	}
	case '.': 	fatal("'.' outside list");	// only valid inside a list
	case ')':	fatal("')' outside list");	// only valid inside a list
	case '\'': {		// 'x is short-hand for the list (quote x)
	    // read the object after the '.'
	    oop obj = read();						assert(1 <= gc_refcnt(obj));
	    if (END == obj) fatal("EOF while reading quoted value");
	    // make it a list
	    oop pair = newPair(obj, nil);				assert(1 == gc_refcnt(pair));
	    DEREF(obj);	 // ref transferred to pair
	    // make is (quote obj)
	    oop list = newPair(sym_quote, pair);			assert(1 == gc_refcnt(list));
	    DEREF(pair); // ref transferred to list
	    return list;
	}
	default: {			// must be an identifier
	    int len = 0, lim = 8;
	    // temporary buffer in collectible memory
	    oop buf = newBytes(lim);					assert(1 == gc_refcnt(buf));
	    gc_debug printf("%p BUFFER\n", buf);
	    for (;;) {
		buf->bytes.at[len++] = c;	// append next character in identifier
		c = nextchar();
		// spaces, list delimiters, quotation, or comment ends an identifier
		if (isspace(c) || EOF == c) break;
		if ('\'' == c || '(' == c || '.' == c || ')' == c || ';' == c) {
		    pushchar(c);	// read one character too much
		    break;		// end of identifier
		}
		if (len == lim - 1) {	// make sure there is always space for a terminating NUL
		    oop new = newBytes(lim += 8);
		    memcpy(new->bytes.at, buf->bytes.at, len);
		    DEREF(buf);		// will not clear because atomic
		    buf = new;
		    gc_debug printf("%p BUFFER REALLOC\n", buf);
		}
	    }
	    assert(len < lim);
	    buf->bytes.at[len] = 0;			// terminate
	    oop obj = REF(intern(buf->bytes.at));	// turn into symbol with +1 ref
	    DEREF(buf);					// discard buffer
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
    if (!is(Pair, keys)) return REF(tail);		// end of the keys
    // head is (key1 . val1)
    oop head = cons(car(keys), car(vals));		assert(1 <= gc_refcnt(head));
    // rest is pairlis of tails
    oop rest = pairlis(cdr(keys), cdr(vals), tail);
    // list is ((key1 . val1) ... pairlis of tails)
    oop list = cons(head, rest);			assert(1 <= gc_refcnt(list));
    DEREF2(head, rest);			// refs transferred to list
    return list;
}

// evaluate each element in an list as an expression and return a list of the results

oop evlis(oop list, oop env)
{
    if (!is(Pair, list)) return REF(nil);	// no more expressions
    oop head = car(list);			// first element
    oop tail = cdr(list);			// rest of list
    oop hval = eval(head, env);			assert(1 <= gc_refcnt(hval));
    oop tval = evlis(tail, env);		assert(1 <= gc_refcnt(tval));
    oop pair = cons(hval, tval);		assert(1 <= gc_refcnt(pair));
    DEREF2(hval, tval);
    return pair;
}

// apply a function to zero or more arguments in a given environment

oop apply(oop fn, oop args, oop env)
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
	    oop env2   = pairlis(get(fn, Closure,parameters), args, env);	// create parameters
	    oop result = REF(nil);
	    oop body   = get(fn, Closure,body);
	    while (is(Pair, body)) {			// evaluate each expression in the body
 		oop tmp = eval(get(body, Pair,a), env2);
		SET(result, tmp);	// result ref -1, tmp ref +1
		DEREF(tmp);		// -1 because of +1 from eval
		body = get(body, Pair,d);
	    }
	    DEREF(env2);	// discard parameters for this function
	    return result;
	}
    }
    // fn is either a non-primitive Symbol or an expression (a list)
    oop fnval = eval(fn, env);			assert(1 <= gc_refcnt(fnval));
    oop apval = apply(fnval, args, env);	assert(1 <= gc_refcnt(apval));
    DEREF(fnval); 	// discard
    return apval;
}

oop eval(oop exp, oop env)	// evaluate expression in environment
{
    gc_debug { printf("@  ");  println(exp); }
    switch (getType(exp)) {
	case Undefined:
	case Integer:
	case Closure: {
	    return REF(exp);	// evaluate to themselves, +1 because of new ref
	}
	case Symbol: {			// lookup symbol in environment alist
	    oop val = assoc(exp, env);
	    if (nil != val) return REF(cdr(val));	// assoc found: use its value
	    return REF(get(exp, Symbol,value));		// else return flobal value of symbol
	}
	case Pair: {			// apply a function to some arguments
	    oop func   = car(exp);	// head of list is the function
	    oop args   = cdr(exp);	// rest of list are the arguments
	    if (is(Symbol, func) && get(func, Symbol,_form))	 // symbol names a special form
		return get(func, Symbol,_form)(func, args, env); // which takes unevaluated arguments
	    oop evargs = evlis(args, env);		// evaluate all arguments
	    oop result = apply(func, evargs, env);	// apply function to them
	    DEREF(evargs);		// discard
	    return result;
	}
    }
}

// special forms: arguments must be GC protected if the form allocates memory

oop form_quote (oop fn, oop args, oop env)	// (quote x) -> x
{
    return REF(car(args));
}

oop form_lambda(oop fn, oop args, oop env)	// (lambda (params...) exprs...) -> Closure
{
    return newClosure(car(args), cdr(args), env);
}

oop form_let(oop fn, oop args, oop env)		// (let ((k1 v1) (k2 v2) ...) exprs...)
{
    oop binds = car(args);		// local variable bindings: ((k1 v1) (k2 v2) ...)
    oop body  = cdr(args);		// exprs in the body of the form
    oop env2  = REF(env);		// environment extended with new local variables
    while (is(Pair, binds)) {		// more local variables
	oop bind  = car(binds);		// first (key value) in the binding list
	oop name  = car(bind);		// the key
	oop val   = cadr(bind);		// the value
	oop evval = eval(val, env);	// which is evaluated
	oop var   = cons(name, evval);	// and then paired with the name as an association
	DEREF(evval);			// ref transferred to var
	oop pair  = cons(var, env2);	// prepend new variable to environment
	DEREF2(var, env2);		// refs transferred to pair
	env2      = pair;		// new start of local variable list
	binds     = cdr(binds);
    }
    oop result = REF(nil);
    while (is(Pair, body)) {		// evaluate each expression in the extended environment
	DEREF(result);
	result = eval(car(body), env2);
	body   = cdr(body);
    }
    DEREF(env2);	// discard new local variables
    return result;
}

oop form_if(oop fn, oop args, oop env)	// (if condition consequent alternate)
{
    oop val = eval(car(args), env);			// condition
    oop exp = (nil != val) ? cadr(args) : caddr(args);	// choose consequent or alternate
    DEREF(val);						// discard
    return eval(exp, env);				// evaluate consequent or alternate
}

oop form_while(oop fn, oop args, oop env)	// (while condition exprs...)
{
    oop cond   = car(args);
    oop body   = cdr(args);
    oop result = REF(nil);
    oop tmp    = 0;
    while (nil != (tmp = eval(cond, env))) {	// condition is true
	DEREF(tmp); 				// discard
	oop prog = body;
	while (is(Pair, prog)) {		// evaluate each expression in the body
	    SET(result, eval(car(prog), env));
	    DEREF(result); 			// -1 because of +1 from eval(prog)
	    prog = cdr(prog);
	}
    }
    DEREF(tmp);	// final eval(cond) returned REF(nil)
    return result;
}

// built-in primitive operations

oop prim_set(oop fn, oop args, oop env)	// (set symbol value)
{
    oop s = car(args), v = cadr(args);
    oop a = assoc(car(args), env);		// look up the symbol in the environment
    if (nil != a) set(a, Pair,d, v);
    else   {					// otherwise
	if (!is(Symbol, s)) fatal("set: non-symbol name");
	set(s, Symbol,value, v);		// set its global value
    }
    return REF(v);
}

oop prim_cons  (oop fn, oop args, oop env)	{ return cons(car(args), cadr(args)); }
oop prim_car   (oop fn, oop args, oop env)	{ return REF(caar(args)); }
oop prim_cdr   (oop fn, oop args, oop env)	{ return REF(cdar(args)); }

oop prim_set_car(oop fn, oop args, oop env)	// modify the 'a' part of a Pair
{
    oop pair = car(args);
    if (!is(Pair, pair)) return REF(nil);
    oop val  = cadr(args);
    set(pair, Pair,a, val);
    return REF(val);
}

oop prim_set_cdr(oop fn, oop args, oop env)	// modify the 'd' part of a Pair
{
    oop pair = car(args);
    if (!is(Pair, pair)) return REF(nil);
    oop val  = cadr(args);
    set(pair, Pair,d, val);
    return REF(val);
}

oop prim_eq  (oop fn, oop args, oop env)	{ return REF(eq(car(args), cadr(args))); }

oop prim_add(oop fn, oop args, oop env)	// add two integers
{
    oop result = newInteger(integerValue(car(args), "+") + integerValue(cadr(args), "+"));
    return result;
}

oop prim_sub(oop fn, oop args, oop env)	// subtract two integers
{
    oop result = newInteger(integerValue(car(args), "-") - integerValue(cadr(args), "-"));
    return result;
}

oop prim_less(oop fn, oop args, oop env) // compare two integers
{
    return REF(integerValue(car(args), "<") < integerValue(cadr(args), "<") ? sym_t : nil);
}

oop prim_gc(oop fn, oop args, oop env) // garbage collect cycles and return memory in use
{
    gc_collect();
    return newInteger(gc_nused);
}

void clearObject(oop obj)	// obj just died: deref all pointers it contains
{
    gc_debug { printf("%p clearing: ", obj);  println(obj); }
    switch (getType(obj)) {
	case Undefined: {
	    if (!gc_isAtomic(obj))
		for (int i = 0;  i < obj->pointers.size;  ++i)
		    DEREF(obj->pointers.at[i]);
	    break;
	}
	case Integer:
	    break;
	case Symbol:
	    DEREF(get(obj, Symbol,name));
	    DEREF(get(obj, Symbol,value));
	    break;
	case Pair:
	    DEREF(get(obj, Pair,a));
	    DEREF(get(obj, Pair,d));
	    break;
	case Closure:
	    DEREF(get(obj, Closure,parameters));
	    DEREF(get(obj, Closure,body));
	    DEREF(get(obj, Closure,environment));
	    break;
    }
}

// call callback(ptr, cookie) for every pointer in obj

void traverseObject(oop obj, gc_callback_t callback, void *cookie)
{
    switch (getType(obj)) {
	case Undefined: {
	    if (!gc_isAtomic(obj))
		for (int i = 0;  i < obj->pointers.size;  ++i)
		    if (obj->pointers.at[i])
			callback(obj->pointers.at[i], cookie);
	    break;
	}
	case Integer:
	    break;
	case Symbol:
	    callback(get(obj, Symbol,name), cookie);
	    callback(get(obj, Symbol,value), cookie);
	    break;
	case Pair:
	    callback(get(obj, Pair,a), cookie);
	    callback(get(obj, Pair,d), cookie);
	    break;
	case Closure:
	    callback(get(obj, Closure,parameters), cookie);
	    callback(get(obj, Closure,body), cookie);
	    callback(get(obj, Closure,environment), cookie);
	    break;
    }
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
    gc_clearFunction    = (gc_clearFunction_t   )clearObject;
    gc_traverseFunction = (gc_traverseFunction_t)traverseObject;

    symbols    = newPointers(maxsymbols);	// array of pointers in collectible memory
    nil        = new(Undefined);			assert(1 == gc_refcnt(nil));
    sym_t      = REF(intern("t"));			assert(2 == gc_refcnt(sym_t));
    sym_quote  = REF(intern("quote"));			assert(2 == gc_refcnt(sym_quote));

    intern("quote")  ->Symbol._form = form_quote;	// special forms
    intern("lambda") ->Symbol._form = form_lambda;
    intern("let")    ->Symbol._form = form_let;
    intern("if")     ->Symbol._form = form_if;
    intern("while")  ->Symbol._form = form_while;

    intern("cons")   ->Symbol._primitive = prim_cons;	// primitive functions
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

    for (;;) {
	oop obj = read();
	if (END == obj) break;
	gc_debug printf("%p\n", obj);			assert(1 <= gc_refcnt(obj));
	printf("   ");
	println(obj);
	oop result = eval(obj, nil);			assert(1 <= gc_refcnt(result));
	printf("=> ");
	println(result);				assert(1 <= gc_refcnt(obj));
	DEREF(obj);					assert(1 <= gc_refcnt(result));
	DEREF(result);
    }

    printf("%d used %d free\n", gc_nused, gc_nfree);

    // break cycles in symbols that store function refering to the same symbol

    for (int i = 0;  i < numsymbols;  ++i)
	SET(symbols->pointers.at[i]->Symbol.value, nil);

    // discard remaining references
    							assert(1 == gc_refcnt(symbols));
    DEREF(symbols);					assert(1 == gc_refcnt(sym_t));
    SET(sym_t,     0);					assert(1 == gc_refcnt(sym_quote));
    SET(sym_quote, 0);					assert(1 == gc_refcnt(nil));
    SET(nil,       0);					assert(0 == gc_used());

    // memory should now be empty

    struct rusage ru;

    getrusage(RUSAGE_SELF, &ru);
    double time = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.;

    printf("%lu bytes allocated in a heap of size %d\n", gc_total, memSize);
    printf("%lu bytes/second allocated in %g seconds\n", (long)(gc_total / time), time);
    printf("%d used %d free\n", gc_nused, gc_nfree); // should report 0 used

    return 0;
}
