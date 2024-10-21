#ifndef TEST
    #define TEST 0
#endif

#ifndef OBJECT_C
#define OBJECT_C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <termios.h>

union Object;
typedef union Object Object;
typedef Object *oop;
typedef oop (*Func)(oop);


#define user_error(COND,MSG,LINE) ({ \
    if(COND){ \
        if(TEST==1)printf("[VM LINE %d]",__LINE__); \
        fprintf(stderr,"line %d: %s\n",LINE,MSG); \
        exit(1); \
    } \
})

void fatal(char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "\n");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

#define out(A) printf("line %4d: %s\n",__LINE__,A)
#define line() printf("         line %4d: ",__LINE__)

typedef union VarData VarData;
typedef VarData *VD;

oop nil   = 0;
oop sys_false = 0;
oop sys_true  = 0;
oop none  = 0;
oop entry_sym = 0;


typedef enum {Default, VarI, VarII, VarF, VarFF, VarT, VarTI} VAR;
typedef enum {F_NONE, F_EOE, F_TRANS, F_ERROR} FLAG;

enum Type {
    Undefined,
    // Float, 
    String,
    // List_d1,
    // Key,    
    // Symbol,  
    // Function, 
    // Event,
    // State, 

    // Pair, 
    // Assoc,
    Array,

    // Binop, 
    // Unyop,  

    // GetVar, 
    // SetVar,  
    // SetVarG,
    // SetVarL,
    // SetArray,
    // GetArray,
    // Call,  
    // Run,     

    // Print, 
    // If,       
    // While, 
    // For,
    // Block,
    // Continue,
    // Break,
    // Return,

    // Primitive,
    // EventFunc,

    _BasePoint,
    _Undefined,
    _Integer,
    _Long,
    _Float,
    _Double,
    _Char,
    _IntegerArray,
    Thread,
    END,   
};

char *TYPENAME[END+1] = {
"Undefined",

"END",
};

struct Undefined { enum Type type; };
struct String    { enum Type type;  char *value; };
struct Array     { enum Type type;  oop *elements; int size,number; int capacity;};

struct END       { enum Type type; };


#define TAGINT	1			// tag bits value for Integer  ........1
#define TAGFLT	2			// tag bits value for Float    .......10
#define TAGCHA  3
#define TAGBITS 2			// how many bits to use for tag bits
#define TAGMASK ((1 << TAGBITS) - 1)	//.mask to extract just the tag bits

struct _BasePoint { enum Type type; int adress; };
struct _Undefined{ enum Type type; };
struct _Integer  { enum Type type; int _value; };
struct _Long     { enum Type type; long long int value; };
struct _Float    { enum Type type; float _value; };
struct _Double   { enum Type type; double value; };
struct _Char     { enum Type type; char _value; };

struct Default{
    unsigned int count;
};

struct VarI{
    int v_i1;
};

struct VarII{
    int v_i1;
    int v_i2;
};

struct VarF{
    float v_f1;
};

struct VarFF{
    float v_f1;
    float v_f2;
};

struct VarT{
    time_t v_t1;
};

struct VarTI{
    time_t v_t1;
    int    v_i1;
    int    v_i2;
};

union VarData{
    struct Default Default;
    struct VarI  VarI;
    struct VarII VarII;
    struct VarF  VarF; 
    struct VarFF VarFF;    
    struct VarT  VarT; 
    struct VarTI VarTI;
};



struct Thread{
    unsigned int pc,rbp,base;   
    char queue_head; 
    char queue_num; 
    char flag; 
    oop stack; 
    oop queue[5];  
    Func func;
    union VarData*  vd;
};

union Object {
    enum   Type     type;
    struct String   String; 

    struct END      END;
    struct Array    Array;


    struct _BasePoint _BasePoint;
    struct _Undefined _Undefined;
    struct _Integer   _Integer  ;
    struct _Long      _Long     ;
    struct _Float     _Float    ;
    struct _Double    _Double   ;
    struct _Char      _Char     ;

    struct _IntegerArray {enum Type _type;oop* array; int size, capacity;}_IntegerArray;
    struct Thread     Thread;
};



int getType(oop o)
{
    if ((((intptr_t)o) & TAGMASK) == TAGINT) return _Integer;
    if ((((intptr_t)o) & TAGMASK) == TAGFLT) return _Float;
    if ((((intptr_t)o) & TAGMASK) == TAGFLT) return _Float;
    return o->type;
}



oop _check(oop node, enum Type type, char *file, int line)
{
    if (getType(node) != type) {
	fprintf(stderr, "\n%s:%d: expected type %d got type %d\n", file, line, type, getType(node));
		printf("%s but %s\n",TYPENAME[type],TYPENAME[getType(node)]);
    exit(1);
    }
    return node;
}

#define get(PTR, TYPE, FIELD)	(_check((PTR), TYPE, __FILE__, __LINE__)->TYPE.FIELD)

oop _newObject(size_t size, enum Type type)
{
    oop node = calloc(1, size);
    node->type = type;
    return node;
}

#define newObject(TYPE)	_newObject(sizeof(struct TYPE), TYPE)

/*---------------------------------------------*/

#if DEBUG
oop _newInteger(int value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGINT);
}

#else
oop _newInteger(int value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGINT);
}
#endif



oop _newCharInteger(char *number)
{
    int value = atoi(number);
    return (oop)(((intptr_t)value << TAGBITS) | TAGINT);
}

int _Integer_value(oop obj)
{
    assert(_Integer == getType(obj));
    return (intptr_t)obj >> TAGBITS;
}

oop _newLong(long long int value){
    oop node = newObject(_Long);
    node->_Long.value = value;
    return node;
}

oop _newCharLong(char* number){
    char *endptr;
    long long int value = strtoll(number,&endptr,10);
    return _newLong(value);
}

oop _newFloat(float value)
{
    union { double d;  intptr_t i; } u = { .d = (double)value };
    return (oop)(((intptr_t)u.i & ~TAGMASK) | TAGFLT);
}

oop _newCharFloat(char* number)
{
    double value = atof(number);
    union { double d;  intptr_t i; } u = { .d = value };
    return (oop)(((intptr_t)u.i & ~TAGMASK) | TAGFLT);
}

float _Float_value(oop obj)
{
    assert(_Float == getType(obj));
    union { intptr_t i;  double d; } u = { .i = (intptr_t)obj };
    return (float)u.d;
}

oop _newDouble(double value){
    oop node = newObject(_Double);
    node->_Double.value = value;
    return node;
}

oop _newCharDouble(char* number){
    char *endptr;
    double value = strtod(number, &endptr);
    return _newDouble(value);
}



/////////CHAR
// #define _newChar(A) (oop)(((intptr_t)A << TAGBITS) | TAGCHA)
oop _newChar(char value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGCHA);
}

oop _newCharChar(char number)
{
    char value  = number;
    return (oop)(((intptr_t)value << TAGBITS) | TAGCHA);
}

oop _newStrChar(char* number)
{
    assert(strlen(number)==2);
    char value  = number[0];
    return (oop)(((intptr_t)value << TAGBITS) | TAGCHA);
}

#define _Char_value(A) (char)((intptr_t)A >> TAGBITS)



oop _newIntegerArray(int size){
    oop node = newObject(_IntegerArray);
    node->_IntegerArray.capacity = size;/*Free size*/
    node->_IntegerArray.size     = size;
    node->_IntegerArray.array    = calloc(size, sizeof(oop));
    return node;
}




oop newArray(int);



oop newString(char *value){
    oop node = newObject(String);
    node->String.value = strdup(value);
    return node;
}


oop newEND(void){
    oop node = newObject(END);
    return node;
}

oop new_Basepoint(int adress){
    oop node = newObject(_BasePoint);
    node->_BasePoint.adress = adress;
    return node;
}



oop newArray(int capacity){
    oop obj = newObject(Array);
    obj->Array.size = 0;
    obj->Array.number = 0;
    obj->Array.capacity = capacity;
    obj->Array.elements = calloc(capacity, sizeof(oop));
    return obj;
}

int Array_size(oop obj){
    assert(getType(obj)==Array);
    return obj->Array.size;
}

int Array_number(oop obj){
    assert(getType(obj)==Array);
    return obj->Array.number;
}

oop Array_put(oop obj,unsigned int index,oop element)
{
    assert(getType(obj)==Array);
    if(index >obj->Array.capacity){
        obj->Array.elements= realloc(obj->Array.elements,(index+1)*sizeof(oop));
        obj->Array.capacity=index + 1;
    }
    while(obj->Array.size<(index+1)){
        obj->Array.elements[obj->Array.size++]=nil;
        obj->Array.number++;
    }
    return obj->Array.elements[index]=element;
}

oop Array_get(oop obj, unsigned int index){
    assert(getType(obj)==Array);
    if(index < obj->Array.size)
		return obj->Array.elements[index];
	return nil;
};

oop Array_push(oop obj,oop element){
    assert(getType(obj)==Array);
    if(obj->Array.size >= obj->Array.capacity){
		obj->Array.elements = realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
		obj->Array.capacity += 1;
    }
    obj->Array.number++;
    obj->Array.elements[obj->Array.size++] = element;
    return obj;
};

oop Array_pop(oop obj){
    assert(getType(obj)==Array);
	if(obj->Array.size <= 0){
		fprintf(stderr,"Array is empty [array_pop]\n");
		exit(1);
	}
    oop element = obj->Array.elements[--obj->Array.size];
    obj->Array.elements[obj->Array.size] = nil;
    return element;
};

oop _newThread(size_t size)
{
    oop node = newObject(Thread);
    node->Thread.flag       =  0;
    node->Thread.queue_head =  0;
    node->Thread.queue_num  =  0;
    node->Thread.pc         =  0;
    node->Thread.base       =  0;
    node->Thread.rbp        =  1;//1st rbp, 2nd.. event args,
    node->Thread.stack      = newArray(0);
    VD       vd = calloc(1,size);
    node->Thread.vd         = vd;
    return node;
}
#define newThread(TYPE)	_newThread(sizeof(struct TYPE))

oop _setThread(oop t,size_t size)
{
    t->Thread.flag       =  0;
    t->Thread.queue_head =  0;
    t->Thread.queue_num  =  0;
    t->Thread.pc         =  0;
    t->Thread.base       =  0;
    t->Thread.rbp        =  0;
    t->Thread.stack      = newArray(0);
    VD       vd = calloc(1,size);
    t->Thread.vd         = vd;
    return t;
}
#define setThread(T,TYPE)	_setThread(T,sizeof(struct TYPE))



void printThread(oop t){
    int i = getType(t->Thread.stack);
    printf("stack == %s\n",TYPENAME[i]);
    printf("pc     %d\n",t->Thread.pc);
    printf("rbp    %d\n",t->Thread.rbp);
    printf("q head %d\n",t->Thread.queue_head);
    printf("q num  %d\n",t->Thread.queue_num);
    printf("flag   %d\n",t->Thread.flag);
}

// QUEUE
#define SUCCESS     1       /* 成功 */
#define FAILURE     0       /* 失敗 */
#define QUEUE_SIZE 5          /* 待ち行列に入るデータの最大数 */

int enqueue(oop t,oop data)
{
    if (t->Thread.queue_num < QUEUE_SIZE) {
        t->Thread.queue[(t->Thread.queue_head + t->Thread.queue_num) % QUEUE_SIZE] = data;
        t->Thread.queue_num++;
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

oop dequeue(oop t)
{
    if (t->Thread.queue_num > 0) {
        oop data = t->Thread.queue[t->Thread.queue_head];
        t->Thread.queue_head = (t->Thread.queue_head + 1) % QUEUE_SIZE;
        t->Thread.queue_num--;
        return data;
    } else {
        return 0;
    }
}

#endif