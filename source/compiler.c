#define COMPILER_C 1
#include "./compiler/setting.h"

#include <stdio.h>
FILE* SOURCE_FILE;
#define getchar() fgetc(SOURCE_FILE)

#include "./compiler/object.c"
#include "./compiler/parser/parser.c"
#include "./common/inst.c"
#include "./compiler/library/lib.c"

#include "./compiler/tool.c"

#include "./compiler/optimaize.c" 
#include "./compiler/generate.c"
#include "./common/memory.c"



#define ARRAY_DEF(name, size, ...) ({oop name##__ = intern("sss");})


int main(int argc, char const *argv[])
{
    nil   = newObject(Undefined);
    sys_false = _newInteger(0);
    sys_true  = _newInteger(1);
    none  = _newInteger(2);

    // コマンドライン引数の確認
    if (argc != 1){

        // ファイル名の拡張子をチェック
        if (!has_extension(argv[1],".txt")) {
            fprintf(stderr, "エラー: ファイルは .txt 拡張子でなければなりません\n");
            return 1;
        }

        // ファイルを読み込みモードで開く
        SOURCE_FILE = fopen(argv[1], "r");
        if (SOURCE_FILE == NULL) {
            perror("ファイルを開けませんでした");
            return 1;
        }

        setting_lib();
        entry_sym = intern("entry");
        

        oop program = newArray(0);
        Local_VNT   = newArray(0);
        Global_VNT  = newArray(0);
        state_Pair = nil;
        
        printf("\n \x1b[31m parsing ******************\x1b[0m\n\n");
        emitII(MSET,0);
        while (yyparse()) {
            if(sys_false == compile(program,result,nil,Undefined))break;
        }
        emitI(HALT);
        Array_put(program,1,_newInteger(Global_VNT->Array.size));// make a space for global value

        printf("\n \x1b[31m write code *********************\x1b[0m\n\n");
        CodeWrite(program);
        printCode(program);
        if(argc==3  && strcmp(argv[2],"-e")==0){
            memoryWriteC("code.c");
        }
        else memoryWrite("code.stt");

        
        free(program);
        free(symbols);
        memoryClear();
    }else{

        fprintf(stderr, "使用方法: %s <ファイル名>\n", argv[0]);
        return 1;

    }
    return 0;
}