#include "./setting.h"

#ifndef MAXTHREADSIZE
    #define MAXTHREADSIZE 10
#endif


#ifndef OBJECT_C
#define OBJECT_C

#include <stdlib.h>
#if SBC
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
    // #include <termios.h>

// <stdlib.h>
/*
malloc();      // メモリを動的に割り当てる
calloc();      // メモリを動的に割り当て、ゼロで初期化
realloc();     // メモリのサイズを変更
free();        // 動的に割り当てたメモリを解放
exit();        // プログラムを終了
atoi();        // 文字列を整数に変換
atof();        // 文字列を浮動小数点数に変換
strtol();      // 文字列を長整数に変換
*/

// <stdio.h>
/*
printf();      // 標準出力にフォーマット付き出力
scanf();       // 標準入力からフォーマット付き入力
fopen();       // ファイルをオープン
fclose();      // ファイルをクローズ
fread();       // ファイルからデータを読み込む
fwrite();      // ファイルにデータを書き込む
SHICA_FPRINTF();     // フォーマット付きのファイル出力
fscanf();      // フォーマット付きのファイル入力
*/

// <string.h>
/*
strlen();      // 文字列の長さを取得
strcpy();      // 文字列をコピー
strncpy();     // 指定した長さで文字列をコピー
strcat();      // 文字列を連結
strncat();     // 指定した長さで文字列を連結
strcmp();      // 文字列を比較
strncmp();     // 指定した長さで文字列を比較
*/

// <ctype.h>
/*
isalnum();     // 文字が英数字か判定
isalpha();     // 文字がアルファベットか判定
isdigit();     // 文字が数字か判定
toupper();     // 文字を大文字に変換
tolower();     // 文字を小文字に変換
*/

// <assert.h>
/*
assert();      // 条件が真であることを確認
*/

// <stdarg.h>
/*
va_start();    // 可変引数の処理開始
va_arg();      // 次の引数を取得
va_end();      // 可変引数の処理終了
*/

// <time.h>
/*
time();        // 現在の時刻を取得
difftime();    // 2つの時刻の差を計算
strftime();    // 時刻をフォーマットする
*/

// <sys/time.h>
/*
gettimeofday(); // 現在の時刻を取得
*/

// <sys/stat.h>
/*
stat();        // ファイルの情報を取得
chmod();       // ファイルのパーミッションを変更
mkdir();       // ディレクトリを作成
*/

// <unistd.h>
/*
read();        // ファイルを読み込む
write();       // ファイルに書き込む
close();       // ファイルディスクリプタをクローズ
sleep();       // 指定した時間だけ処理を停止
*/

// <fcntl.h>
/*
open();        // ファイルをオープン
fcntl();       // ファイルディスクリプタの操作
O_RDONLY;     // 読み取り専用モード
O_WRONLY;     // 書き込み専用モード
O_RDWR;       // 読み書きモード
*/

// <errno.h>
/*
errno;        // エラー番号を格納するグローバル変数
*/

// <sys/select.h>
/*
select();      // 複数のファイルディスクリプタを監視
FD_SET();      // ファイルディスクリプタをセット
FD_CLR();      // ファイルディスクリプタをクリア
FD_ISSET();    // ファイルディスクリプタがセットされているか判定
*/

// <termios.h>
/*
tcgetattr();   // 端末属性を取得
tcsetattr();   // 端末属性を設定
cfmakeraw();   // 端末を生データモードに設定
tcflush();     // 入力・出力キューをフラッシュ
*/
#endif


#include "./lib/msgc.c"

//remove me in the feature
#define user_error(COND,MSG,LINE) ({ \
    if(COND){ \
        if(TEST==1)SHICA_PRINTF("[line %d]",__LINE__);  \
        SHICA_FPRINTF(stderr,"line %d: %s\n",LINE,MSG); \
        exit(1); \
    } \
})

#if DEBUG
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
    vSHICA_FPRINTF(stderr, format, args); 
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
    vSHICA_FPRINTF(stderr, format, args); 
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

// DEBUG LOG Function
union Object;
typedef union Object Object;
typedef Object *oop;
typedef oop (*Func)(oop);

typedef union VarData VarData;
typedef VarData *VD;


// should be mark
oop nil       = 0;
oop sys_false = 0;
oop sys_true  = 0;
oop none      = 0;
oop threads   = 0;

typedef enum {Default, VarI, VarII, VarF, VarFF, VarT, VarTI} VAR;
typedef enum {F_NONE, F_EOE, F_TRANS, F_ERROR,F_TRUE,F_FALSE} FLAG;

enum Type {
    Undefined,
    _BasePoint,

    _Undefined,
    _Integer,
    _Long,
    _Float,
    _Double,
    _Char,
    _String,

    Thread,
    Array,
    Queue,
    END,   
};

#if DEBUG
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

        "Thread",
        "Array",
        "END",   
    };
#endif

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

#if SBC
struct VarT{
    time_t v_t1;
};

struct VarTI{
    time_t v_t1;
    int    v_i1;
    int    v_i2;
};
#endif

union VarData{
    struct Default Default;
    struct VarI  VarI;
    struct VarII VarII;
    struct VarF  VarF; 
    struct VarFF VarFF;   
#if SBC 
    struct VarT  VarT; 
    struct VarTI VarTI;
#endif
};


struct Undefined { enum Type type; };
struct _BasePoint{ enum Type type; int adress; };
struct _Undefined{ enum Type type; };
struct _Integer  { enum Type type; int _value; };
struct _Long     { enum Type type; long long int value; };
struct _Float    { enum Type type; float _value; };
struct _Double   { enum Type type; double value; };
struct _Char     { enum Type type; char _value; };
struct _String    { enum Type type;  char *value; };
#define QUEUE_SIZE 5          /* 待ち行列に入るデータの最大数 */
struct Queue     { enum Type type;  oop *elements/*gc_mark*/; unsigned head:4; unsigned size:4; };
struct Array     { enum Type type;  oop *elements/*gc_mark*/; int size; int capacity;};

struct Thread{ 
    enum Type type;
    oop stack; /*gc_mark*/
    oop queue; /*gc_mark*/
    Func func;
    unsigned int *loc_cond;/*stock location of condtion instraction*/
    union VarData*  vd; /*gc_mark*/
    unsigned int pc,rbp,base;
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
    struct _String    _String;
    struct Queue      Queue;
    struct Array      Array;
    struct Thread     Thread;
};

FLAG sub_execute(oop process,oop GM);


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

#if Debug
    oop _check(oop node, enum Type type, char *file, int line)
    {
        if (getType(node) != type) {
        SHICA_FPRINTF(stderr, "\n%s:%d: expected type %d got type %d\n", file, line, type, getType(node));
            printf("%s but %s\n",TYPENAME[type],TYPENAME[getType(node)]);
        exit(1);
        }
        return node;
    }
    #define getChild(PTR, TYPE, FIELD)	(_check((PTR), TYPE, __FILE__, __LINE__)->TYPE.FIELD)
#else
    #define getChild(PTR, TYPE, FIELD)  (PTR->TYPE.FIELD)
#endif

// GC_MARK
#if MSGC
void markObject(oop obj){
    switch(getType(obj)){
        case Queue:{
            gc_markOnly(obj->Queue.elements);//this
            for(int i=0;i<obj->Queue.size;i++){
                gc_mark(obj->Queue.elements[(obj->Queue.head + i) % QUEUE_SIZE]);
            }
            return ;
        }
        case Array:{
            gc_markOnly(obj->Array.elements);//this
            for(int i = 0;i<obj->Array.size;i++){
                gc_mark(obj->Array.elements[i]);
            }
            return ;
        }
        case Thread:{
            gc_mark(obj->Thread.stack);
            gc_mark(obj->Thread.queue);
            gc_mark(obj->Thread.vd);
            return ;
        }
        default:{
#if DEBUG
            DEBUG_ERROR("this is not happen type %d\n",getType(obj));
#else
            SHICA_PRINTF("this is not happen markObject()\n");
#endif
            return;
        }
    }
}

void isMarkObject(oop obj){
    switch(getType(obj)){
        case Queue:{
            SHICA_PRINTF("mark Queue\n");
            for(int i=0;i<obj->Queue.size;i++){
                gc_isMark(obj->Queue.elements[obj->Queue.head + i]);
            }
            break;
        }
        case Array:{
            SHICA_PRINTF("mark Array\n");
            for(int i = 0;i<obj->Array.size;i++){
                gc_isMark(obj->Array.elements[i]);
            }
            break;
        }
        case Thread:{
            SHICA_PRINTF("mark Thread\n");
            gc_isMark(obj->Thread.stack);
            gc_isMark(obj->Thread.queue);
            gc_isMark(obj->Thread.vd);//atomic
            break;
        }
        default:{
#if DEBUG
            DEBUG_ERROR("this is not happen type %d\n",getType(obj));
#else
            SHICA_PRINTF("this is not happen isMarkObject()\n");
#endif
        }
    }
}

void collectObjects(void)	// pre-collection funciton to mark all the symbols
{
    gc_mark(threads);
    return;
}
#endif


oop _newObject(size_t size, enum Type type)
{
#if MSGC
#if SBC
    oop node = gc_alloc(size);
#else //C++
    oop node = (oop)(gc_alloc(size));
#endif
    node->type = type;
    return node;
#else
    oop node = calloc(1, size);
    node->type = type;
    return node;
#endif
}
#define newObject(TYPE)	_newObject(sizeof(struct TYPE), TYPE)

#if SBC
#define newAtomicObject(TYPE) gc_beAtomic(newObject(TYPE))
#else //C++
#define newAtomicObject(TYPE) (oop)(gc_beAtomic(newObject(TYPE)))
#endif

oop new_Basepoint(int adress){
#if MSGC
    oop node = newAtomicObject(_BasePoint);
#else
    oop node = newObject(_BasePoint);
#endif
    node->_BasePoint.adress = adress;
    return node;
}



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
/*
// if error apper use this instead of above
#define _Char_value(A) (char)((intptr_t)A >> TAGBITS)
*/


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
#if MSGC
    oop node = newAtomicObject(_Long);
#else
    oop node = newObject(_Long);
#endif
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
#if MSGC
    oop node = newAtomicObject(_Double);
#else
    oop node = newObject(_Double);
#endif
    node->_Double.value = value;
    return node;
}

//IGNOREME: 
oop newString(char *value){
    SHICA_PRINTF("now, String is not string type\n");
    exit(1);
#if MSGC
    oop node = newAtomicObject(_String);
    // node->String.value = strdup(value);
#else
    oop node = newObject(_String);
    node->_String.value = strdup(value);
#endif
    return node;
}



//ARRAY
oop newArray(int capacity){
#if MSGC
    GC_PUSH(oop, obj, newObject(Array));
    obj->Array.size     = 0;
    obj->Array.capacity = capacity;
#if SBC 
    obj->Array.elements = gc_alloc(sizeof(oop) * capacity);
#else //C++
    obj->Array.elements = (oop*)(gc_alloc(sizeof(oop) * capacity));
#endif
    GC_POP(obj);
#else
    oop obj = newObject(Array);
    obj->Array.size = 0;
    obj->Array.capacity = capacity;
    obj->Array.elements = calloc(capacity, sizeof(oop));
#endif
    return obj;
}


//ARRAT PUT
#if DEBUG
    oop _Array_put(char* file, int line,oop obj,unsigned int index,oop element)
    {
        DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
        if(index >obj->Array.capacity){
            DEBUG_LOG("Array Put Over current memory capacity %d", obj->Array.capacity);
    #if MSGC    //protect obj and element
            gc_pushRoot((void*)&obj);
            gc_pushRoot((void*)&element);
            obj->Array.elements = gc_realloc(obj->Array.elements,(index + 1) * sizeof(oop));
            gc_popRoots(2);
    #else
            obj->Array.elements= realloc(obj->Array.elements,(index+1)*sizeof(oop));
    #endif
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
    #if MSGC    //protect obj and element
        gc_pushRoot((void*)&obj);
        gc_pushRoot((void*)&element);
        #if SBC
        obj->Array.elements = gc_realloc(obj->Array.elements,(index + 1) * sizeof(oop));
        #else //C++
        obj->Array.elements = (Object**)gc_realloc(obj->Array.elements, (index + 1) * sizeof(oop));
        #endif
        gc_popRoots(2);
    #else
        obj->Array.elements= realloc(obj->Array.elements,(index+1)*sizeof(oop));
    #endif
        obj->Array.capacity=index + 1;
        
        while(obj->Array.size<(index+1)){
            obj->Array.elements[obj->Array.size++]=nil;
        }
        return obj->Array.elements[index]=element;
    }
#endif


//ARRAY GET
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


//ARRAY PUSH
#if DEBUG
    oop _Array_push(char* file, int line,oop obj,oop element){
        DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
        if(obj->Array.size >= obj->Array.capacity){
    #if MSGC    //protect obj and element
            gc_pushRoot((void*)&obj);
            gc_pushRoot((void*)&element);
            obj->Array.elements = gc_realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
            gc_popRoots(2);
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
    #if MSGC    //protect obj and element
            gc_pushRoot((void*)&obj);
            gc_pushRoot((void*)&element);
            #if SBC
            obj->Array.elements = gc_realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
            #else //C++
            obj->Array.elements = (Object**)gc_realloc(obj->Array.elements, (obj->Array.size + 1) * sizeof(oop));
            #endif
            gc_popRoots(2);
    #else
            obj->Array.elements = realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
    #endif
            obj->Array.capacity += 1;
        }
        obj->Array.elements[obj->Array.size++] = element;
        return obj;
    };
#endif


//ARRAY POP
#if DEBUG
oop _Array_pop(char* file,int line,oop obj){
    DEBUG_ERROR_COND_REF(getType(obj)==Array,"Array but %d",getType(obj));
	if(obj->Array.size <= 0){
		DEBUG_ERROR("Array is empty [array_pop]\n");
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
		SHICA_FPRINTF(stderr,"Array is empty [array_pop]\n");
		exit(1);
	}
    oop element = obj->Array.elements[--obj->Array.size];
    obj->Array.elements[obj->Array.size] = nil;
    return element;
};
#endif


//ARRAY FREE NEED TO FIX it is not real free. header should be 000
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


//ARRAY ARGS COPY
//ADDME: add #IF MSGC and normal var
#if DEBUG
oop _Array_args_copy(char* file,int line,oop from,oop to)
{
    DEBUG_ERROR_COND_REF(getType(from)==Array,"Array[to] but %d",getType(from));
    DEBUG_ERROR_COND_REF(getType(to)==Array,"Array[from] but %d",getType(to));
    int toArrCap = to->Array.capacity;
    int fromArrSize = from->Array.size;
    DEBUG_ERROR_COND_REF(toArrCap > fromArrSize,"from Array size over toArray size");
    DEBUG_ERROR_COND_REF(to->Array.size==0,"Array[to] is not empty");
    //CHECK: protect from and to
    gc_pushRoot((void*)&from);
    gc_pushRoot((void*)&to);
    Array_push(to,new_Basepoint(1));//1st rbp, 2nd.. event args,
    gc_popRoots(2);
    for(int i=0;i<fromArrSize;i++){
        to->Array.elements[to->Array.size++] = from->Array.elements[i];
    }
    return to;
}
#define Array_args_copy(FROM,TO) _Array_args_copy(__FILE__,__LINE__,FROM,TO)
#else
oop Array_args_copy(oop from,oop to)
{
    int toArrCap = to->Array.capacity;
    int fromArrSize = from->Array.size;
    gc_pushRoot((void*)&from);
    gc_pushRoot((void*)&to);
    Array_push(to,new_Basepoint(1));//1st rbp, 2nd.. event args,
    gc_popRoots(2);
    for(int i=0;i<fromArrSize;i++){
        to->Array.elements[to->Array.size++] = from->Array.elements[i];
    }
    return to;
}
#endif



// QUEUE
#define SUCCESS     1       /* 成功 */
#define FAILURE     0       /* 失敗 */

oop newQueue(int capacity){
#if MSGC
    GC_PUSH(oop, node, newObject(Queue));
    #if SBC
    node->Queue.elements = gc_alloc(sizeof(oop)*capacity);
    #else //C++
    node->Queue.elements = (Object**)gc_alloc(sizeof(oop) * capacity);
    #endif
    GC_POP(node);
#else
    oop node = newObject(Queue);
    node->Queue.elements = calloc(capacity,sizeof(oop));
#endif
    node->Queue.head = 0;
    node->Queue.size = 0;
    return node;
}

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



//THREAD
oop _newThread(size_t vd_size,int stk_size,int num_args)
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
    #if SBC
        int* loc_cond = gc_beAtomic(gc_alloc(num_args*sizeof(int)));
        node->Thread.loc_cond = loc_cond;
        VD vd = gc_beAtomic(gc_alloc(vd_size));
    #else //C++
        int* loc_cond = (int*)gc_beAtomic(gc_alloc(num_args*sizeof(int)));
        VD vd = (VarData*)gc_beAtomic(gc_alloc(vd_size));
    #endif
    GC_POP(node);
#else
    VD       vd = calloc(1,vd_size);
#endif
    node->Thread.vd         = vd;
    return node;
}
#define newThread(VD_TYPE,STACK_SIZE,NUM_ARGS)	_newThread(sizeof(struct VD_TYPE),STACK_SIZE,NUM_ARGS)

oop _setThread(oop t,size_t size)
{
    gc_pushRoot((void*)&t);
    t->Thread.flag       =  0;
    t->Thread.pc         =  0;
    t->Thread.base       =  0;
    t->Thread.rbp        =  0;
    t->Thread.stack      = newArray(0);
#if MSGC
    #if SBC
        VD vd = gc_beAtomic(gc_alloc(size));
    #else //C++
        VD vd = (VarData*)gc_alloc(size);
    #endif
    gc_popRoots(1);
#else
    VD vd = calloc(1,size);
#endif
    t->Thread.vd         = vd;
    return t;
}
#define setThread(T,TYPE)	_setThread(T,sizeof(struct TYPE))

#endif //OBJECT_C