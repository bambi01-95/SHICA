#include "./setting.h"

#ifndef EXECUTER_C
#define EXECUTER_C

#include "./object.c"
#include "./tool.c"
#include "../common/inst.c"
#include "../common/liblist/library.h"
#include "../common/liblist/stdlib.h"
#include "../common/memory.c"

#include "./agent.c"

oop newBoolean(int flag) { return flag ? sys_true : sys_false; }

oop Event_userlib(int eve_num,oop stack){
    switch(eve_num){
        default:
            SHICA_FPRINTF(stderr,"this is not happen Event_userlib\n");
    }
    exit(1);
}

// EVENT...
oop setCore(int lib_num,int eve_num,oop stack);


char inst      = 0;
char char_value    = 0;
int  int_value      = 0;
long long int long_value = 0;
float float_value   = 0;
double double_value = 0;
char   string_value[256];
const unsigned int SIZE_INST   = sizeof(unsigned char);
const unsigned int SIZE_INT    = sizeof(int);            //size of int
const unsigned int SIZE_LONG   = sizeof(long long int);  //size of long long int
const unsigned int SIZE_FLOAT  = sizeof(float);          //size of float
const unsigned int SIZE_DOUBLE = sizeof(double);         //size of double

#define getData(D,PC,S) memcpy(&D,&memory[PC],S); PC+=S
#define getInst(PC)     memcpy(&inst,&memory[PC],SIZE_INST); PC+=SIZE_INST
#define getChar(PC)     memory[PC++]
#define getInt(PC)      memcpy(&int_value,&memory[PC],SIZE_INT);PC+=SIZE_INT 
#define getSetInt(VAL,PC) int VAL;memcpy(&VAL,&memory[PC],SIZE_INT);PC+=SIZE_INT
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


#include <unistd.h>//remove


//should be one execute function
// FLAG executor(oop thread,oop GM){

// }
void main_execute(){
    int pc = 0;
    int rbp = 0;
    int coreLoc = 0;    //FIXME: STTからの相対位置の取得に使用する

#if MSGC
    //CHECK ME: GMとstackの違いは？使い分けは？
    GC_PUSH(oop,GM,newThread(pc,20));
    GC_PUSH(oop,stack,newArray(20));
    gc_pushRoot((void*)&MY_AGENT_INFO);
#else
    oop stack = newArray(20);
    oop GM    = newThread(pc,20);
#endif
    for(;;){
        getInst(pc);
        switch(inst){
            case MSET:{
#if TEST  
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getInt(pc);GM->Thread.rbp = int_value;
                for(int i =0;i<int_value ;i++){
                    Array_push(GM->Thread.stack,nil);
                }
                continue;
            }
            case i_load:{
#if TEST  
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getInt(pc);
                Array_push(stack,_newInteger(int_value));
                continue;
            }
            case CALL:{
#if TEST    
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
#if MSGC
                GC_PUSH(oop,code,newThread(pc,20));
#else
                oop code = newThread(pc,20);
#endif
                Array_push(code->Thread.stack,new_Basepoint(0));
                getInt(pc);
                Array_push(code->Thread.stack,_newInteger(int_value));//number of args
                getInt(pc);
                Array_push(code->Thread.stack,_newInteger(pc));//relative location of function
                code->Thread.pc = int_value + pc;
                for(;;){
                    FLAG flag = sub_execute(code,GM);
                    if(flag == F_EOA)break;
                }
                continue;
            }

            case MKCORE:{
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getSetInt(numCore,pc);
                if(numCore == 0)continue;
                //init setting core
                coreLoc = pc;
                coreSize = -1;//CHECKME AND REMOVE ME: coreSize -1
                mainCore = mkCores(numCore);
                continue;
            }
            case COPYCORE:{
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getSetInt(grobalMemoryIndex,pc);
                getSetInt(jumpRelPos,pc);
                //get the core from the GM
                int rbp = GM->Thread.rbp;
                oop copyCore = GM->Thread.stack->Array.elements[rbp + grobalMemoryIndex];
                if(copyCore!=nil){
                    mainCore[++coreSize] = copyCore;
                    pc = jumpRelPos + pc;
                    GM->Thread.stack->Array.elements[rbp + grobalMemoryIndex] = nil;
                }
                continue;
            }
            case SETCORE:{
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getSetInt(libNum,pc);
                getSetInt(eveNum,pc);
                getSetInt(numInitVals,pc);
                oop core = setCore(libNum,eveNum,stack);
                mainCore[++coreSize] = core;
                continue;
            }
            case SETSUBCORE:{
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getSetInt(libNum,pc);
                getSetInt(eveNum,pc);
                getSetInt(numInitVals,pc);//don't use
                oop core = setCore(libNum,eveNum,stack);
                mainCore[++coreSize] = core;
                SHICA_FPRINTF(stderr,"this is not supported now\n");
                continue;
            }
            case MKTHREAD:{//FIXME: rechange the name of this instruction => SETCORE
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getSetInt(numThread,pc);
                mainCore[coreSize]->Core.threads = newThreads(coreLoc,20,numThread);
                continue;
            }

            case SETTHREAD:{
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getSetInt(aRelPos,pc);
                getSetInt(cRelPos,pc);

                oop thread = newThread(coreLoc + aRelPos,20);
                
                thread->Thread.condRelPos = cRelPos;
                mainCore[coreSize]->Core.threads[mainCore[coreSize]->Core.size++] = thread;
                continue;
            }
            case STARTIMP:{
#if TEST
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                for(int isTrans=0;isTrans==0;){   //isActive: 1:not stt transision, 0->inac
                    for(int core_i=0;core_i<=coreSize;core_i++){
                    //<イベントの確認>/<check event>
                        mainCore[core_i]->Core.func(mainCore[core_i]);
                    //<イベントアクションの実行>/<implement event action>
                        for(int thread_i=0;thread_i<mainCore[core_i]->Core.size;thread_i++){
                            oop thread = mainCore[core_i]->Core.threads[thread_i];
                            
                            if(thread->Thread.flag == 1){
                                //implement function of event
                                FLAG flag = sub_execute(thread,GM);
                                switch(flag){
                                    case F_TRANS:{
                                        if(evalEventArgsThread->Thread.stack->Array.capacity>0){
                                            gc_unmarkOnly((void*)evalEventArgsThread->Thread.stack);
                                            evalEventArgsThread->Thread.stack = newArray(10);
                                        }


                                        int pc_i = thread->Thread.pc++;//location of thread[i]'s pc
                                        getSetInt(relpos,pc_i);
                                        getSetInt(numCopyCore,pc);
                                        for(int i=0;i<numCopyCore;i++){
                                            oop coreIndex = Array_get(GM->Thread.stack,GM->Thread.rbp + i);
                                            if(_Integer_value(coreIndex)==-1){
                                                Array_put(GM->Thread.stack,GM->Thread.rbp + i,nil);
                                            }else{
                                                Array_put(GM->Thread.stack,GM->Thread.rbp + i,mainCore[_Integer_value(coreIndex)]);
                                            }
                                        }
                                        pc = relpos + pc_i;
                                    #if DEBUG
                                        SHICA_PRINTF("Trans to %d\n",pc);
                                    #endif
                                        isTrans = 1;
                                        int gm_size = GM->Thread.stack->Array.size;
                                    //CHECK ME: ここでGMのスタックをクリアするか？
                                        for(int i = gm_size;i>GM->Thread.rbp;i--){
                                            Array_pop(GM->Thread.stack);
                                        }
                                        break;
                                    }
                                    case F_EOE:{
                                        thread->Thread.flag = 0;
                                        thread->Thread.pc = thread->Thread.base;
                                        break;
                                    }
                                    case F_EOA:
                                    case F_NONE:{
                                        break;
                                    }
                                    default:{
                                    #if DEBUG
                                        DEBUG_ERROR("this is not happen, main_execute case STARTIMP\n");
                                        exit(1);
                                    #else
                                        SHICA_PRINTF("this is not happen, main_execute case STARTIMP\n");
                                        exit(1);
                                    #endif
                                    }
                                }
                            }else if(thread->Thread.queue->Queue.size>0){
                                thread->Thread.flag = 1;
                                oop variables =  dequeue(thread->Thread.queue);
                                Array_args_copy(variables,thread->Thread.stack);
                            }
                        }
                    }
                }
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
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
#if MSGC
                GC_PUSH(oop,code,newThread(0,10));//FIXME: using new thread here is not good for ...
#else
                oop code = newThread(0,10);
#endif
                code->Thread.pc = pc;
                for(;;){
                    FLAG flag = sub_execute(code,GM);
                    if(flag == F_EOE)break;
                }
                pc = code->Thread.pc;
#if MSGC
                GC_POP(code);
#else
                // free(code);//IFERROR
#endif
                continue;
            }

            case ENTRY:{
#if TEST  
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getInt(pc);
                int s_pc = pc;//store corrent pc
#if MSGC
                GC_PUSH(oop, code,newThread(0,20));//FIXME: this is  not good for memory
#else   
                oop code = newThread(0,20);
#endif
                code->Thread.pc = pc + int_value;
                Array_push(code->Thread.stack,new_Basepoint(0));
                
                for(int isStop=0;isStop!=1;){
                    FLAG flag = sub_execute(code,GM);
                    switch(flag){
                        case F_EOE:isStop = 1;break;
                        case F_TRANS:{
                            if(evalEventArgsThread->Thread.stack->Array.capacity>0){
                                gc_unmarkOnly((void*)evalEventArgsThread->Thread.stack);
                                evalEventArgsThread->Thread.stack = newArray(10);
                            }
                            int pc_i = code->Thread.pc++;//location of thread[i]'s pc
                            getSetInt(pos,pc_i);
                            s_pc = pos + pc_i;
                        #if DEBUG
                            SHICA_PRINTF("Trans to %d\n",s_pc);
                        #endif
                            int gm_size = GM->Thread.stack->Array.size;
                        //CHECK ME: ここでGMのスタックをクリアするか？
                            for(int i = gm_size;i>GM->Thread.rbp;i--){
                                Array_pop(GM->Thread.stack);
                            }
                            isStop = 1;
                            break;
                        }
                    }
                }
#if MSGC
                GC_POP(code);
#else
                // free(code);
#endif
                pc = s_pc;
                continue;                
            }



            case JUMP:{
#if TEST  
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                getInt(pc); pc += int_value;
                continue;       
            }
            case GET_G:{
                getInt(pc);
                Array_push(stack,Array_get(GM->Thread.stack,int_value));
                continue;
            }
            case HALT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: main pc    [%03d] %s\n",__LINE__,pc,INSTNAME[inst]);}
#endif
                return;
            }
            default:
            {
#if DEBUG
            SHICA_FPRINTF(stderr,"main_execute error %s\n",INSTNAME[inst]);
#else
            SHICA_FPRINTF(stderr,"main_execute error %d\n",inst);
#endif
            }
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
#define aps()     Array_pop(mstack)->_String.value
#define apc()    _Char_value(Array_pop(mstack))

#include "./library/lib.c"

oop setCore(int lib_num,int eve_num,oop stack){
    switch(lib_num){
        case STDLIB:{
            return Event_stdlib(eve_num,stack);
        }
        case COMMUNICATELIB:{
            return Event_communicate(eve_num,stack);
        }
        case USERLIB:{
            return 0;
        }
#if RPI
        case GPIOLIB:{
            return Event_gpiolib(eve_num,stack);
        }
#endif
        default:{
            SHICA_PRINTF("ERROR: Event lib[%d] eve[%d]\n",lib_num, eve_num);
            exit(1);
        }
    }
    SHICA_PRINTF("Event_Func(): not happen..\n");
    return 0;
}



////////////////////////////////////////////////




FLAG sub_execute(oop process,oop GM){
    for(;;){
        getInst(mpc);
        switch(inst){
            case TRANS:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                return F_TRANS;
            }
            case i_load:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
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
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l == r));
                continue;
            }
            case i_NE:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l != r));
                continue;
            }
            case i_LT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api();
                int l = api();
                Array_push(mstack,newBoolean(l <  r));
                continue;
            }
            case i_LE:{//inprogress
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l <= r));
                continue;
            }
            case i_GE:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l >= r));
                continue;
            }
            case i_GT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l >  r));
                continue;
            }
            case i_AND:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l && r));
                continue;
            }
            case i_OR:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,newBoolean(l || r));
                continue;
            }
            case i_ADD:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l + r));
                continue;
            }
            case i_SUB:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l - r));
                continue;
            }
            case i_MUL:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l * r));
                continue;
            }
            case i_DIV:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l / r));
                continue;
            }
            case i_MOD:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l % r));
                continue;
            }
            case i_BAND:{
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l & r));
                continue;
            }
            case i_BOR:{
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l | r));
                continue;
            }
            case i_LSH:{
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l << r));
                continue;
            }
            case i_RSH:{
                int r = api(),l = api();
                Array_push(mstack,_newInteger(l >> r));
                continue;
            }
/* Long */
            case l_EQ:{
                long long int r = apl(),l = apl();
                if(l==r)Array_push(mstack,sys_true);
                else    Array_push(mstack,sys_false);
                continue;
            }
            case l_NE:{
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
                oop newStr = newString(strcat(strdup(l->_String.value),r->_String.value));
                Array_push(mstack,newStr);
                continue;
            }
/* end calc */
            case CALL:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                Array_push(mstack,_newInteger(int_value));//number of args
                getInt(mpc);
                Array_push(mstack,_newInteger(mpc));//relative location of function
                mpc = mpc + int_value;
                return F_NONE;
            }
            case CALL_P:{
#if TEST
SHICA_PRINTF("CALL_P\n");
#endif
                Call_Primitive(process,GM);
                return F_NONE;
            }
            case GET:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                oop data = Array_get(mstack,mrbp + int_value);
                Array_push(mstack,data);//index
                continue;
            }
            case GET_L:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                // SHICA_PRINTF("    %s\n",TYPENAME[getType(mstack)]);
                Array_push(mstack,Array_get(GM->Thread.stack,GM->Thread.rbp + int_value));//index
                continue;
            }
            case GET_G:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                Array_push(mstack,Array_get(GM->Thread.stack,int_value));//index
                continue;
            }
            case DEFINE:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                oop data  = Array_pop(mstack);
                Array_put(mstack,mrbp + int_value, data);
                continue;
            }
            case DEFINE_L:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                oop data  = Array_pop(mstack);
                Array_put(GM->Thread.stack,GM->Thread.rbp + int_value, data);
                continue;
            }
            case DEFINE_G:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);
                oop data = Array_pop(mstack);
                Array_put(GM->Thread.stack,int_value,data);
                continue;
            }
            case DEFINE_List:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
#if DEBUG
                DEBUG_LOG("this is not support now");
#else
                SHICA_PRINTF("this is not support now sub_execute DEFINE_List\n");
#endif
                // getInt(mpc);
                // int index = _Integer_value(Array_pop(mstack));
                // oop data  = Array_pop(mstack);
                // //FIXME: サイズの確認とかやってない
                // GM->Thread.stack->Array.elements[int_value]->_IntegerArray.array[index] = data;
                // continue;
            }
            case GLOBAL_END:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                return F_EOE;
            }
            case SETQ:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                continue;
            }
            case RET:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                oop value = Array_pop(mstack);//return value
                oop data = nil;
                while(getType(data = Array_pop(mstack)) != _BasePoint);
                mrbp = getChild(data,_BasePoint,adress);
                mpc = api();//next mpc 
                int num_arg = api();
                for(int i = 0;i<num_arg;i++)
                    Array_pop(mstack);
                Array_push(mstack,value);
                return F_NONE;
            }
            case JUMPF:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
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
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                getInt(mpc);     //get offset
                mpc += int_value;//get offset
                return F_NONE;//go next thread
            }
            case MSUB:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
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
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
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
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
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
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                SHICA_PRINTF("%d\n",_Integer_value(Array_pop(mstack)));
                continue;
            }
            case l_PRINT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                SHICA_PRINTF("%lld\n",Array_pop(mstack)->_Long.value);
                continue;
            }
            case f_PRINT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                SHICA_PRINTF("%f",_Float_value(Array_pop(mstack)));
                continue;
            }
            case d_PRINT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                SHICA_PRINTF("%f",Array_pop(mstack)->_Double.value);
                continue;
            }
            case c_PRINT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                SHICA_PRINTF("%c",_Char_value(Array_pop(mstack)));
                continue;
            }
            case s_PRINT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                SHICA_PRINTF("%s\n\n",Array_pop(mstack)->_String.value);
                continue;
            }
            case EOE:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                // mpc  = mrbp;
                mpc -= 1;
#if DEBUG
                DEBUG_ERROR_COND(mstack->Array.size == 0,"IMPRIMENT ERROR: stack size is %d\n",mstack->Array.size);
#endif

                return F_EOE;
            }
            case EOC:{
#if TEST
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                return F_TRUE;
            }
            case EOA:{
#if TEST
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                oop data = nil;
                while(getType(data = Array_pop(mstack)) != _BasePoint);
                mrbp = getChild(data,_BasePoint,adress);
                mpc = api();//next mpc 
                return F_EOA;
            }
            case COND:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                oop cond = Array_pop(mstack);
                //FIXME: usign sys_false and sys_true
                if(cond == sys_false)return F_FALSE;//offset
                else if(cond == sys_true)continue;
                else if(_Integer_value(cond)==0)return F_FALSE;//offset
                continue;
            }
            case HALT:{
#if TEST  
if(1){SHICA_PRINTF("line %d: sub    [%03d] %s\n",__LINE__,mpc,INSTNAME[inst]);}
#endif
                if(getChild(mstack,Array,size)==1){
                    SHICA_PRINTF("HALT-----------------------\n");
                    printlnObject(Array_pop(mstack),1);
                    return F_NONE;
                }
                SHICA_PRINTF("HALT with %d items on mstack\n",getChild(mstack,Array,size));
                int size = getChild(mstack,Array,size);
                for(int i = 0; i<size;i++){
                    SHICA_PRINTF("%d: ",i);
                    printlnObject(Array_pop(mstack),1);
                    
                }
                exit(1);
                return F_NONE;
            }
            default:
            {
#if DEBUG
            SHICA_FPRINTF(stderr,"  sub_execute error %s\n",INSTNAME[inst]);
#else
            SHICA_FPRINTF(stderr,"  sub_execute error %d\n",inst);
#endif
            }
        }
        return F_NONE;
    } 
}




oop printByteCode(){
    int pc = 0;
    for(;;){
        SHICA_PRINTF("%3d ",pc);
        getInst(pc);
        switch(inst){
            case TRANS:{
                SHICA_PRINTF("COPYCORE ");//T
                getInt(pc);int nextStateRelPos = int_value;
                int nextPC = pc + nextStateRelPos;
                getInt(pc);int numOfEvent = int_value;
                SHICA_PRINTF("%3d  %3d (to %d)\n",nextStateRelPos,numOfEvent,nextPC);
                continue;
            }
            case i_load: getInt(pc);SHICA_PRINTF("i_load    %3d\n",int_value);continue;
            case l_load: getLong(pc);SHICA_PRINTF("l_load    %3lld\n",long_value);continue;
            case f_load: getFloat(pc);SHICA_PRINTF("f_load    %3f\n",float_value);continue;
            case d_load: getDouble(pc);SHICA_PRINTF("d_load    %3f\n",double_value);continue;
            case c_load: SHICA_PRINTF("c_load    %3c\n",getChar(pc));continue;
            case s_load: getString(pc);SHICA_PRINTF("s_load    %s\n",string_value);continue;
            case il_load: getInt(pc);SHICA_PRINTF("il_load    %d\n",int_value);continue;
            case i_EQ:   SHICA_PRINTF("i_EQ\n"); continue; 
            case i_NE:   SHICA_PRINTF("i_NE\n"); continue; 
            case i_LT:   SHICA_PRINTF("i_LT\n"); continue; 
            case i_LE:   SHICA_PRINTF("i_LE\n"); continue; 
            case i_GE:   SHICA_PRINTF("i_GE\n"); continue; 
            case i_GT:   SHICA_PRINTF("i_GT\n"); continue; 
            case i_ADD:  SHICA_PRINTF("i_ADD\n");continue;
            case i_SUB:  SHICA_PRINTF("i_SUB\n");continue;
            case i_MUL:  SHICA_PRINTF("i_MUL\n");continue;
            case i_BAND: SHICA_PRINTF("i_BAND\n");continue;
            case i_BOR:  SHICA_PRINTF("i_BOR\n");continue;
            case i_LSH:  SHICA_PRINTF("i_LSH\n");continue;
            case i_RSH:  SHICA_PRINTF("i_RSH\n");continue;
            case i_DIV:  SHICA_PRINTF("i_DIV\n");continue;
            case i_MOD:  SHICA_PRINTF("i_MOD\n");continue;
            case l_EQ:   SHICA_PRINTF("l_EQ\n");continue; 
            case l_NE:   SHICA_PRINTF("l_NE\n");continue; 
            case l_LT:   SHICA_PRINTF("l_LT\n");continue; 
            case l_LE:   SHICA_PRINTF("l_LE\n");continue; 
            case l_GE:   SHICA_PRINTF("l_GE\n");continue; 
            case l_GT:   SHICA_PRINTF("l_GT\n");continue; 
            case l_ADD:  SHICA_PRINTF("l_ADD\n");continue;
            case l_SUB:  SHICA_PRINTF("l_SUB\n");continue;
            case l_MUL:  SHICA_PRINTF("l_MUL\n");continue;
            case l_DIV:  SHICA_PRINTF("l_DIV\n");continue;
            case l_MOD:  SHICA_PRINTF("l_MOD\n");continue;
            case f_EQ:   SHICA_PRINTF("f_EQ\n");continue; 
            case f_NE:   SHICA_PRINTF("f_NE\n");continue; 
            case f_LT:   SHICA_PRINTF("f_LT\n");continue; 
            case f_LE:   SHICA_PRINTF("f_LE\n");continue; 
            case f_GE:   SHICA_PRINTF("f_GE\n");continue; 
            case f_GT:   SHICA_PRINTF("f_GT\n");continue; 
            case f_ADD:  SHICA_PRINTF("f_ADD\n");continue;
            case f_SUB:  SHICA_PRINTF("f_SUB\n");continue;
            case f_MUL:  SHICA_PRINTF("f_MUL\n");continue;
            case f_DIV:  SHICA_PRINTF("f_DIV\n");continue;
            case d_EQ:   SHICA_PRINTF("d_EQ\n");continue; 
            case d_NE:   SHICA_PRINTF("d_NE\n");continue; 
            case d_LT:   SHICA_PRINTF("d_LT\n");continue; 
            case d_LE:   SHICA_PRINTF("d_LE\n");continue; 
            case d_GE:   SHICA_PRINTF("d_GE\n");continue; 
            case d_GT:   SHICA_PRINTF("d_GT\n");continue; 
            case d_ADD:  SHICA_PRINTF("d_ADD\n");continue;
            case d_SUB:  SHICA_PRINTF("d_SUB\n");continue;
            case d_MUL:  SHICA_PRINTF("d_MUL\n");continue;
            case d_DIV:  SHICA_PRINTF("d_DIV\n");continue;

            case s_EQ:   SHICA_PRINTF("d_EQ\n");continue; 
            case s_NE:   SHICA_PRINTF("d_NE\n");continue; 
            case s_LT:   SHICA_PRINTF("d_LT\n");continue; 
            case s_LE:   SHICA_PRINTF("d_LE\n");continue; 
            case s_GE:   SHICA_PRINTF("d_GE\n");continue; 
            case s_GT:   SHICA_PRINTF("d_GT\n");continue; 
            case s_ADD:  SHICA_PRINTF("d_ADD\n");continue;

            case MKCORE:    getInt(pc);SHICA_PRINTF("MKCORE    %3d\n",int_value);continue;
            case COPYCORE:{
                SHICA_PRINTF("COPYCORE ");//T
                getInt(pc);int indexOfGlobalMemory = int_value;
                getInt(pc);int jumpRelPos = int_value;
                SHICA_PRINTF("%3d  %3d (to %d)\n",indexOfGlobalMemory,jumpRelPos,pc+jumpRelPos);
                continue;
            }
            case SETCORE:{
                SHICA_PRINTF("SETCORE    ");
                getInt(pc);int libNum = int_value;
                getInt(pc);int eveNum = int_value;
                getInt(pc);int numInitVals = int_value;
                SHICA_PRINTF("%3d  %3d  %3d\n",libNum,eveNum,numInitVals);
                continue;
            }         
            case SETSUBCORE:{
                SHICA_PRINTF("SETSUBCORE    ");
                getInt(pc);int libNum = int_value;
                getInt(pc);int eveNum = int_value;
                getInt(pc);int numInitVals = int_value;
                SHICA_PRINTF("%3d  %3d  %3d\n",libNum,eveNum,numInitVals);
                continue;
            }     
            case MKTHREAD:{
                SHICA_PRINTF("MKTHREAD    ");
                getInt(pc);int threadNum = int_value;
                SHICA_PRINTF("%3d\n",threadNum);
                continue;
            }
            case SETTHREAD:{
                SHICA_PRINTF("SETTHREAD    ");
                getInt(pc);int threadNum = int_value;
                getInt(pc);int relPos = int_value;
                SHICA_PRINTF("%3d  %3d\n",threadNum,relPos);
                continue;
            }
            case STARTIMP:  SHICA_PRINTF("STARTIMP\n");continue;

            case EOE:    SHICA_PRINTF("EOE\n");continue;
            case EOC:    SHICA_PRINTF("EOC\n");continue;
            case EOA:    SHICA_PRINTF("EOA\n");continue;
            case COND:   SHICA_PRINTF("COND\n");continue;

            case CALL:{
                SHICA_PRINTF("CALL      ");
                getInt(pc);int num_arg = int_value;
                getInt(pc);int index = int_value;
                SHICA_PRINTF("%3d  %3d (to %3d)\n",num_arg,index,pc + index);
                continue;
            }
            case CALL_P:{
                SHICA_PRINTF("CALL_P      ");
                getInt(pc);int lib_num = int_value;
                getInt(pc);int func_num = int_value;
                getInt(pc);int args_s = int_value;
                SHICA_PRINTF("%3d  %3d  %3d\n",lib_num,func_num,args_s);
                continue;
            }
            case GET:{
                SHICA_PRINTF("Get       ");//T
                getInt(pc);int symbol = int_value;
                SHICA_PRINTF("%3d\n",symbol);
                continue;
            }
            case GET_L:{
                SHICA_PRINTF("Get_L     ");//T
                getInt(pc);int symbol = int_value;
                SHICA_PRINTF("%3d\n",symbol);
                continue;
            }
            case GET_G:{
                SHICA_PRINTF("GET_G     ");
                getInt(pc);int symbol = int_value;
                SHICA_PRINTF("%3d\n",symbol);
                continue;
            }
            case DEFINE:{
                SHICA_PRINTF("DEFINE    ");//T
                getInt(pc);int symbol = int_value;
                SHICA_PRINTF("%3d\n",symbol);//T
                continue;
            }
            case DEFINE_L:{
                SHICA_PRINTF("DEFINE_L  ");//T
                getInt(pc);int symbol = int_value;
                SHICA_PRINTF("%3d\n",symbol);//T
                continue;
            }
            case DEFINE_G:{
                SHICA_PRINTF("DEFINE_List  ");//T
                getInt(pc);int symbol = int_value;
                SHICA_PRINTF("%3d\n",symbol);//T
                continue;
            }  
            case GLOBAL:{
                SHICA_PRINTF("GLOBAL\n");
                continue;
            }
            case GLOBAL_END:{
                SHICA_PRINTF("GLOBAL_END\n");
                continue;
            }
            case ENTRY:{
                SHICA_PRINTF("ENTRY     ");
                getInt(pc);int index = int_value;
                SHICA_PRINTF("%3d (to %3d)\n",index,pc + index);//T
                continue;
            }
            case SETQ:{
                SHICA_PRINTF("SETQ\n");
                continue;
            }
            case RET:{
                SHICA_PRINTF("RET       ");
                SHICA_PRINTF("  X\n");
                continue;
            }
            case MSUB:{
                SHICA_PRINTF("MSUB      ");
                getInt(pc);SHICA_PRINTF("%3d\n",int_value);
                continue;
            }
            case MPOP:{
                SHICA_PRINTF("MPOP\n");
                continue;
            }
            case MPICK:{
                SHICA_PRINTF("MPICK     ");
                getInt(pc);int i = int_value;
                getInt(pc);int j = int_value;
                SHICA_PRINTF("%3d %3d\n",i,j);
                continue;
            }
            case MSET:{
                SHICA_PRINTF("MSET      ");
                getInt(pc);
                SHICA_PRINTF("%3d\n",int_value);
                continue;
            }
            case JUMPF:{
                SHICA_PRINTF("jumpF     ");//T
                getInt(pc);
                int offset = int_value; 
                SHICA_PRINTF("%3d\n",offset);//T
                continue;
            }
            case JUMP:{
                SHICA_PRINTF("jump      ");
                getInt(pc);
                int offset = int_value; 
                SHICA_PRINTF("%3d (to %3d)\n",offset,pc + offset);
                continue;       
            }
            case i_PRINT:{
                SHICA_PRINTF("i PRINT\n");
                continue;
            }
            case l_PRINT:{
                SHICA_PRINTF("l PRINT\n");
                continue;
            }
            case f_PRINT:{
                SHICA_PRINTF("f PRINT\n");
                continue;
            }
            case d_PRINT:{
                SHICA_PRINTF("d PRINT\n");
                continue;
            }
            case c_PRINT:{
                SHICA_PRINTF("c PRINT\n");
                continue;
            }
            case s_PRINT:{
                SHICA_PRINTF("s PRINT\n");
                continue;
            }
            case HALT:{
                SHICA_PRINTF("HALT\n");
                return nil;
            }
            default:{
#if DEBUG
                SHICA_PRINTF("  printByteCode error %s\n",INSTNAME[inst]);
#else
                SHICA_PRINTF("%s line %d this is not happen, inst [%d]\n",__FILE__,__LINE__,inst);
#endif
            }
        }
    }
}
#endif