#define COMPILER_C 1
#include "./compiler/setting.h"
#include "./compiler/error.c"
#include <stdio.h>
FILE* SOURCE_FILE;
#define getchar() fgetc(SOURCE_FILE)

#include "./compiler/object.c"
#include "./compiler/preprocess.c"
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
    wildcard_aop = intern("_wildcard_aop_");
    specific_aop = newArray(0);


    // コマンドライン引数の確認
    if (argc != 1){

        // ファイル名の拡張子をチェック
        if (!has_extension(argv[1],".txt")) {
            fprintf(stderr, "エラー: ファイルは .txt 拡張子でなければなりません\n");
            return 1;
        }
        char* inputFineName = strdup(argv[1]);
        // ファイルを読み込みモードで開く
        SOURCE_FILE = fopen(argv[1], "r");
        if (SOURCE_FILE == NULL) {
            perror("ファイルを開けませんでした");
            return 1;
        }

        setting_lib();
        entry_sym = intern("entry");
        exit_sym = intern("exit");
        state_Pair = nil;
        stateNameG = nil;
        

        oop program = newArray(0);
        oop programTrees = newArray(0);
        // STATE_SUBCORE_LISTS      = newArray(0);//REMOVE ME
        STATE_DEF_LOCAL_EVENT_LISTS = newArray(0);
        Local_VNT   = newArray(0);
        Global_VNT  = newArray(0);
        state_Pair = nil;
#if DEBUG
        printf("\n \x1b[31m preprocess ******************\x1b[0m\n\n");
#endif
        while(yyparse()){
            if(sys_false == preprocess(result,programTrees))break;
        }
#if DEBUG           
        printf("\n \x1b[31m check ******************\x1b[0m\n\n");
        printlnObject(programTrees,2);
        printEventBinaryData();
#endif  
        emitOI(MSET,0);
        oop *elements  = get(programTrees,Array,elements);
        int size = get(programTrees,Array,size);
        for(int i = 0; i<size; i++){
#if DEBUG
            printf("\n \x1b[31m%03d\x1b[0m\n",i);
#endif
            compile(program,elements[i],nil,Undefined);
        }
        emitO(HALT);
        Array_put(program,1,_newInteger(Global_VNT->Array.size));// make a space for global value
#if DEBUG
        printf("\n \x1b[31m write code *********************\x1b[0m\n\n");
        printCode(program);
#endif
        CodeWrite(program);
        char outputFileName[32];
        if(strcmp(inputFineName,"input.txt")!=0){
            change_extension(inputFineName,"stt",outputFileName,32);
        }else{
            strcpy(outputFileName,"code.stt");
        }
        
        if(argc==3  && strcmp(argv[2],"-e")==0){
            memoryWriteC(outputFileName);
        }
        else memoryWrite(outputFileName);

        
        free(program);
        free(symbols);
        memoryClear();
    }else{
        fprintf(stderr, "使用方法: %s <ファイル名>\n", argv[0]);
        return 1;
    }
    return 0;
}