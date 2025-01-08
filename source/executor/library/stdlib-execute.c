#include "../setting.h"


#ifndef STDLIB_EXECUTE_C
#define STDLIB_EXECUTE_C
#include "../object.c"
#include "../../common/liblist/stdlib.h"

/*
    threads: multi function thread space
    argsCond: それぞれのconditionの内容が入っている
*/
#if SBC //event()
oop eve_test(oop core){
    time_t current_time = time(NULL);
    if(current_time - core->Core.vd->VarTI.v_t1 >= core->Core.vd->VarTI.v_i2){
        core->Core.vd->VarTI.v_t1 = current_time;
        core->Core.vd->VarTI.v_i1  += core->Core.vd->VarTI.v_i2;

        int isOnce = 0;
        evalEventArgsThread->Thread.stack->Array.size = 1;//1:basepoint
        for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
            int isFalse = 0;
            oop thread = core->Core.threads[thread_i];
            //<引数の評価>/<Evaluation of arguments>
            if(thread->Thread.condRelPos != 0){
                if(isOnce == 0){
                    Array_push(evalEventArgsThread->Thread.stack,new_Basepoint(0));
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(core->Core.vd->VarTI.v_i1));
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(core->Core.vd->VarTI.v_i1));
                    isOnce = 1;
                }else
                {
                    evalEventArgsThread->Thread.stack->Array.size = 3;
                }
                evalEventArgsThread->Thread.pc = thread->Thread.base + thread->Thread.condRelPos;
                for(;;){
                    FLAG flag = sub_execute(evalEventArgsThread,nil);
                    if(flag == F_TRUE){
                        break;
                    }
                    else if(flag == F_FALSE){
                        isFalse = 1;
                        break;
                    }
                }
            }
            
            //<条件が満たされたときの処理>/<Processing when the condition is met>
            if(!isFalse){
                //protect t:thread
                gc_pushRoot((void*)&core);//CHECKME: is it need?
                oop data = newArray(2);
                Array_push(data,_newInteger(core->Core.vd->VarTI.v_i1));
                Array_push(data,_newInteger(core->Core.vd->VarTI.v_i1));
                gc_popRoots(1);
                enqueue(thread->Thread.queue,data);
            }
        }
                // FIXME: Array-free or somthing need
    }
    return core;
}
#else
oop eve_test(oop core){
    SHICA_PRINTF("eve_test\n");
    return core;
}
#endif



oop eve_loop(oop core){
    for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
        oop thread = core->Core.threads[thread_i];
        if(thread->Thread.flag == 0){
#if MSGC
            gc_pushRoot((void*)&core);
            oop data = newArray(2);
            Array_push(data,_newInteger(1));
            gc_popRoots(1);
            enqueue(thread->Thread.queue,data);
#else
            oop data = newArray(2);
            Array_push(data,_newInteger(1));
            enqueue(thread->Thread.queue,data);
#endif
        }
    }
    return core;
}


#if SBC
oop eve_timer(oop core){
    time_t current_time = time(NULL);
    if(current_time - core->Core.vd->VarTI.v_i1 >= core->Core.vd->VarTI.v_i2){
        core->Core.vd->VarTI.v_t1 = current_time;
        core->Core.vd->VarTI.v_i1  += core->Core.vd->VarTI.v_i2;

        int isOnce = 0;
        evalEventArgsThread->Thread.stack->Array.size = 1;//1:basepoint
        for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
            int isFalse = 0;
            oop t = core->Core.threads[thread_i];
            if(t->Thread.condRelPos != 0){
                if(isOnce ==0){
                    Array_push(evalEventArgsThread->Thread.stack,new_Basepoint(0));
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(core->Core.vd->VarTI.v_i1));
                    isOnce = 1;
                }else{
                    evalEventArgsThread->Thread.stack->Array.size = 3;
                }
                evalEventArgsThread->Thread.pc = t->Thread.base + t->Thread.condRelPos;
                for(;;){
                    FLAG flag = sub_execute(evalEventArgsThread,nil);
                    if(flag == F_TRUE)break;
                    else if(flag == F_FALSE){
                        isFalse = 1;
                        break;
                    }
                }
            }
            
            if(!isFalse){
                //protect t:thread
                gc_pushRoot((void*)&t);
                oop data = newArray(2);
                Array_push(data,_newInteger(core->Core.vd->VarTI.v_i1));
                gc_popRoots(1);
                enqueue(t->Thread.queue,data);
            }
        }
    }
    return core;
}
#else
oop eve_timer(oop core){
    SHICA_PRINTF("eve_timer\n");
    return core;
}
#endif



#if SBC
#include <termios.h> //keyboard input
oop eve_keyget(oop core){
    char buf;
    struct termios old_flags, new_flags;
    fd_set fds;
    struct timeval tv;
    int ret;

    // 標準入力をノンブロッキングモードに設定
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // 現在の端末属性を取得
    tcgetattr(STDIN_FILENO, &old_flags);
    new_flags = old_flags;
    // ICANON(カノニカルモード)とECHO(エコー)を無効にする
    new_flags.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_flags);

    // 標準入力からの読み取りを待たない
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    // タイムアウトを設定（0秒）
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    // 標準入力からの読み取りをチェック
    ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret < 0) {
        perror("select");
        exit(EXIT_FAILURE);
    } else if (ret == 0) {
        // 標準入力からの読み取りが利用可能でない場合
        return core;
    } else {
        // 標準入力からの読み取りが利用可能な場合、1バイト読み取る
        if (read(STDIN_FILENO, &buf, 1) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 読み取りが非同期にブロックされている場合
                return core;
            } else {
                perror("read");
                exit(EXIT_FAILURE);
            }
        }
        // 読み取った文字を返す

        int isOnce = 0;
        evalEventArgsThread->Thread.stack->Array.size = 1;//1:basepoint
        for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
            oop t = core->Core.threads[thread_i];
            int isFalse = 0;
            if(!isOnce){
                Array_push(evalEventArgsThread->Thread.stack,new_Basepoint(0));
                Array_push(evalEventArgsThread->Thread.stack,_newInteger(buf));
                isOnce = 1;
            }else{
                evalEventArgsThread->Thread.stack->Array.size = 2;
            }

            if(t->Thread.condRelPos != 0){
                evalEventArgsThread->Thread.pc = t->Thread.base + t->Thread.condRelPos;
                for(;;){
                    FLAG flag = sub_execute(evalEventArgsThread,nil);
                    if(flag == F_TRUE)break;
                    else if(flag == F_FALSE){
                        isFalse = 1;
                        break;
                    }
                }
            }
            
            if(!isFalse){
                //protect t:thread
                gc_pushRoot((void*)&core);
                oop data = newArray(2);
                Array_push(data,_newInteger(buf));
                gc_popRoots(1);
                enqueue(t->Thread.queue,data);
            }
        }
        return core;
    }
}
#else
oop eve_keyget(oop core){
    SHICA_PRINTF("eve_keyget\n");
    return t;
}
#endif



//STDLIB EVENT
//FOR SETTING
oop Event_stdlib(int eve_num,oop stack,int numThread){
    //cheack: protect stack, but it is already protected
    gc_pushRoot((void*)&stack);
    //protect t:new thread
    GC_PUSH(oop,core,0);
    switch(eve_num){
        case TEST_E:{
#if SBC     
            core = newCore(VarTI,numThread);
            core->Core.vd->VarTI.v_t1 = time(NULL);
            core->Core.vd->VarTI.v_i1  = 0;
            core->Core.vd->VarTI.v_i2  = 1;
            core->Core.func = &eve_test;
#else
            core = newCore(Default,numThread);
            core->Core.vd->Default.count = 0;
            core->Core.func = &eve_test;
#endif
            break;
        }
        case LOOP_E:{
            core = newCore(Default,numThread);
            core->Core.vd->Default.count = 0;
            core->Core.func = &eve_loop;
            break;
        }
        case TIMERSEC_E:{
#if SBC     
            core = newCore(VarTI,numThread);
            core->Core.vd->VarTI.v_t1 = time(NULL);
            core->Core.vd->VarTI.v_i1  = 0;
            core->Core.vd->VarTI.v_i2  = _Integer_value(Array_pop(stack));
            core->Core.func = &eve_timer;
#else
            core = newCore(Default,numThread);
            core->Core.vd->Default.count = 0;
            core->Core.func = &eve_timer;
#endif
            break;
        }
        case KEYGET_E:{
            core = newCore(Default,numThread);
            core->Core.func = &eve_keyget;
            //clear key buffer
            while(0!=core->Core.func(core))dequeue(core);
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_stdlib eve[%d]\n",eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return core;
}

#endif