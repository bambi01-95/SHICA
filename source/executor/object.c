#ifndef TEST
    #define TEST 0
#endif

#ifndef DEBUG
    #define DEBUG 1
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
    printf("[LOG] %s line %d:",file,line);
    vprintf(format, args);  // 可変引数を処理
    printf("\n");
    va_end(args);
}
#define DEBUG_LOG(...) debug_log(__FILE__, __LINE__, __VA_ARGS__)

//file1 & line1: that call function that include debug_error()
//file2 & line2: that call debug_error()
void debug_error(char* file1,int line1,char* file2,int line2,const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[ERROR] %s line %d\n",file1,line1);
    fprintf(stderr, "        %s line %d:",file2,line2);
    vfprintf(stderr, format, args);  // エラーメッセージは stderr に出力
    fprintf(stderr, "\n");
    va_end(args);
}
// #define DEBUG_ERROR(...) debug_error(__FILE__, __LINE__, __VA_ARGS__)
#define DEBUG_ERROR_COND(COND,...) ({ \
    if(COND){ \
        debug_error(__FILE__,__LINE__,file,line,__VA_ARGS__);\
    }\
})

//FIXME: use above
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



struct Undefined { enum Type type; };
struct _BasePoint { enum Type type; int adress; };
struct _Undefined{ enum Type type; };
struct _Integer  { enum Type type; int _value; };
struct _Long     { enum Type type; long long int value; };
struct _Float    { enum Type type; float _value; };
struct _Double   { enum Type type; double value; };
struct _Char     { enum Type type; char _value; };
struct String    { enum Type type;  char *value; };
struct Array     { enum Type type;  oop *elements; int size,number; int capacity;};
struct END       { enum Type type; };

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

    struct _BasePoint _BasePoint;
    struct _Undefined _Undefined;

    struct _Char      _Char     ;
    struct _Integer   _Integer  ;
    struct _Long      _Long     ;
    struct _Float     _Float    ;
    struct _Double    _Double   ;
    struct String     String; 
    struct _IntegerArray {enum Type _type;oop* array; int size, capacity;}_IntegerArray;

    struct END      END;
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

oop _newObject(size_t size, enum Type type)
{
    oop node = calloc(1, size);
    node->type = type;
    return node;
}
#define newObject(TYPE)	_newObject(sizeof(struct TYPE), TYPE)


oop new_Basepoint(int adress){
    oop node = newObject(_BasePoint);
    node->_BasePoint.adress = adress;
    return node;
}

oop _newChar(char value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGCHA);
}
#if DEBUG
char Char_value(char* file, int line, oop obj){
    DEBUG_ERROR_COND(_Char != getType(obj),"CHAR but %d",getType(obj));
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
    DEBUG_ERROR_COND(_Integer!=getType(obj),"INTEGER but %d",getType(obj));
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
    oop node = newObject(_Long);
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
    DEBUG_ERROR_COND(_Float != getType(obj),"FLOAT but %d",getType(obj));
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
    oop node = newObject(_Double);
    node->_Double.value = value;
    return node;
}

oop _newIntegerArray(int size){
    oop node = newObject(_IntegerArray);
    node->_IntegerArray.capacity = size;/*Free size*/
    node->_IntegerArray.size     = size;
    node->_IntegerArray.array    = calloc(size, sizeof(oop));
    return node;
}

oop newString(char *value){
    oop node = newObject(String);
    node->String.value = strdup(value);
    return node;
}

oop newEND(void){
    oop node = newObject(END);
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

//FIXME: use get() function 
int Array_size(oop obj){
    assert(getType(obj)==Array);
    return obj->Array.size;
}

//FIXME: use get() function
int Array_number(oop obj){
    assert(getType(obj)==Array);
    return obj->Array.number;
}

#if DEBUG
oop _Array_put(char* file, int line,oop obj,unsigned int index,oop element)
{
    DEBUG_ERROR_COND(getType(obj)!=Array,"Array but %d",getType(obj));
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
        obj->Array.number++;
    }
    return obj->Array.elements[index]=element;
}
#endif


#if DEBUG
oop _Array_get(char *file, int line,oop obj, unsigned int index){
    DEBUG_ERROR_COND(getType(obj)!=Array,"Array but %d",getType(obj));
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
    DEBUG_ERROR_COND(getType(obj)!=Array,"Array but %d",getType(obj));
    if(obj->Array.size >= obj->Array.capacity){
		obj->Array.elements = realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
		obj->Array.capacity += 1;
    }
    obj->Array.number++;
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
    obj->Array.number++;
    obj->Array.elements[obj->Array.size++] = element;
    return obj;
};
#endif

#if DEBUG
oop _Array_pop(char* file,int line,oop obj){
    DEBUG_ERROR_COND(getType(obj)!=Array,"Array but %d",getType(obj));
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