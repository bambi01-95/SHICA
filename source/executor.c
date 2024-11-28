#define TEST 0
#define MSGC 1
#define MAXTHREADSIZE 10
#include <stdio.h>
FILE* SOURCE_FILE;
#define getchar() fgetc(SOURCE_FILE)

#include "./executor/object.c"

#include "./executor/library/lib.c"

#include "./common/inst.c"

#include "./executor/run.c"
#include "./common/memory.c"

#include "../lib/mingc/msgc.c"



int main(int argc, char const *argv[])
{
#if MSGC
    int memSize = 1024  * 32 * 10;//1024 * 2; // default memory size
    gc_init(memSize);
    gc_collectFunction = (gc_collectFunction_t)collectObjects;
    gc_markFunction    = (gc_markFunction_t)markObject;
    gc_isMarkFuction   = (gc_markFunction_t)isMarkObject;

    nil       = newAtomicObject(Undefined);
#else
    nil       = newObject(Undefined);
#endif
    // initialization 
    sys_false = _newInteger(0);
    sys_true  = _newInteger(1);
    none      = _newInteger(2);
    threads   = newArray(MAXTHREADSIZE);
#if MSGC
    gc_pushRoot((void*)&nil);
    gc_pushRoot((void*)&sys_false);
    gc_pushRoot((void*)&sys_true);
    gc_pushRoot((void*)&none);
    gc_pushRoot((void*)&threads);
#endif

    // コマンドライン引数の確認
    if (argc == 1){

        printf("    execute     %s\n",argv[0]);

        printf("\n \x1b[31m read code ******************\x1b[0m\n\n");
        memoryRead("code.stt");
#if TEST
        printf("\n \x1b[31m print byte code after memory read ******************\x1b[0m\n\n");
        printByteCode();
#endif
        printf("\n \x1b[31m main_execute code ******************\x1b[0m\n\n");
        main_execute();
    }else{
        fprintf(stderr, "使用方法: %s <ファイル名>\n", argv[0]);
        return 1;
    }
    return 0;
}