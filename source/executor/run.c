#ifndef EXECUTER_C
#define EXECUTER_C

#include "./object.c"
#include "./tool.c"
#include "../common/inst.c"
#include "../common/liblist/library.h"
#include "../common/liblist/stdlib.h"
#include "../common/memory.c"
#include "./library/stdlib-execute.c"


// END QUEUE

FLAG sub_execute(oop process,oop GM);
oop newBoolean(int flag) { return flag ? sys_true : sys_false; }

oop Event_userlib(int eve_num,oop stack){
    switch(eve_num){
        default:
            fprintf(stderr,"this is not happen Event_userlib\n");
    }
    exit(1);
}

// EVENT...
oop Event_Func(int lib_num,int eve_num,oop stack,int stk_size){
    switch(lib_num){
        case STDLIB:{
            return Event_stdlib(eve_num,stack,stk_size);
        }
        case USERLIB:{
            return 0;
        }
        default:{
            printf("ERROR: Event lib[%d] eve[%d]\n",lib_num, eve_num);
            exit(1);
        }
    }
    printf("Event_Func(): not happen..\n");
    return 0;
}

char inst      = 0;
char char_value    = 0;
int  int_value      = 0;
long long int long_value = 0;
float float_value   = 0;
double double_value = 0;
char   string_value[256];

#define getData(D,PC,S) memcpy(&D,&memory[PC],S); PC+=S
#define getInst(PC)     memcpy(&inst,&memory[PC],SIZE_INST); PC+=SIZE_INST
#define getChar(PC)     memory[PC++]
#define getInt(PC)      memcpy(&int_value,&memory[PC],SIZE_INT);PC+=SIZE_INT 
#define getLong(PC)     memcpy(&long_value,&memory[PC],SIZE_LONG);PC+=SIZE_LONG
#define getFloat(PC)    memcpy(&float_value,&memory[PC],SIZE_FLOAT);PC+=SIZE_FLOAT
#define getDouble(PC)   memcpy(&double_value,&memory[PC],SIZE_DOUBLE);PC+=SIZE_DOUBLE



#define getString(PC)   ({ \
    int i = 0; \
    while(memory[PC]!='\0'){ \
        string_value[i++] = memory[PC++]; \
    } \
    string_value[i] = memory[PC++]; \
})

//-----------_----------__------______----________--__________


#include <unistd.h>//remove


void main_execute(){
    int test_impliment_limit = 0; //REMOVEME: TEST only 
    int pc = 0;
    int rbp = 0;
    // oop stack = newArray(20); //動的にメモリを確保するようにしないとメモリが被ってしまうため、スタックのデータがかぶる。
    GC_PUSH(oop,stack,newArray(20));
    // GROBAL MEMORY
    // oop GM    = newThread(Default,10);
    GC_PUSH(oop,GM,newThread(Default,10));
    // GM->Thread.stack = newArray(30);
    // GM->Thread.rbp   = 0;
    for(;;){
        getInst(pc);
        switch(inst){
            case i_load:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(pc);
                Array_push(stack,_newInteger(int_value));
                continue;
            }

            case THREAD:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(pc);
                int num_thread = int_value;
                if(num_thread == 0)continue;
#if DEBUG //CHECK ME: 本番環境でここどうするのか？
                DEBUG_ERROR_COND(MAXTHREADSIZE >= num_thread,"Number of Event definitin is over setting value");
                DEBUG_ERROR_COND(Array==getType(threads),"TYPE is %d",getType(threads));
#endif       
                printf("%s line %d\n",__FILE__,__LINE__);
                exit(1);
                int thread_pc = pc;
                
            //init setting thread スレッドの初期設定
                printf("\n              setting\n\n");
                for(int thread_index=0; thread_index<num_thread;/*case CALL_E, thread_index++*/){
                    getInst(pc);
                    switch(inst){
                        case i_load:{// load pin number / load location of event task
#if TEST  
line();printf("THREAD => %s\n",INSTNAME[inst]);
#endif
                            getInt(pc);
                            Array_push(stack,_newInteger(int_value));
                            continue;
                        }
                        case CALL_E:{//now
#if TEST  
line();printf("THREAD => %s\n",INSTNAME[inst]);
#endif
                            getInt(pc);int lib_num = int_value;
                            getInt(pc);int eve_num = int_value;
                            getInt(pc);int pin_num = int_value;
                            assert(Array == getType(stack));
                            threads = Array_push(threads,Event_Func(lib_num,eve_num,stack,120/num_thread));              // connect function of event / イベントの関数と紐付け                            assert(Array == getType(stack));
                            
                            int inst_loc = thread_pc + _Integer_value(Array_pop(stack));
                            DEBUG_LOG("this isi hrere\n%d",inst_loc);
                            threads->Array.elements[thread_index]->Thread.pc   = inst_loc;
                            DEBUG_LOG("this isi hrere\n%d",inst_loc);
                            threads->Array.elements[thread_index]->Thread.base = inst_loc;
                            DEBUG_LOG("this isi hrere\n%d",inst_loc);
                            thread_index++;
                            
                            continue;
                        }
                        default:{
                            printf("this is not happen, main main_execute case THREAD\n");
                            exit(1);
                        }
                    }
                }

                DEBUG_LOG("\n              implement\n\n");
    // implement thread/Event concurrelty
                for(int is_active = 0,count = 5; is_active==0;){
                    count++;
                    for(int i =0;i<num_thread;i++){
                        threads->Array.elements[i]->Thread.func(threads->Array.elements[i]);             //implement function of event
                        if(threads->Array.elements[i]->Thread.flag == 1){
                            FLAG flag = sub_execute(threads->Array.elements[i],GM);
                            if(flag == F_TRANS){//TRANS
                                DEBUG_LOG("        TRANS\n");
                                int pc_i = threads->Array.elements[i]->Thread.pc++;//location of thread[i]'s pc
                                getInt(pc_i);//thread num<-pc_i
                                pc = int_value + pc_i;
                                // printf("pc-> %d\n",pc);
                                is_active = 1;
                                int gm_size = GM->Thread.stack->Array.size;
                                for(int i = gm_size;i>GM->Thread.rbp;i--){
                                    Array_pop(GM->Thread.stack);
                                }
                                break;
                            }
                            else if(flag == F_EOE){//EOE
                                threads->Array.elements[i]->Thread.flag = 0;
                                threads->Array.elements[i]->Thread.pc = threads->Array.elements[i]->Thread.base;
                                // Array_free(threads->Array.elements[i]->Thread.stack);
                                if(test_impliment_limit>1000000){
                                    printf("\n\n");
                                    return;
                                }else test_impliment_limit++;
                            }

                        }
                        else if(threads->Array.elements[i]->Thread.queue->Queue.size>0){
                                threads->Array.elements[i]->Thread.flag = 1;
                                oop variables =  dequeue(threads->Array.elements[i]->Thread.queue);
                                Array_args_copy(variables,threads->Array.elements[i]->Thread.stack);
                        }
                    }
                    // if(count>5){
                    //     count = 0;
                    // }
                }
                Array_free(threads);
                continue;
            }
            case DEFINE_L:{
                getInt(pc);
                oop data  = Array_pop(stack);
                Array_put(GM->Thread.stack,GM->Thread.rbp + int_value, data);
                continue;
            }
            case GLOBAL:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                oop code = newThread(Default,10);//FIXME: using new thread here is not good for ...
                code->Thread.pc = pc;
                for(;;){
                    FLAG flag = sub_execute(code,GM);
                    if(flag == F_EOE)break;
                }
                pc = code->Thread.pc;
                // free(code);//IFERROR
                continue;
            }
            case ENTRY:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(pc);
                int s_pc = pc;//store corrent pc
                oop code = newThread(Default,20);//FIXME: this is  not good for memory
                code->Thread.pc = pc + int_value;
                Array_push(code->Thread.stack,new_Basepoint(0));
                for(;;){
                    FLAG flag = sub_execute(code,GM);
                    if(flag == F_EOE)break;
                }
                // free(code);
                pc = s_pc;
                continue;                
            }
            case MSET:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(pc);GM->Thread.rbp = int_value;
                for(int i =0;i<int_value ;i++){
                    Array_push(GM->Thread.stack,nil);
                }
                continue;
            }
            case JUMP:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(pc); pc += int_value;
                continue;       
            }
            case HALT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                return;
            }
            default:fprintf(stderr,"main_execute error %s\n",INSTNAME[inst]);
        }
    }
}



#define mpc    process->Thread.pc
#define mrbp   process->Thread.rbp
#define mstack process->Thread.stack

#define api()     _Integer_value(Array_pop(mstack))
#define apl()     Array_pop(mstack)->_Long.value
#define apf()     _Float_value(Array_pop(mstack))
#define apd()     Array_pop(mstack)->_Double.value
#define aps()     Array_pop(mstack)->String.value
#define apc()    _Char_value(Array_pop(mstack))


// ライブラリ関数
void stdlib_print(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int value = api();
    printf("%d\n",value);
    return;
}

void stdlib_itoc(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    char value = (char)api();
    oop c = _newChar(value);
    Array_push(mstack,c);
    return; 
}

void stdlib_exit(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int value = api();
    exit(value);
    return;
}

void stdlib_appendchar(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    char* str = aps();
    char c = apc();
    size_t length = strlen(str);
#if MSGC
    char* newStr = (char*)gc_alloc((length + 2) * sizeof(char)); // 新しい文字列のメモリを確保
#else
    char* newStr = (char*)malloc((length + 2) * sizeof(char)); // 新しい文字列のメモリを確保
#endif
    if (newStr != NULL){
        strcpy(newStr, str); // 元の文字列をコピー
        newStr[length] = c;  // 追加する文字をセット
        newStr[length + 1] = '\0'; // ヌル終端を追加
    }
    Array_push(mstack,newString(newStr));
    return;
}

// void stdlib_time(oop process, oop GM){
//     getInt(mpc);int size_args = int_value;
//     int t = time(1);
//     Array_push(mstack,_newInteger(t));
// }

// void stdlib_queue(oop process, oop GM){
//     getInt(mpc);int size_args = int_value;
//     while(size_args--){
        
//     }
// }

void lib_stdlib(oop process,oop GM){
    getInt(mpc);int func_num = int_value;
    switch(func_num){
        case OUTPUT_P:{
            stdlib_print(process,GM);
            break;
        }
        case ITOC_P:{
            stdlib_itoc(process,GM);
            break;
        }
        case EXIT_P:{
            stdlib_exit(process,GM);
            break;
        }
        case APPENDCHAR_P:{
            stdlib_appendchar(process,GM);
            break;
        }
        default:
            break;
    }
}


oop Call_Primitive(oop process,oop GM){
    getInt(mpc);int lib_num = int_value;
    switch(lib_num){
        case STDLIB:{
            lib_stdlib(process,GM);
            break;
        }
        default:{
            printf("Call_Primitive %d\n",lib_num);
            exit(1);
        }
    }
    return nil;
}
////////////////////////////////////////////////




FLAG sub_execute(oop process,oop GM){
    for(;;){
        getInst(mpc);
        switch(inst){
            case TRANS:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                return F_TRANS;
            }
            case i_load:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                Array_push(mstack,_newInteger(int_value));
                continue;
            }
            case l_load:
            {
                getLong(mpc);
                Array_push(mstack,_newLong(long_value));
                continue;
            }
            case f_load:{
                getFloat(mpc);
                Array_push(mstack,_newFloat(float_value));
                continue;
            }
            case d_load:{
                getDouble(mpc);
                Array_push(mstack,_newDouble(double_value));
                continue;
            }
            case c_load:{
                char vlaue = getChar(mpc);
                Array_push(mstack,_newDouble(double_value));
                continue;    
            }
            case s_load:{
                getString(mpc);
                Array_push(mstack,newString(string_value));
                continue;
            }
        // list or array of integer
            // case il_load:{
            //     getInt(mpc);
            //     oop list = _newIntegerArray(_Integer_value(Array_pop(mstack)));
            //     for(int i=0;i<int_value;i++){
            //         list->_IntegerArray.array[i] = Array_pop(mstack);
            //     }
            //     Array_put(mstack,int_value,list);
            //     continue;
            // }
/* _Integer */
            case i_EQ:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l == r));
                continue;
            }
            case i_NE:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l != r));
                continue;
            }
            case i_LT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l <  r));
                continue;
            }
            case i_LE:{//inprogress
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l <= r));
                continue;
            }
            case i_GE:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l >= r));
                continue;
            }
            case i_GT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l >  r));
                continue;
            }
            case i_AND:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l && r));
                continue;
            }
            case i_OR:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l || r));
                continue;
            }
            case i_ADD:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l + r));
                continue;
            }
            case i_SUB:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l - r));
                continue;
            }
            case i_MUL:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l * r));
                continue;
            }
            case i_DIV:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l / r));
                continue;
            }
            case i_MOD:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l % r));
                continue;
            }
/* Long */
            case l_EQ:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                long long int r = apl(),l = apl();
                if(l==r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case l_NE:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                long long int r = apl(),l = apl();
                if(l!=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case l_LT: {
                long long int r = apl(),l = apl();
                if(l< r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case l_LE: {
                long long int r = apl(),l = apl();
                if(l<=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case l_GE:{
                long long int r = apl(),l = apl();
                if(l>=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            } 
            case l_GT:{
                long long int r = apl(),l = apl();
                if(l< r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            } 
            case l_ADD:{
                long long int r = apl(),l = apl();
                Array_push(mstack,_newLong(l+r));
                continue;
            } 
            case l_SUB:{
                long long int r = apl(),l = apl();
                Array_push(mstack,_newLong(l-r));
                continue;
            } 
            case l_MUL:{
                long long int r = apl(),l = apl();
                Array_push(mstack,_newLong(l*r));
                continue;
            } 
            case l_DIV:{
                long long int r = apl(),l = apl();
                Array_push(mstack,_newLong(l/r));
                continue;
            } 
            case l_MOD:{
                long long int r = apl(),l = apl();
                Array_push(mstack,_newLong(l%r));
                continue;
            } 
/* _Float */
            case f_EQ:{
                float r = apf(),l = apf();
                if(l==r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case f_NE:{
                float r = apf(),l = apf();
                if(l!=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case f_LT:{
                float r = apf(),l = apf();
                if(l< r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            } 
            case f_LE: {
                float r = apf(),l = apf();
                if(l<=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case f_GE: {
                float r = apf(),l = apf();
                if(l>=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case f_GT: {
                float r = apf(),l = apf();
                if(l< r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case f_ADD:{
                float r = apf(),l = apf();
                Array_push(mstack,_newFloat(l+r));
                continue;
            } 
            case f_SUB:{
                float r = apf(),l = apf();
                Array_push(mstack,_newFloat(l-r));
                continue;
            } 
            case f_MUL:{
                float r = apf(),l = apf();
                Array_push(mstack,_newFloat(l*r));
                continue;
            } 
            case f_DIV:{
                float r = apf(),l = apf();
                Array_push(mstack,_newFloat(l/r));
                continue;
            } 
/* Double */
            case d_EQ:{
                double r = apd(),l = apd();
                if(l==r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case d_NE:{
                double r = apd(),l = apd();
                if(l!=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case d_LT:{
                double r = apd(),l = apd();
                if(l< r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            } 
            case d_LE:{
                double r = apd(),l = apd();
                if(l<=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            } 
            case d_GE:{
                double r = apd(),l = apd();
                if(l>=r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case d_GT:{
                double r = apd(),l = apd();
                if(l> r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            } 
            case d_ADD:{
                double r = apd(),l = apd();
                Array_push(mstack,_newDouble(l+r));
                continue;
            } 
            case d_SUB:{
                double r = apd(),l = apd();
                Array_push(mstack,_newDouble(l-r));
                continue;
            } 
            case d_MUL:{
                double r = apd(),l = apd();
                Array_push(mstack,_newDouble(l*r));
                continue;
            } 
            case d_DIV:{
                double r = apd(),l = apd();
                Array_push(mstack,_newDouble(l/r));
                continue;
            } 


            case s_EQ: {
                char* r = aps(),*l = aps();
                if(strcmp(l,r)==0)Array_push(mstack,sys_true);
                else Array_push(mstack,sys_false);
                continue;
            } 
            case s_NE: {
                char* r = aps(),*l = aps();
                if(strcmp(l,r)!=0)Array_push(mstack,sys_true);
                else Array_push(mstack,sys_false);
                continue;
            } 
            case s_LT: {
                char* r = aps(),*l = aps();
                if(strcmp(l,r)<0)Array_push(mstack,sys_true);
                else Array_push(mstack,sys_false);
                continue;
            } 
            case s_LE:{
                char* r = aps(),*l = aps();
                if(strcmp(l,r)<=0)Array_push(mstack,sys_true);
                else Array_push(mstack,sys_false);
                continue;
            } 
            case s_GE: {
                char* r = aps(),*l = aps();
                if(strcmp(l,r)>=0)Array_push(mstack,sys_true);
                else Array_push(mstack,sys_false);
                continue;
            } 
            case s_GT: {
                char* r = aps(),*l = aps();
                if(strcmp(l,r)>0)Array_push(mstack,sys_true);
                else Array_push(mstack,sys_false);
                continue;
            }
            case s_ADD:{
                oop r = Array_pop(mstack);
                oop l = Array_pop(mstack);
                oop new = newString(strcat(strdup(l->String.value),r->String.value));
                Array_push(mstack,new);
                continue;
            }





/* end calc */
            case CALL:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                Array_push(mstack,_newInteger(int_value));//number of args
                getInt(mpc);
                Array_push(mstack,_newInteger(mpc));//CHECKME: funtion index?
                mpc = mpc + int_value;
                return F_NONE;
            }
            case CALL_P:{
#if TEST
printf("CALL_P\n");
#endif
                Call_Primitive(process,GM);
                return F_NONE;
            }
            case GET:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                Array_push(mstack,Array_get(mstack,mrbp + int_value));//index
                continue;
            }
            case GET_L:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                // printf("    %s\n",TYPENAME[getType(mstack)]);
                Array_push(mstack,Array_get(GM->Thread.stack,GM->Thread.rbp + int_value));//index
                continue;
            }
            case GET_G:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                Array_push(mstack,Array_get(GM->Thread.stack,int_value));//index
                continue;
            }
            case DEFINE:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                oop data  = Array_pop(mstack);
                Array_put(mstack,mrbp + int_value, data);
                continue;
            }
            case DEFINE_L:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                oop data  = Array_pop(mstack);
                Array_put(GM->Thread.stack,GM->Thread.rbp + int_value, data);
                continue;
            }
            case DEFINE_G:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                oop data = Array_pop(mstack);
                Array_put(GM->Thread.stack,int_value,data);
                continue;
            }
            case DEFINE_List:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                DEBUG_LOG("this is not support now");
                // getInt(mpc);
                // int index = _Integer_value(Array_pop(mstack));
                // oop data  = Array_pop(mstack);
                // //FIXME: サイズの確認とかやってない
                // GM->Thread.stack->Array.elements[int_value]->_IntegerArray.array[index] = data;
                // continue;
            }
            case GLOBAL_END:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                return F_EOE;
            }
            case SETQ:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                continue;
            }
            case RET:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                oop value = Array_pop(mstack);//return value
                oop data = nil;
                while(getType(data = Array_pop(mstack)) != _BasePoint);
                mrbp = get(data,_BasePoint,adress);
                mpc = api();//next mpc 
                int num_arg = api();
                for(int i = 0;i<num_arg;i++)
                    Array_pop(mstack);
                Array_push(mstack,value);
                return F_NONE;
            }
            case JUMPF:{
#if TEST  
line();printf("%s [%d]\n",INSTNAME[inst],inst);
#endif
                getInt(mpc);//get offset inpro
                oop cond = Array_pop(mstack);
                //FIXME: usign sys_false and sys_true
                if(cond == sys_false)mpc += int_value;//offset
                else if(cond == sys_true)return F_NONE;
                else if(_Integer_value(cond)==0)mpc += int_value;//offset
                return F_NONE;
            }
            case JUMP:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);     //get offset
                mpc += int_value;//get offset
                return F_NONE;//go next thread
            }
            case MSUB:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                //now
                mstack = Array_push(mstack, new_Basepoint(mrbp));//before mrpb
                mrbp = mstack->Array.size;            //current mrpb
                getInt(mpc);//memry
                int memory_size = int_value;
                for(int i=0;i<memory_size;i++){
                    Array_push(mstack,nil);//put nil into mstack for variable
                }
                continue;
            }
            case MPOP:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                for(;;){
                    oop data = Array_pop(mstack);
                    if(getType(data) == _BasePoint){
                        mrbp = data->_BasePoint.adress;
                        break;
                    }
                }
                continue;
            }
            case MPICK:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                getInt(mpc);
                int adress = int_value;
                getInt(mpc);
                int index  = int_value;
                oop data   = Array_get(mstack, mrbp - adress -1);
                Array_put(mstack,mrbp + index,data);
                continue;
            }
            case i_PRINT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                printf("%d\n",_Integer_value(Array_pop(mstack)));
                continue;
            }
            case l_PRINT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                printf("%lld\n",Array_pop(mstack)->_Long.value);
                continue;
            }
            case f_PRINT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                printf("%f\n",_Float_value(Array_pop(mstack)));
                continue;
            }
            case d_PRINT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                printf("%f\n",Array_pop(mstack)->_Double.value);
                continue;
            }
            case c_PRINT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                printf("%c\n",_Char_value(Array_pop(mstack)));
                continue;
            }
            case s_PRINT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                printf("%s\n",Array_pop(mstack)->String.value);
                continue;
            }
            case EOE:{
                // mpc  = mrbp;
                mpc -= 1;
                DEBUG_ERROR_COND(mstack->Array.size == 0,"IMPRIMENT ERROR: stack size is %d\n",mstack->Array.size);
                if(mstack->Array.size != 0){
                    printlnObject(Array_pop(mstack),1);
                    exit(1);
                }
                return F_EOE;
            }
            case HALT:{
#if TEST  
line();printf("%s\n",INSTNAME[inst]);
#endif
                if(Array_size(mstack)==1){
                    printf("HALT-----------------------\n");
                    printlnObject(Array_pop(mstack),1);
                    return F_NONE;
                }
                printf("HALT with %d items on mstack\n",Array_size(mstack));
                int size = Array_size(mstack);
                for(int i = 0; i<size;i++)
                    printlnObject(Array_pop(mstack),1);
                return F_NONE;
            }
        }
        return F_NONE;
    } 
}




oop printByteCode(){
    int pc = 0;
    for(;;){
        printf("%3d ",pc);
        getInst(pc);
        switch(inst){
            case TRANS:  getInt(pc);printf("TRANS     %3d\n",int_value);continue;
            case i_load: getInt(pc);printf("i_load    %3d\n",int_value);continue;
            case l_load: getLong(pc);printf("l_load    %3lld\n",long_value);continue;
            case f_load: getFloat(pc);printf("f_load    %3f\n",float_value);continue;
            case d_load: getDouble(pc);printf("d_load    %3f\n",double_value);continue;
            case c_load: printf("c_load    %3c\n",getChar(pc));continue;
            case s_load: getString(pc);printf("s_load    %s\n",string_value);continue;
            case il_load: getInt(pc);printf("il_load    %d\n",int_value);continue;
            case i_EQ:   printf("i_EQ\n"); continue; 
            case i_NE:   printf("i_NE\n"); continue; 
            case i_LT:   printf("i_LT\n"); continue; 
            case i_LE:   printf("i_LE\n"); continue; 
            case i_GE:   printf("i_GE\n"); continue; 
            case i_GT:   printf("i_GT\n"); continue; 
            case i_ADD:  printf("i_ADD\n");continue;
            case i_SUB:  printf("i_SUB\n");continue;
            case i_MUL:  printf("i_MUL\n");continue;
            case i_DIV:  printf("i_DIV\n");continue;
            case i_MOD:  printf("i_MOD\n");continue;
            case l_EQ:   printf("l_EQ\n");continue; 
            case l_NE:   printf("l_NE\n");continue; 
            case l_LT:   printf("l_LT\n");continue; 
            case l_LE:   printf("l_LE\n");continue; 
            case l_GE:   printf("l_GE\n");continue; 
            case l_GT:   printf("l_GT\n");continue; 
            case l_ADD:  printf("l_ADD\n");continue;
            case l_SUB:  printf("l_SUB\n");continue;
            case l_MUL:  printf("l_MUL\n");continue;
            case l_DIV:  printf("l_DIV\n");continue;
            case l_MOD:  printf("l_MOD\n");continue;
            case f_EQ:   printf("f_EQ\n");continue; 
            case f_NE:   printf("f_NE\n");continue; 
            case f_LT:   printf("f_LT\n");continue; 
            case f_LE:   printf("f_LE\n");continue; 
            case f_GE:   printf("f_GE\n");continue; 
            case f_GT:   printf("f_GT\n");continue; 
            case f_ADD:  printf("f_ADD\n");continue;
            case f_SUB:  printf("f_SUB\n");continue;
            case f_MUL:  printf("f_MUL\n");continue;
            case f_DIV:  printf("f_DIV\n");continue;
            case d_EQ:   printf("d_EQ\n");continue; 
            case d_NE:   printf("d_NE\n");continue; 
            case d_LT:   printf("d_LT\n");continue; 
            case d_LE:   printf("d_LE\n");continue; 
            case d_GE:   printf("d_GE\n");continue; 
            case d_GT:   printf("d_GT\n");continue; 
            case d_ADD:  printf("d_ADD\n");continue;
            case d_SUB:  printf("d_SUB\n");continue;
            case d_MUL:  printf("d_MUL\n");continue;
            case d_DIV:  printf("d_DIV\n");continue;

            case s_EQ:   printf("d_EQ\n");continue; 
            case s_NE:   printf("d_NE\n");continue; 
            case s_LT:   printf("d_LT\n");continue; 
            case s_LE:   printf("d_LE\n");continue; 
            case s_GE:   printf("d_GE\n");continue; 
            case s_GT:   printf("d_GT\n");continue; 
            case s_ADD:  printf("d_ADD\n");continue;

            case THREAD: getInt(pc);printf("thread    %3d\n",int_value);continue;
            case EOE:    printf("EOE\n");continue;

            case CALL:{
                printf("CALL      ");
                getInt(pc);int num_arg = int_value;
                getInt(pc);int index = int_value;
                printf("%3d  %3d\n",num_arg,index);
                continue;
            }
            case CALL_P:{
                printf("CALL_P      ");
                getInt(pc);int lib_num = int_value;
                getInt(pc);int func_num = int_value;
                getInt(pc);int args_s = int_value;
                printf("%3d  %3d  %3d\n",lib_num,func_num,args_s);
                continue;
            }
            case CALL_E:{
                printf("CALL_E      ");
                getInt(pc);int lib_num = int_value;
                getInt(pc);int func_num = int_value;
                getInt(pc);int args_s = int_value;
                printf("%3d  %3d  %3d\n",lib_num,func_num,args_s);
                continue;
            }
            case GET:{
                printf("Get       ");//T
                getInt(pc);int symbol = int_value;
                printf("%3d\n",symbol);
                continue;
            }
            case GET_L:{
                printf("Get_L     ");//T
                getInt(pc);int symbol = int_value;
                printf("%3d\n",symbol);
                continue;
            }
            case GET_G:{
                printf("GET_G     ");
                getInt(pc);int symbol = int_value;
                printf("%3d\n",symbol);
                continue;
            }
            case DEFINE:{
                printf("DEFINE    ");//T
                getInt(pc);int symbol = int_value;
                printf("%3d\n",symbol);//T
                continue;
            }
            case DEFINE_L:{
                printf("DEFINE_L  ");//T
                getInt(pc);int symbol = int_value;
                printf("%3d\n",symbol);//T
                continue;
            }
            case DEFINE_G:{
                printf("DEFINE_List  ");//T
                getInt(pc);int symbol = int_value;
                printf("%3d\n",symbol);//T
                continue;
            }  
            case GLOBAL:{
                printf("GLOBAL\n");
                continue;
            }
            case GLOBAL_END:{
                printf("GLOBAL_END\n");
                continue;
            }
            case ENTRY:{
                printf("ENTRY     ");
                getInt(pc);int index = int_value;
                printf("%3d\n",index);//T
                continue;
            }
            case SETQ:{
                printf("SETQ\n");
                continue;
            }
            case RET:{
                printf("RET       ");
                printf("  X\n");
                continue;
            }
            case MSUB:{
                printf("MSUB      ");
                getInt(pc);printf("%3d\n",int_value);
                continue;
            }
            case MPOP:{
                printf("MPOP\n");
                continue;
            }
            case MPICK:{
                printf("MPICK     ");
                getInt(pc);int i = int_value;
                getInt(pc);int j = int_value;
                printf("%3d %3d\n",i,j);
                continue;
            }
            case MSET:{
                printf("MSET      ");
                getInt(pc);
                printf("%3d\n",int_value);
                continue;
            }
            case JUMPF:{
                printf("jumpF     ");//T
                getInt(pc);
                int offset = int_value; 
                printf("%3d\n",offset);//T
                continue;
            }
            case JUMP:{
                printf("jump      ");
                getInt(pc);
                int offset = int_value; 
                printf("%3d\n",offset);
                continue;       
            }
            case i_PRINT:{
                printf("i PRINT\n");
                continue;
            }
            case l_PRINT:{
                printf("l PRINT\n");
                continue;
            }
            case f_PRINT:{
                printf("f PRINT\n");
                continue;
            }
            case d_PRINT:{
                printf("d PRINT\n");
                continue;
            }
            case c_PRINT:{
                printf("c PRINT\n");
                continue;
            }
            case s_PRINT:{
                printf("s PRINT\n");
                continue;
            }
            case HALT:{
                printf("HALT\n");
                return nil;
            }
        }
    }
}
#endif