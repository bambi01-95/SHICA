#include "./executor/setting.h"

#include <stdio.h>

#if SBC
FILE* SOURCE_FILE;
#define getchar() fgetc(SOURCE_FILE)
#endif

#include "./executor/object.c"
// #include "./executor/library/lib.c"
#include "./common/memory.c"
#include "./common/inst.c"
#include "./executor/run.c"
#include "./executor/lib/msgc.c"


#if SBC
int main(int argc, char const *argv[])
#else
int SHICA_MAIN()
#endif
{
    if(!SBC && DEBUG){
        printf("DEBUG is not support enviroment of micro controller \n");
        exit(1);
    }
#if MSGC
    int memSize = 1024  * 256;//8 * 10;//1024 * 2; // default memory size
    gc_init(memSize);
    gc_collectFunction = (gc_collectFunction_t)collectObjects;
    gc_markFunction    = (gc_markFunction_t)markObject;
    gc_isMarkFuction   = (gc_markFunction_t)isMarkObject;
#if SBC
    nil       = gc_beAtomic(_newObject(sizeof(struct Undefined), Undefined));
#else //C++
nil = static_cast<oop>(gc_beAtomic(_newObject(sizeof(struct Undefined), Undefined)));
#endif
#else
    nil       = newObject(Undefined);
#endif
    // initialization 
    sys_false = _newInteger(0);
    sys_true  = _newInteger(1);
    none      = _newInteger(2);
    evalEventArgsThread = newThread(0,10);
    Array_push(evalEventArgsThread->Thread.stack,new_Basepoint(0));
#if MSGC
    gc_pushRoot((void*)&nil);
    gc_pushRoot((void*)&sys_false);
    gc_pushRoot((void*)&sys_true);
    gc_pushRoot((void*)&none);
    gc_pushRoot((void*)&evalEventArgsThread);
#endif
    // コマンドライン引数の確認
#if SBC
        memoryRead("code.stt");
#endif
#if DEBUG
        printf("\n \x1b[31m print byte code after memory read ******************\x1b[0m\n\n");
        printByteCode();
        printf("\n \x1b[31m implement main()******************\x1b[0m\n\n");
#endif
        main_execute();
    return 0;
}