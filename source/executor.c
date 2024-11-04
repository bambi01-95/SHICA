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
    int memSize = 1024 * 8;//1024 * 2; // default memory size
    gc_init(memSize);
    gc_collectFunction = (gc_collectFunction_t)collectObjects;
    gc_markFunction    = (gc_markFunction_t)markObject;
#endif
    // initialization 
    nil   = newObject(Undefined);
    sys_false = _newInteger(0);
    sys_true  = _newInteger(1);
    none  = _newInteger(2);
    threads = newArray(MAXTHREADSIZE);
#if MSGC
    gc_pushRoot(nil);
    gc_pushRoot(sys_false);
    gc_pushRoot(sys_true);
    gc_pushRoot(none);
    gc_pushRoot(entry_sym);
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
        struct timeval startTime, endTime;  // 構造体宣言
        clock_t startClock, endClock;       // clock_t型変数宣言
        
        gettimeofday(&startTime, NULL);     // 開始時刻取得
        startClock = clock();               // 開始時刻のcpu時間取得
        
        memoryRead("code.stt");

        printf("\n \x1b[31m print byte code after memory read ******************\x1b[0m\n\n");
        printByteCode();
        printf("\n \x1b[31m main_execute code ******************\x1b[0m\n\n");
    //how many event function 
        main_execute();
        printf("\n \x1b[31m**************************\x1b[0m\n"); 
        gettimeofday(&endTime, NULL);       // 開始時刻取得
        endClock = clock();                 // 開始時刻のcpu時間取得
        
        time_t diffsec = difftime(endTime.tv_sec, startTime.tv_sec);    // 秒数の差分を計算
        suseconds_t diffsub = endTime.tv_usec - startTime.tv_usec;      // マイクロ秒部分の差分を計算
        // 以下の処理は不要(15/10/28)
        if (diffsub < 0) {                                              // マイクロ秒が負になったとき
           diffsec -= 1;                                               // 秒部分を繰り下げ
           diffsub = 1000000 + diffsub;                                // 1秒との差
        }
        double realsec = diffsec+diffsub*1e-6;                          // 実時間を計算
        double cpusec = (endClock - startClock)/(double)CLOCKS_PER_SEC; // cpu時間を計算
        printf("%f\n",realsec);
        printf("%f\n",cpusec);
        double percent = 100.0*cpusec/realsec;                          // 使用率を100分率で計算
        printf("CPU使用率%f %%\n", percent);                            // 表示
        printf("\n\n  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *\n");
        printf(" *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *\n");
        printf("*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * \n\n");
    }else{

        fprintf(stderr, "使用方法: %s <ファイル名>\n", argv[0]);
        return 1;

    }
    return 0;
}