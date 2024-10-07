#define TEST 0
#include <stdio.h>
FILE* SOURCE_FILE;
#define getchar() fgetc(SOURCE_FILE)

#include "object/object.c"

#include "library/lib_compile/lib_compile.c"

#include "print/print.c"

#include "parser/parser.c"

#include "inst/inst.c"

#include "compiler/compiler.c"

#include "executer/executer.c"

int has_txt_extension(const char *filename) {
    const char *ext = strrchr(filename, '.');  // ファイル名中の最後の'.'を探す
    if (ext != NULL && strcmp(ext, ".txt") == 0) {
        return 1;  // .txt拡張子がある場合は1を返す
    }
    return 0;      // それ以外の場合は0を返す
}

oop printCode(oop);
oop printByteCode();

#define ARRAY_DEF(name, size, ...) ({oop name##__ = intern("sss");})


int main(int argc, char const *argv[])
{
    nil   = newObject(Undefined);
    sys_false = _newInteger(0);
    sys_true  = _newInteger(1);
    none  = _newInteger(2);

    // コマンドライン引数の確認
    if (argc == 2){

        // ファイル名の拡張子をチェック
        if (!has_txt_extension(argv[1])) {
            fprintf(stderr, "エラー: ファイルは .txt 拡張子でなければなりません\n");
            return 1;
        }

        // ファイルを読み込みモードで開く
        SOURCE_FILE = fopen(argv[1], "r");
        if (SOURCE_FILE == NULL) {
            perror("ファイルを開けませんでした");
            return 1;
        }

        setting_stdlib();
        entry_sym = intern("entry");
        ARRAY_DEF(heke,1,2);
        newSymbol("exit");
        state_Pair = nil;
        oop program = newArray(0);
        Local_VNT   = newArray(0);
        Global_VNT  = newArray(0);
        
        printf("    compile     %d%s\n",argc,argv[0]);
        printf("\n \x1b[31m parsing ******************\x1b[0m\n\n");
        emitII(MSET,0);
        while (yyparse()) {
            // printlnObject(result, 0);
            if(sys_false == compile(program,result,nil,Undefined))break;
        }
        emitI(HALT);
        Array_put(program,1,_newInteger(Global_VNT->Array.size));
    #if TEST
        printf("Successful\n");
        printf("\n \x1b[31m print code *********************\x1b[0m\n\n");
        printCode(program);
    #endif
        printf("\n \x1b[31m write code *********************\x1b[0m\n\n");
        CodeWrite(program);
        memoryWrite("code.stt");
    #if TEST
        printf("\n \x1b[31m print byte code after memory write ******************\x1b[0m\n\n");
        printByteCode();
    #endif
        
    #if TEST
        printf("\n \x1b[31m print byre code *********************\x1b[0m\n\n");
        printf("memory_size %zu\n",memsize);
    #endif
        free(program);
        free(symbols);
        memoryClear();

    }else if(argc == 1){

        printf("    execute     %d %s\n",argc,argv[0]);

        printf("\n \x1b[31m read code ******************\x1b[0m\n\n");
        struct timeval startTime, endTime;  // 構造体宣言
        clock_t startClock, endClock;       // clock_t型変数宣言
        
        gettimeofday(&startTime, NULL);     // 開始時刻取得
        startClock = clock();               // 開始時刻のcpu時間取得

        memoryRead("code.stt");
    #if TEST
        printf("memory_size %zu\n",memsize);
    #endif
        printf("\n \x1b[31m print byte code after memory read ******************\x1b[0m\n\n");
        printByteCode();
        printf("\n \x1b[31m main_execute code ******************\x1b[0m\n\n");
        main_execute();
        printf("\n \x1b[31m**************************\x1b[0m\n"); 
        gettimeofday(&endTime, NULL);       // 開始時刻取得
        endClock = clock();                 // 開始時刻のcpu時間取得
        
        time_t diffsec = difftime(endTime.tv_sec, startTime.tv_sec);    // 秒数の差分を計算
        suseconds_t diffsub = endTime.tv_usec - startTime.tv_usec;      // マイクロ秒部分の差分を計算
        //以下の処理は不要(15/10/28)
        //if (diffsub < 0) {                                              // マイクロ秒が負になったとき
        //    diffsec -= 1;                                               // 秒部分を繰り下げ
        //    diffsub = 1000000 + diffsub;                                // 1秒との差
        //}
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