#ifndef TEST
    #define TEST 0
#endif

#ifndef DEBUG
    #define DEBUG 1
#endif 

#ifndef MAXTHREADSIZE
    #define MAXTHREADSIZE 10
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
#include "../../lib/mingc/msgc.c"
#ifndef MSGC
#define MSGC 1
#endif

union Object;
typedef union Object Object;
typedef Object *oop;
typedef oop (*Func)(oop);

/* for what wwww */
#define user_error(COND,MSG,LINE) ({ \
    if(COND){ \
        if(TEST==1)printf("[line %d]",__LINE__); \
        fprintf(stderr,"line %d: %s\n",LINE,MSG); \
        exit(1); \
    } \
})

void debug_log(char *file,int line,const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[LOG] %s line %d:\t",file,line);
    vprintf(format, args);  // 可変引数を処理
    printf("\n");
    va_end(args);
}
#define DEBUG_LOG(...) debug_log(__FILE__, __LINE__, __VA_ARGS__)


void debug_error_1ref(char* file,int line,const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[ERROR] %s line %d\n",file,line);
    vfprintf(stderr, format, args);  // エラーメッセージは stderr に出力
    fprintf(stderr, "\n");
    va_end(args);
}
#define DEBUG_ERROR(...) debug_error_1ref(__FILE__, __LINE__,__VA_ARGS__)
#define DEBUG_ERROR_COND(COND,...) ({ \
    if(!(COND)){ \
        debug_error_1ref(__FILE__,__LINE__,__VA_ARGS__);\
    }\
})

//file1 & line1: that call function that include debug_error()
//file2 & line2: that call debug_error()
void debug_error_2ref(char* file1,int line1,char* file2,int line2,const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[ERROR] %s line %d\n",file1,line1);
        fprintf(stderr, "        %s line %d:\t",file2,line2);
    vfprintf(stderr, format, args);  // エラーメッセージは stderr に出力
    fprintf(stderr, "\n");
    va_end(args);
}
#define DEBUG_ERROR_REF(...) debug_error_2ref(__FILE__, __LINE__,file,line, __VA_ARGS__)
#define DEBUG_ERROR_COND_REF(COND,...) ({ \
    if(!(COND)){ \
        debug_error_2ref(__FILE__,__LINE__,file,line,__VA_ARGS__);\
    }\
})

// //FIXME: use above
// void fatal(char *msg, ...)
// {
//     va_list ap;
//     va_start(ap, msg);
//     fprintf(stderr, "\n");
//     vfprintf(stderr, msg, ap);
//     fprintf(stderr, "\n");
//     va_end(ap);
//     exit(1);
// }

#define out(A) printf("line %4d: %s\n",__LINE__,A)
#define line() printf("         line %4d: ",__LINE__)

typedef union VarData VarData;
typedef VarData *VD;

// should be mark
oop nil   = 0;
oop sys_false = 0;
oop sys_true  = 0;
oop none  = 0;
oop entry_sym = 0;
oop threads = 0;

typedef enum {Default, VarI, VarII, VarF, VarFF, VarT, VarTI} VAR;
typedef enum {F_NONE, F_EOE, F_TRANS, F_ERROR} FLAG;

enum Type {
    Undefined,
    _BasePoint,

    _Undefined,
    _Integer,
    _Long,
    _Float,
    _Double,
    _Char,
    String,
    _IntegerArray,

    Thread,
    Array,
    Queue,
    END,   
};

char *TYPENAME[END+1] = {
    "Undefined",
    "_BasePoint",

    "_Undefined",
    "_Char",
    "_Integer",
    "_Long",
    "_Float",
    "_Double",
    "String",
    "_IntegerArray",

    "Thread",
    "Array",
    "END",   
};
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


struct Undefined { enum Type type; };
struct _BasePoint { enum Type type; int adress; };
struct _Undefined{ enum Type type; };
struct _Integer  { enum Type type; int _value; };
struct _Long     { enum Type type; long long int value; };
struct _Float    { enum Type type; float _value; };
struct _Double   { enum Type type; double value; };
struct _Char     { enum Type type; char _value; };
struct String    { enum Type type;  char *value; };
#define QUEUE_SIZE 5          /* 待ち行列に入るデータの最大数 */
struct Queue     { enum Type type;  oop *elements; unsigned head:4; unsigned size:4; };
struct Array     { enum Type type;  oop *elements; int size; int capacity;};
struct END       { enum Type type; };

struct Thread{ 
    enum Type type;
    oop stack; 
    oop queue;
    // oop queue[5];  
    Func func;
    union VarData*  vd;
    unsigned int pc,rbp,base;  
    // unsigned queue_head:4;
    // unsigned queue_num:4; 
    unsigned flag:1; 
};

union Object {
    enum   Type     type;

    struct _BasePoint _BasePoint;
    struct _Undefined _Undefined;

    struct _Char      _Char     ;
    struct _Integer   _Integer  ;
    struct _Long      _Long     ;
    struct _Float     _Float    ;
    struct _Double    _Double   ;
    struct String     String; 
    // struct _IntegerArray {enum Type _type;oop* array; int size, capacity;}_IntegerArray;

    struct END      END;
    struct Queue    Queue;
    struct Array    Array;
    struct Thread     Thread;
};



#define TAGINT	1			// tag bits value for Integer  ........1
#define TAGFLT	2			// tag bits value for Float    .......10
#define TAGCHA  3
#define TAGBITS 2			// how many bits to use for tag bits
#define TAGMASK ((1 << TAGBITS) - 1)	//.mask to extract just the tag bits
int getType(oop o)
{
    if ((((intptr_t)o) & TAGMASK) == TAGINT) return _Integer;
    if ((((intptr_t)o) & TAGMASK) == TAGFLT) return _Float;
    if ((((intptr_t)o) & TAGMASK) == TAGCHA) return _Char;
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

#if DEBUG
    #define get(PTR, TYPE, FIELD)	(_check((PTR), TYPE, __FILE__, __LINE__)->TYPE.FIELD)
#else
    #define get(PTR, TYPE, FIELD)  (PTR->TYPE.FIELD)
#endif


void markObject(oop obj){
    DEBUG_LOG("markObject()");
    switch(getType(obj)){
        case Queue:{
            printf("mark Queue\n");
            for(int i=0;i<obj->Queue.size;i++){
                gc_mark(obj->Queue.elements[obj->Queue.head + i]);
            }
            break;
        }
        case Array:{
            printf("mark Array\n");
            for(int i = 0;i<obj->Array.size;i++){
                gc_mark(obj->Array.elements[i]);
            }
            break;
        }
        case Thread:{
            printf("mark Thread\n");
            gc_mark(obj->Thread.stack);
            gc_mark(obj->Thread.queue);
            gc_mark(obj->Thread.vd);
            break;
        }
    }
}

void printlnObject(oop node, int indent);
void collectObjects(void)	// pre-collection funciton to mark all the symbols
{
#if DEBUG
    DEBUG_LOG("\ncollectObject()");
    // for(int i = 0;i<threads->Array.size;i++){
    //     printlnObject(threads->Array.elements[i],0);
    // }
#endif
    gc_mark(threads);
    DEBUG_LOG("\nend of collectObject()\n");
    return;
}

oop _newObject(size_t size, enum Type type)
{
#if MSGC
    // printf("gc total %5lu\n",gc_total);
    oop node = gc_alloc(size);
    node->type = type;
    return node;
#else
    oop node = calloc(1, size);
    node->type = type;
    return node;
#endif
}

#define newObject(TYPE)	_newObject(sizeof(struct TYPE), TYPE)
#define newAtomicObject(TYPE) gc_beAtomic(newObject(TYPE))

oop new_Basepoint(int adress){
    oop node = newAtomicObject(_BasePoint);
    node->_BasePoint.adress = adress;
    return node;
}

//このアドレスは、stack内のアドレスに書き込まれるため、
//制作時には無視して良い、が、mark時に考慮する必要がある。
oop _newChar(char value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGCHA);
}
#if DEBUG
char Char_value(char* file, int line, oop obj){
    DEBUG_ERROR_COND_REF(_Char == getType(obj),"CHAR but %d",getType(obj));
    return (char)((intptr_t)obj >> TAGBITS);
}
#define _Char_value(OBJ) Char_value(__FILE__,__LINE__,OBJ)
#else
char _Char_value(oop obj){
    return (char)((intptr_t)obj >> TAGBITS);
}
#endif
//if error apper use this instead of above
//#define _Char_value(A) (char)((intptr_t)A >> TAGBITS)


oop _newInteger(int value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGINT);
}

#if DEBUG
int Integer_value(char* file, int line, oop obj)
{
    DEBUG_ERROR_COND_REF(_Integer==getType(obj),"INTEGER but %d",getType(obj));
    return (intptr_t)obj >> TAGBITS;
}
#define _Integer_value(OBJ) Integer_value(__FILE__,__LINE__, OBJ)
#else
int _Integer_value(oop obj)
{
    return (intptr_t)obj >> TAGBITS;
}
#endif

oop _newLong(long long int value){
    oop node = newAtomicObject(_Long);
    node->_Long.value = value;
    return node;
}

oop _newFloat(float value)
{
    union { double d;  intptr_t i; } u = { .d = (double)value };
    return (oop)(((intptr_t)u.i & ~TAGMASK) | TAGFLT);
}

#if DEBUG
float Float_value(char* file, int line, oop obj)
{
    DEBUG_ERROR_COND_REF(_Float == getType(obj),"FLOAT but %d",getType(obj));
    union { intptr_t i;  double d; } u = { .i = (intptr_t)obj };
    return (float)u.d;
}
#define _Float_value(OBJ) Float_value(__FILE__,__LINE__,OBJ)
#else
float _Float_value(oop obj)
{
    union { intptr_t i;  double d; } u = { .i = (intptr_t)obj };
    return (float)u.d;
}
#endif

oop _newDouble(double value){
    oop node = newAtomicObject(_Double);
    node->_Double.value = value;
    return node;
}

// oop _newIntegerArray(int size){
//     oop node = newObject(_IntegerArray);
//     node->_IntegerArray.capacity = size;/*Free size*/
//     node->_IntegerArray.size     = size;
//     node->_IntegerArray.array    = calloc(size, sizeof(oop));
//     return node;
// }

oop newString(char *value){
    oop node = newAtomicObject(String);
    node->String.value = strdup(value);
    return node;
}

oop newEND(void){
    oop node = newAtomicObject(END);
    return node;
}


oop newArray(int capacity){
#if MSGC
    GC_PUSH(oop, obj, newObject(Array));
    obj->Array.size = 0;
    obj->Array.capacity = capacity;
    obj->Array.elements = gc_alloc(sizeof(oop) * capacity);
    GC_POP(obj);
#else
    oop obj = newObject(Array);
    obj->Array.size = 0;
    obj->Array.capacity = capacity;
    obj->Array.elements = calloc(capacity, sizeof(oop));
#endif
    return obj;
}

//FIXME: use get() function 
int Array_size(oop obj){
    assert(getType(obj)==Array);
    return obj->Array.size;
}

#if DEBUG
oop _Array_put(char* file, int line,oop obj,unsigned int index,oop element)
{
    DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
    if(index >obj->Array.capacity){
        DEBUG_LOG("Array Put Over current memory capacity %d", obj->Array.capacity);
        obj->Array.elements= realloc(obj->Array.elements,(index+1)*sizeof(oop));
        obj->Array.capacity=index + 1;
    }
    while(obj->Array.size<(index+1)){
        obj->Array.elements[obj->Array.size++]=nil;
    }
    return obj->Array.elements[index]=element;
}
#define Array_put(OBJ,INDEX,ELEMENT) _Array_put(__FILE__,__LINE__,OBJ,INDEX,ELEMENT)
#else
oop Array_put(oop obj,unsigned int index,oop element)
{
    if(index >obj->Array.capacity){
        obj->Array.elements= realloc(obj->Array.elements,(index+1)*sizeof(oop));
        obj->Array.capacity=index + 1;
    }
    while(obj->Array.size<(index+1)){
        obj->Array.elements[obj->Array.size++]=nil;
    }
    return obj->Array.elements[index]=element;
}
#endif


#if DEBUG
oop _Array_get(char *file, int line,oop obj, unsigned int index){
    DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
    if(index < obj->Array.size)
		return obj->Array.elements[index];
	return nil;
};
#define Array_get(OBJ,INDEX) _Array_get(__FILE__,__LINE__,OBJ,INDEX)
#else
oop Array_get(oop obj, unsigned int index){
    assert(getType(obj)==Array);
    if(index < obj->Array.size)
		return obj->Array.elements[index];
	return nil;
};
#endif

#if DEBUG
oop _Array_push(char* file, int line,oop obj,oop element){
    DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
    if(obj->Array.size >= obj->Array.capacity){
        DEBUG_ERROR_REF("Reallocate array cap %d", obj->Array.capacity);
#if MSGC
		obj->Array.elements = gc_realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
#else
		obj->Array.elements = realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
#endif
		obj->Array.capacity += 1;
    }
    obj->Array.elements[obj->Array.size++] = element;
    return obj;
}
#define Array_push(OBJ,ELEMENT) _Array_push(__FILE__,__LINE__,OBJ,ELEMENT)
#else
oop Array_push(oop obj,oop element){
    if(obj->Array.size >= obj->Array.capacity){
		obj->Array.elements = realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
		obj->Array.capacity += 1;
    }
    obj->Array.elements[obj->Array.size++] = element;
    return obj;
};
#endif

#if DEBUG
oop _Array_pop(char* file,int line,oop obj){
    DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
	if(obj->Array.size <= 0){
		fprintf(stderr,"Array is empty [array_pop]\n");
		exit(1);
	}
    oop element = obj->Array.elements[--obj->Array.size];
    obj->Array.elements[obj->Array.size] = nil;
    return element;
};
#define Array_pop(OBJ) _Array_pop(__FILE__,__LINE__,OBJ)
#else
oop Array_pop(oop obj){
	if(obj->Array.size <= 0){
		fprintf(stderr,"Array is empty [array_pop]\n");
		exit(1);
	}
    oop element = obj->Array.elements[--obj->Array.size];
    obj->Array.elements[obj->Array.size] = nil;
    return element;
};
#endif

#if DEBUG
oop _Array_free(char* file,int line, oop obj){
    DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
    DEBUG_LOG("FREE THREADS");
    obj->Array.size = 0;
    return obj;
}
#define Array_free(OBJ) _Array_free(__FILE__,__LINE__,OBJ)
#else
#define Array_free(OBJ) OBJ->Array.size = 0
#endif

#if DEBUG
oop _Array_args_copy(char* file,int line,oop from,oop to)
{
    DEBUG_ERROR_COND_REF(getType(from)==Array,"Array[to] but %d",getType(from));
    DEBUG_ERROR_COND_REF(getType(to)==Array,"Array[from] but %d",getType(to));
    int toArrCap = to->Array.capacity;
    int fromArrSize = from->Array.size;
    DEBUG_ERROR_COND_REF(toArrCap > fromArrSize,"from Array size over toArray size");
    DEBUG_ERROR_COND_REF(to->Array.size==0,"Array[to] is not empty");
    Array_push(to,new_Basepoint(1));//1st rbp, 2nd.. event args,
    for(int i=0;i<fromArrSize;i++){
        to->Array.elements[to->Array.size++] = from->Array.elements[i];
    }
    return to;
}
#define Array_args_copy(FROM,TO) _Array_args_copy(__FILE__,__LINE__,FROM,TO)
#else
oop _Array_args_copy(char* file,int line,oop from,oop to)
{
    int toArrCap = to->Array.capacity;
    int fromArrSize = from->Array.size;
    Array_push(to,new_Basepoint(1));//1st rbp, 2nd.. event args,
    for(int i=0;i<fromArrSize;i++){
        to->Array.elements[to->Array.size++] = from->Array.elements[i];
    }
    return to;
}
#endif

oop newQueue(int capacity){
#if MSGC
    GC_PUSH(oop, node, newObject(Queue));
    node->Queue.elements = gc_alloc(sizeof(oop)*capacity);
    GC_POP(node);
#else
    oop node = newObject(Queue);
    node->Queue.elements = calloc(capacity,sizeof(oop));
#endif
    node->Queue.head = 0;
    node->Queue.size = 0;
    return node;
}

oop _newThread(size_t vd_size,int stk_size)
{
#if MSGC
    GC_PUSH(oop,node, newObject(Thread));
#else
    oop node = newObject(Thread);
#endif
    node->Thread.queue      = newQueue(5);
    node->Thread.flag       =  0;
    node->Thread.pc         =  0;
    node->Thread.base       =  0;
    node->Thread.rbp        =  1;//1st rbp, 2nd.. event args,
    node->Thread.stack      = newArray(stk_size);
#if MSGC
    VD vd = gc_alloc(vd_size);
    GC_POP(node);
#else
    VD       vd = calloc(1,vd_size);
#endif
    node->Thread.vd         = vd;
    return node;
}

#define newThread(VD_TYPE,STACK_SIZE)	_newThread(sizeof(struct VD_TYPE),STACK_SIZE)

oop _setThread(oop t,size_t size)
{
    t->Thread.flag       =  0;
    t->Thread.pc         =  0;
    t->Thread.base       =  0;
    t->Thread.rbp        =  0;
    t->Thread.stack      = newArray(0);
#if MSGC
    VD vd = gc_alloc(size);
#else
    VD vd = calloc(1,size);
#endif
    t->Thread.vd         = vd;
    return t;
}
#define setThread(T,TYPE)	_setThread(T,sizeof(struct TYPE))



void printThread(oop t){
    int i = getType(t->Thread.stack);
    printf("stack == %s\n",TYPENAME[i]);
    printf("pc     %d\n",t->Thread.pc);
    printf("rbp    %d\n",t->Thread.rbp);
    // printf("q head %d\n",t->Thread.queue.head);
    // printf("q num  %d\n",t->Thread.queue_num);
    printf("flag   %d\n",t->Thread.flag);
}

// QUEUE
#define SUCCESS     1       /* 成功 */
#define FAILURE     0       /* 失敗 */


#if DEBUG
int _enqueue(char* file, int line,oop t,oop data)
{
    DEBUG_ERROR_COND_REF(Queue==getType(t),"Queue but %d",getType(t));
    if (t->Queue.size < QUEUE_SIZE) {
        t->Queue.elements[(t->Queue.head + t->Queue.size) % QUEUE_SIZE] = data;
        t->Queue.size++;
        return SUCCESS;
    } else {
        DEBUG_LOG("queue is full");
        return FAILURE;
    }
}
#define enqueue(OBJ,DATA) _enqueue(__FILE__,__LINE__,OBJ,DATA)
#else
int enqueue(oop t,oop data)
{
    if (t->Queue.size < QUEUE_SIZE) {
        t->Queue.elements[(t->Queue.head + t->Queue.size) % QUEUE_SIZE] = data;
        t->Queue.size++;
        return SUCCESS;
    } else {
        return FAILURE;
    }
}
#endif


#if DEBUG
oop _dequeue(char*file, int line,oop t)
{
    DEBUG_ERROR_COND_REF(Queue==getType(t),"Queue but %d",getType(t));
    if (t->Queue.size > 0) {
        oop data = t->Queue.elements[t->Queue.head];
        t->Queue.head = (t->Queue.head + 1) % QUEUE_SIZE;
        t->Queue.size--;
        return data;
    } else {
        return 0;
    }
}
#define dequeue(OBJ) _dequeue(__FILE__,__LINE__,OBJ)
#else
oop dequeue(oop t)
{
    if (t->Queue.size > 0) {
        oop data = t->Queue.elements[t->Queue.head];
        t->Queue.head = (t->Queue.head + 1) % QUEUE_SIZE;
        t->Queue.size--;
        return data;
    } else {
        return 0;
    }
}
#endif

#endif