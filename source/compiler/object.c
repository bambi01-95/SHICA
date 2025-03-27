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

#ifndef MAXTHREADSIZE
    #define MAXTHREADSIZE 10
#endif

union Object;
typedef union Object Object;
typedef Object *oop;
typedef oop (*Func)(oop);

#if DEBUG
void stop(char *file,int line){
    fprintf(stderr,"stop %s line %d\n",file,line);
    exit(0);
}
#define STOP stop(__FILE__,__LINE__);
// DEBUG LOG Function
void debug_log(char *file,int line,const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[LOG] %s line %d:\t",file,line);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}
#define DEBUG_LOG(...) debug_log(__FILE__, __LINE__, __VA_ARGS__)

void debug_log_ref(char *file1,int line1,char* file2,int line2,const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[LOG] %s line %d:\n",file1,line1);
    printf("        %s line %d:\t",file2,line2);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}
#define DEBUG_LOG_REF(...) debug_log_ref(__FILE__, __LINE__,file,line, __VA_ARGS__)

void debug_error_1ref(char* file,int line,const char *format, ...) {
    va_list args;
    va_start(args, format);
    SHICA_FPRINTF(stderr, "[ERROR] %s line %d\n",file,line);
    vprintf(format, args); 
    SHICA_FPRINTF(stderr, "\n");
    va_end(args);
}
#define DEBUG_ERROR(...) debug_error_1ref(__FILE__, __LINE__,__VA_ARGS__)
#define DEBUG_ERROR_COND(COND,...) ({ \
    if(!(COND)){ \
        debug_error_1ref(__FILE__,__LINE__,__VA_ARGS__);\
    }\
})

void debug_error_2ref(char* file1,int line1,char* file2,int line2,const char *format, ...) {
    va_list args;
    va_start(args, format);
    SHICA_FPRINTF(stderr, "[ERROR] %s line %d\n",file1,line1);//that call function that include debug_error()
        SHICA_FPRINTF(stderr, "        %s line %d:\t",file2,line2);//that call debug_error()
    vprintf(format, args); 
    SHICA_FPRINTF(stderr, "\n");
    va_end(args);
}
#define DEBUG_ERROR_REF(...) debug_error_2ref(__FILE__, __LINE__,file,line, __VA_ARGS__)
#define DEBUG_ERROR_COND_REF(COND,...) ({ \
    if(!(COND)){ \
        debug_error_2ref(__FILE__,__LINE__,file,line,__VA_ARGS__);\
    }\
})
#endif

#define user_error(COND,MSG,LINE) ({ \
    if(COND){ \
        if(TEST==1)printf("[VM LINE %d]",__LINE__); \
        fprintf(stderr,"line %d: %s\n",LINE,MSG); \
        exit(1); \
    } \
})

void _fatal(char*s, int line,char *msg, ...)
{   
    printf("%s line %d:\n",s,line);
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "\n d-.-b ERROR:");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}
#define fatal(...) _fatal(__FILE__,__LINE__,__VA_ARGS__)

#define out(A) printf("line %4d: %s\n",__LINE__,A)
#define line() printf("         line %4d: ",__LINE__)

typedef union VarData VarData;
typedef VarData *VD;

oop nil   = 0;
oop sys_false = 0;
oop sys_true  = 0;
oop none  = 0;
oop entry_sym = 0;

oop wildcard_aop = 0;//for transAspect
oop specific_aop = 0;//for transAspect

typedef enum {Default, VarI, VarII, VarF, VarFF, VarT, VarTI} VAR;
typedef enum {F_NONE, F_EOE, F_TRANS, F_ERROR} FLAG;

enum Type {
    Undefined,
    Operator,
    Integer, 
    Float, 
    String,
    List_d1,
    Key,    
    Symbol,  
    Function, 
    Struct,
    Event,
    State, 
    Advice,
    Pointcut,

    Pair, 
    EventParam,
    DupEvent,
    Assoc,
    Array,

    Binop, 
    Unyop,  
    Jointp,
    TransAspect,

    GetVar, 
    SetVar,  
    SetVarG,
    SetVarL,
    SetArray,
    GetArray,
    GetElement,
    SetVarEvent,
    SetType,
    Call,  
    Run,   

    Print, 
    If,       
    While, 
    For,
    Block,
    Continue,
    Break,
    Return,

    Primitive,
    EventFunc,

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
    "Operator",
    "Integer",
    "Float",
    "String",
    "List_d1",
    "Key",
    "Symbol",
    "Function",
    "Struct",
    "Event",
    "State",
    "Advice",
    "Pointcut",

    "Pair",
    "EventParam",
    "DupEvent",
    "Assoc",
    "Array",

    "Binop",
    "Unyop",
    "Jointp",
    "TransAspect",

    "GetVar",
    "SetVar",
    "SetVarG",
    "SetVarL",
    "SetArray",
    "GetArray",
    "GetElement",
    "SetVarEvent",
    "SetType",
    "Call",
    "Run",

    "Print",
    "If",
    "While",
    "For",
    "Block",
    "Continue",
    "Break",
    "Return",

    "Primitive",
    "EventFunc",

    "_BasePoint",
    "_Undefined",
    "_Integer",
    "_Long",
    "_Float",
    "_Double",
    "_Char",
    "_IntegerArray",
    "Thread",
    "END",
};



enum binop { AND, OR, ADD, SUB, MUL, DIV, MOD, LT, LE, GE, GT, EQ, NE,BAND,BOR,LSH,RSH };
enum unyop { NEG, AINC, BINC, ADEC, BDEC};
enum jointp{ BEFORE, AFTER, AROUND,};

struct Undefined { enum Type _type_; };
struct Operator  { enum Type _type_; char value;};    // for operator
struct Integer 	 { enum Type _type_;  char* number; int line; };
struct Float     { enum Type _type_;  char* number; int line;};
struct String    { enum Type _type_;  char *value; };
struct Key       { enum Type _type_;  char *pass; };
struct Symbol  	 { enum Type _type_;  char *name;  oop value,aspect; };
struct Function	 { enum Type _type_;  oop parameters, body;int position;enum Type kind;};
struct Struct    { enum Type _type_;  oop symbol, members;};
struct Event     { enum Type _type_; oop id, parameters, body; };
struct State     { enum Type _type_; oop id; oop *events; int size,index; };
struct Advice    { enum Type _type_; oop id,body; int position; };
struct Pointcut  { enum Type _type_; oop id,pair; };

struct Pair  	 { enum Type _type_;  oop a, b; };
struct EventParam   { enum  Type _type_; oop type,symbol,cond;};
struct DupEvent     { enum  Type _type_; oop eventFunc,event; };
//struct Param   { enum Type _type_; oop type, symbol;};
struct Assoc     { enum Type _type_;  oop symbol; enum Type kind; int index; };

struct Array     { enum Type _type_;  oop *elements; int size/* related capacity */,number/* run time memory location */; int capacity;};

struct Binop   	 { enum Type _type_;  enum binop op;  oop lhs, rhs;       int line;};
struct Unyop   	 { enum Type _type_;  enum unyop op;  oop rhs;            int line;};
struct Jointp    { enum Type _type_;  enum jointp point; oop id; int position;};
struct TransAspect{ enum Type _type_;  oop from; oop to; oop body; };

struct GetVar  	 { enum Type _type_;  oop id;                             int line;};
struct GetElement{ enum Type _type_;  oop parent, child;                   int line;};
struct SetVar  	 { enum Type _type_;  enum Type typeset; oop id; oop rhs; int line;};
struct SetVarG   { enum Type _type_;  enum Type typeset; oop id; oop rhs; int line;};
struct SetVarL   { enum Type _type_;  enum Type typeset; oop id; oop rhs; int line;};
struct SetVarEvent{ enum Type _type_;  oop id; oop rhs; int line;};
struct SetType   { enum Type _type_;  oop id; oop child; int line;};
struct Call 	 { enum Type _type_;  oop function, arguments; char callType;            int line;};//callType: 0: normal, 1: init Event func
struct Run       { enum Type _type_; oop state; };

struct Print   	 { enum Type _type_;  oop arguments;};
struct If      	 { enum Type _type_;  oop condition, statement1, statement2; };
struct While   	 { enum Type _type_;  oop condition, statement; };
struct For       { enum Type _type_;  oop initstate, condition,  update, statement; };
struct Block   	 { enum Type _type_;  oop *statements;  int size; };
struct Continue  { enum Type _type_; };
struct Break     { enum Type _type_; };
struct Return    { enum Type _type_; oop value;};

struct END       { enum Type _type_; };

struct Primitive{    
    enum Type _type_;
    char  lib_num;
    char func_num;
    char* args_type_array;
    char  size_of_args_type_array;
    char  return_type;
};

struct EventFunc{
    enum Type _type_;
    char  event_type; //0: normal, 1: communicate
    char  lib_num;
    char  eve_num;
    char* args_type_array;
    char  size_of_args_type_array;
    char* pin_num_type;
    char  size_of_pin_num;
    oop   *pin_exps;
    oop   ownFunclist;//eventFuncName.ownFunc()
};

#define TAGINT	1			// tag bits value for Integer  ........1
#define TAGFLT	2			// tag bits value for Float    .......10
#define TAGCHA  3
#define TAGBITS 2			// how many bits to use for tag bits
#define TAGMASK ((1 << TAGBITS) - 1)	//.mask to extract just the tag bits

struct _BasePoint { enum Type _type_; int adress; };
struct _Undefined{ enum Type _type_; };
struct _Integer  { enum Type _type_; int _value; };
struct _Long     { enum Type _type_; long long int value; };
struct _Float    { enum Type _type_; float _value; };
struct _Double   { enum Type _type_; double value; };
struct _Char     { enum Type _type_; char _value; };

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
    //rbp: current base point
    unsigned int pc,rbp,base;   
    char queue_head; 
    char queue_num; 
    char flag; 
    oop stack; 
    oop queue[5];  
    Func func;
    union VarData*  vd;//REMOVE ME
};

union Object {
    enum   Type     _type_;
    struct Operator Operator;
    struct Integer  Integer;
    struct Symbol   Symbol;
    struct String   String; 
    struct Key      Key;
    struct Float    Float;
    struct List_d1  { enum Type __type_;enum Type Typeset; }List_d1;

    struct Pair     Pair;
    struct EventParam EventParam;
    struct DupEvent DupEvent;
    struct Function Function;
    struct Struct   Struct;
    struct Binop    Binop;
    struct Unyop    Unyop;
    struct Jointp   Jointp;
    struct TransAspect TransAspect;
    struct GetVar   GetVar;
    struct SetVar   SetVar;
    struct SetVarG SetVarG;
    struct SetVarL SetVarL;
    struct SetVarEvent SetVarEvent;
    struct SetType  SetType;

    struct GetArray	 { enum Type __type_;enum Type typeset;  oop array, index; int line;}GetArray;
    struct SetArray	 { enum Type __type_;enum Type typeset;  oop array, index, value; int line;}SetArray;
    struct GetElement GetElement;

    struct Call     Call;
    struct Print    Print;
    struct If       If;
    struct While    While;
    struct For      For;
    struct Block    Block;

    struct Continue Continue;
    struct Break    Break;
    struct Return   Return;
    struct Assoc    Assoc;
    struct Run      Run;
    struct State    State;
    struct Event    Event;
    struct Advice   Advice;
    struct Pointcut Pointcut;
    struct END      END;
    struct Array    Array;

    struct Primitive Primitive;
    struct EventFunc EventFunc;

    struct _BasePoint _BasePoint;
    struct _Undefined _Undefined;
    struct _Integer   _Integer  ;
    struct _Long      _Long     ;
    struct _Float     _Float    ;
    struct _Double    _Double   ;
    struct _Char      _Char     ;

    struct _IntegerArray {enum Type __type_;oop* array; int size, capacity;}_IntegerArray;
    struct Thread     Thread;
};

// union OBJECT{
//     struct BasePoint { enum Type _type_; int adress; }BasePoint;
//     struct Undefined{ enum Type _type_; }Undefined;
//     struct Integer  { enum Type _type_; int _value; }Integer;
//     struct Long     { enum Type _type_; long long int value; }Long;
//     struct Float    { enum Type _type_; float _value; };
//     struct Double   { enum Type _type_; double value; };
//     struct Char     { enum Type _type_; char _value; };
// };


int getType(oop o)
{
    if ((((intptr_t)o) & TAGMASK) == TAGINT) return _Integer;
    if ((((intptr_t)o) & TAGMASK) == TAGFLT) return _Float;
    if ((((intptr_t)o) & TAGMASK) == TAGFLT) return _Float;
    return o->_type_;
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
    node->_type_ = type;
    return node;
}

#define newObject(TYPE)	_newObject(sizeof(struct TYPE), TYPE)

/*---------------------------------------------*/
oop _newInteger(int value)
{
    return (oop)(((intptr_t)value << TAGBITS) | TAGINT);
}

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
    char value  = number[0];
    return (oop)(((intptr_t)value << TAGBITS) | TAGCHA);
}

#define _Char_value(A) (char)((intptr_t)A >> TAGBITS)
//FIXME: make segmentation error
// char _Char_value(oop obj)
// {
//     assert(_Char == getType(obj));
//     out("char value");
//     return (intptr_t)obj >> TAGBITS;
// }



oop _newIntegerArray(int size){
    oop node = newObject(_IntegerArray);
    node->_IntegerArray.capacity = size;/*Free size*/
    node->_IntegerArray.size     = size;
    node->_IntegerArray.array    = calloc(size, sizeof(oop));
    return node;
}




oop newArray(int);



/*---------------------------------------------*/
// oop newOperator(char value){
//     oop node = newObject(Operator);
//     node->Operator.value = value;
//     return node;
// }


oop newInteger(char* number,int line)
{
    oop node = newObject(Integer);
    node->Integer.number = strdup(number);
    node->Integer.line   = line;
    return node;
}

oop newFloat(char* number, int line){
    oop node = newObject(Float);
    node->Float.number = strdup(number);
    node->Float.line = line;
    return node;
}

oop newString(char *value){
    oop node = newObject(String);
    node->String.value = strdup(value);
    return node;
}

oop newKey(char *pass){
    oop node = newObject(Key);
    // is it need here? pass length 8~12 or fix num?
    // in this time 8
    if(8!=strlen(pass)){
        fprintf(stderr,"Key length shoud be 8\n");
        exit(1);
    }
    node->Key.pass = strdup(pass);
    return node;
}

oop newSymbol(char *name)
{
    oop sym = newObject(Symbol);
    sym->Symbol.name  = strdup(name);
    sym->Symbol.value = sys_false;
    sym->Symbol.aspect = nil;
    return sym;
}

oop *symbols = 0;
int nsymbols = 0;

oop intern(char *name)
{
    // binary search for existing symbol
    int lo = 0, hi = nsymbols - 1;
    while (lo <= hi) {
	int mid = (lo + hi) / 2;
	int cmp = strcmp(name, get(symbols[mid], Symbol,name));
	if      (cmp < 0) hi = mid - 1;
	else if (cmp > 0) lo = mid + 1;
	else    return symbols[mid];
    }
    symbols   = realloc(symbols,   sizeof(*symbols)   * (nsymbols + 1));
    memmove(symbols + lo + 1,
	    symbols + lo,
	    sizeof(*symbols) * (nsymbols++ - lo));
    return symbols[lo] = newSymbol(name);
}

oop newList_d1(enum Type t){
    oop node = newObject(List_d1);
    node->List_d1.Typeset = t;
    return node;
}

oop Array_push(oop obj,oop element);
void printlnObject(oop node, int indent);

oop newPair(oop a, oop b)
{
    oop obj = newObject(Pair);
    obj->Pair.a = a;
    obj->Pair.b = b;
    return obj;
}

oop rePair(oop p,oop tail){
    if(getType(p)!=Pair){
        return tail;
    }
    oop child = get(p,Pair,b);
    get(p,Pair,b) = tail;
    return rePair(child,p); 
}

oop newEventParam(oop type,oop symbol,oop cond){
    oop obj = newObject(EventParam);
    obj->EventParam.type = type;
    obj->EventParam.symbol = symbol;
    obj->EventParam.cond = cond;
    return obj;
}
oop newDupEvent(oop eventFunc,oop event){
    oop obj = newObject(DupEvent);
    obj->DupEvent.eventFunc = eventFunc;
    obj->DupEvent.event = event; // if some erro occur at Event function, check here
    return obj;
}

oop newFunction(oop parameters, oop body)
{
    oop node = newObject(Function);
    // * type need here *//
    node->Function.parameters = parameters;
    node->Function.body       = body;
    node->Function.position   = 0;
    node->Function.kind       = Undefined;
    return node;
}

oop newStruct(oop symbol, oop members)
{
    oop node = newObject(Struct);
    node->Struct.symbol  = symbol;
    node->Struct.members = members;
    return node;
}

oop newBinop(enum binop op, oop lhs, oop rhs,int line)
{
    oop node = newObject(Binop);
    node->Binop.op  = op;
    node->Binop.lhs = lhs;
    node->Binop.rhs = rhs;
    node->Binop.line = line;
    return node;
}

oop newUnyop(enum unyop op, oop rhs,int line)
{
    oop node = newObject(Unyop);
    node->Unyop.op  = op;
    node->Unyop.rhs = rhs;
    node->Unyop.line = line;
    return node;
}

oop newJointp(enum jointp point, oop id)
{
    oop node = newObject(Jointp);
    node->Jointp.point = point;
    node->Jointp.id    = id;
    node->Jointp.position = 0; // it is used at the optimaize.c
    return node;
}

oop newTransAspect(oop from, oop to, oop body)
{
    oop node = newObject(TransAspect);
    node->TransAspect.from = from;
    node->TransAspect.to   = to;
    node->TransAspect.body = body;
    return node;
}

oop setTrans(enum jointp point,oop id,int pc){
    oop aspect = get(id,Symbol,aspect);
    if(aspect==nil){
        aspect = newArray(0);
    }
    oop jointp = newJointp(point,id);
    jointp->Jointp.position = pc;
    Array_push(aspect,jointp);
    id->Symbol.aspect = aspect;
    return id;
}

oop newGetVar(oop id,int line)
{
    oop node = newObject(GetVar);
    node->GetVar.id = id;
    node->GetVar.line = line;
    return node;
}

oop newSetVar(oop typeset, oop id, oop rhs,int line)
{
    oop node = newObject(SetVar);
    node->SetVar.id  = id;
    node->SetVar.typeset = getType(typeset);
    node->SetVar.rhs = rhs;
    node->SetVar.line = line;
    return node;
}

oop newSetVarG(oop typeset, oop id, oop rhs,int line)
{
    oop node = newObject(SetVarG);
    node->SetVarG.id  = id;
    node->SetVarG.typeset = getType(typeset);
    node->SetVarG.rhs = rhs;
    node->SetVarG.line = line;
    return node;
}

oop newSetVarL(oop typeset, oop id, oop rhs,int line)
{
    oop node = newObject(SetVarL);
    node->SetVarL.id  = id;
    node->SetVarL.typeset = getType(typeset);
    node->SetVarL.rhs = rhs;
    node->SetVarL.line = line;
    return node;
}

//in progress
oop newSetArray(oop typeset,oop array, oop index, oop value,int line)
{
    oop node = newObject(SetArray);
    node->SetArray.typeset = getType(typeset);
    node->SetArray.array = array;
    node->SetArray.index = index;
    node->SetArray.value = value;
    node->SetArray.line  = line;
    return node;
}

oop newGetArray(oop array, oop index,int line)
{
    oop node = newObject(GetArray);
    node->GetArray.array = array;
    node->GetArray.index = index;
    node->SetArray.line  = line;
    return node;
}

oop newGetElement(oop parent, oop child,int line)
{
    oop node = newObject(GetElement);
    node->GetElement.parent = parent;
    node->GetElement.child  = child;
    node->GetElement.line   = line;
    return node;
}

oop newSetVarEvent(oop id, oop rhs,int line)
{
    oop node = newObject(SetVarEvent);
    node->SetVarEvent.id  = id;
    if(id->Symbol.value==nil){
        fatal("line %d: [%s] is already defined variable",line,get(id,Symbol,name));
    }
    node->SetVarEvent.rhs = rhs;
    node->SetVarEvent.line = line;
    return node;
}

oop newSetType(oop id,oop child,int line)
{
    oop node = newObject(SetType);
    node->SetType.id    = id;
    node->SetType.child = child;
    node->SetType.line  = line;
    return node;
}

oop newCall(oop arguments, oop function,int line)
{
    oop node = newObject(Call);
    node->Call.arguments = arguments;
    oop value = get(function,Symbol,value);
    if(getType(value)==EventFunc){
        fatal("line %d: EventFunc[ %s ] is not allowed to set without init symbol",line,get(function,Symbol,name));
    }
    node->Call.function  = function;
    node->Call.line = line;
    node->Call.callType = 0;
    return node;
}

oop newEventCall(oop arguments, oop function,int line)
{
    oop node = newObject(Call);
    oop value = get(function,Symbol,value);
    node->Call.arguments = arguments;
    node->Call.function  = function;
    node->Call.line = line;
    node->Call.callType = 1;
    return node;
}

oop copyEventFunc(oop func){
    oop node = newObject(EventFunc);
    node->EventFunc.event_type = get(func,EventFunc,event_type);
    node->EventFunc.lib_num    = get(func,EventFunc,lib_num);
    node->EventFunc.eve_num    = get(func,EventFunc,eve_num);
    node->EventFunc.size_of_args_type_array = get(func,EventFunc,size_of_args_type_array);
    node->EventFunc.args_type_array = strdup(get(func,EventFunc,args_type_array));
    node->EventFunc.size_of_pin_num = get(func,EventFunc,size_of_pin_num);
    node->EventFunc.pin_num_type = strdup(get(func,EventFunc,pin_num_type));
    node->EventFunc.pin_exps = malloc(sizeof(oop)*get(func,EventFunc,size_of_pin_num));
    return node;
}

oop newPrint(oop arguments)
{
    oop node = newObject(Print);
    node->Print.arguments = arguments;
    return node;
}

oop newIf(oop condition, oop s1, oop s2)
{
    oop node = newObject(If);
    node->If.condition = condition;
    node->If.statement1 = s1;
    node->If.statement2 = s2;
    return node;
}

oop newWhile(oop condition, oop s)
{
    oop node = newObject(While);
    node->While.condition = condition;
    node->While.statement = s;
    return node;
}

oop newFor(oop initstate,oop condition,oop updata,oop statement){
    oop node = newObject(For);
    node->For.initstate = initstate;
    node->For.condition = condition;
    node->For.update    = updata;
    node->For.statement = statement;
    return node;
}

oop newBlock(void)
{
    oop node = newObject(Block);
    node->Block.statements = 0;
    node->Block.size = 0;
    return node;
}

void Block_append(oop b, oop s)
{
    oop *ss = get(b, Block,statements);
    int  sz = get(b, Block,size);
    ss = realloc(ss, sizeof(oop) * (sz + 1));
    ss[sz++] = s;
    get(b, Block,statements) = ss;
    get(b, Block,size) = sz;
}
// leg 9
oop newContinue(){
    return newObject(Continue);
}
oop newBreak(){
    return newObject(Break);
}
oop newReturn(oop value){
    oop node =  newObject(Return);
    node->Return.value = value;
    return node;
}

oop newRun(oop s){
    oop node = newObject(Run);
    node->Run.state = s;
    return node;
}

oop newState(void)
{
    oop node = newObject(State);
    node->State.events = 0;
    node->State.size   = 0;
    return node;
}

oop setState(oop id,oop s){
    s->State.id = id;
    return s;
}

oop newEvent(oop id,oop params,oop body){
    oop node = newObject(Event);
    node->Event.id = id;
    node->Event.parameters = params;
    node->Event.body   = body;
    return node;
}

oop newAdvice(oop id,oop body){
    oop node = newObject(Advice);
    node->Advice.id = id;
    node->Advice.body = body;
    node->Advice.position = 0; // it is used at the optimaize.c
    return node;
}

oop newPointcut(oop id,oop pair){
    oop node = newObject(Pointcut);
    node->Pointcut.id = id;
    node->Pointcut.pair = pair;
    return node;
}

void State_append(oop s, oop e)
{
    oop *ss = get(s, State,events);
    int  sz = get(s, State,size);
    ss = realloc(ss, sizeof(oop) * (sz + 1));
    ss[sz++] = e;
    get(s, State,events) = ss;
    get(s, State,size)   = sz;
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

oop newAssoc(oop symbol,enum Type kind,int index){
    oop node = newObject(Assoc);
    node->Assoc.symbol = symbol;
    node->Assoc.kind   = kind;
    node->Assoc.index  = index;
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

oop *Array_put_elements(oop *main,oop sub,int index){
    assert(getType(sub)==Array);
    int end = sub->Array.size;
    for(int i = index; i < end; i++){
        main[i] = sub->Array.elements[i - index];
    }
    return main;
}


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