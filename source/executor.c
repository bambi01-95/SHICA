#define TEST 1
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
    int memSize = 1024 * 1024 * 16;//1024 * 2; // default memory size
    gc_init(memSize);
    gc_collectFunction = (gc_collectFunction_t)collectObjects;
    gc_markFunction    = (gc_markFunction_t)markObject;
    gc_printFunction   = (gc_printFunction_t)printlnObject;
#endif
    // initialization 
    nil   = newAtomicObject(Undefined);
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
    // for(int i = 0; i<nroots; i++){
    //     printlnObject((oop)roots[i],0);
    // }
#endif
    // コマンドライン引数の確認
    if (argc == 1){

        // // ファイル名の拡張子をチェック
        // if (!has_extension(argv[1],".stt")) {
        //     fprintf(stderr, "エラー: ファイルは .txt 拡張子でなければなりません\n");
        //     return 1;
        // }

        printf("    execute     %d %s\n",argc,argv[0]);

        printf("\n \x1b[31m read code ******************\x1b[0m\n\n");
        
        memoryRead("code.stt");

        printf("\n \x1b[31m print byte code after memory read ******************\x1b[0m\n\n");
        printByteCode();
        printf("\n \x1b[31m main_execute code ******************\x1b[0m\n\n");
    //how many event function 
        main_execute();
        printf("\n \x1b[31m**************************\x1b[0m\n"); 
    }else{

        fprintf(stderr, "使用方法: %s <ファイル名>\n", argv[0]);
        return 1;

    }
    return 0;
}