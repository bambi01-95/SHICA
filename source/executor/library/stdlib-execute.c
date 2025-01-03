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
oop eve_test(oop t){
    time_t current_time = time(NULL);
    if(current_time - t->Thread.vd->VarTI.v_t1 >= t->Thread.vd->VarTI.v_i2){
        t->Thread.vd->VarTI.v_t1 = current_time;
        t->Thread.vd->VarTI.v_i1  += t->Thread.vd->VarTI.v_i2;
        //eval cond
#if MSGC
                GC_PUSH(oop, code,newThread(Default,10,0));//FIXME: this is  not good for memory
#else   
                oop code = newThread(Default,20,0);
#endif//Add instruction of JUDGE, if it is ture, FLAG some, else some....
                int isFalse = 0;
                for(int i=0;i<2;i++){
                    if(t->Thread.loc_cond[i] == 0)continue;
                    code->Thread.pc = t->Thread.base + t->Thread.loc_cond[i];
                    Array_push(code->Thread.stack,new_Basepoint(0));
                    Array_push(code->Thread.stack,_newInteger(t->Thread.vd->VarTI.v_i1));
                    for(;;){
                        FLAG flag = sub_execute(code,nil);
                        if(flag == F_TRUE){
                            // SHICA_PRINTF("TRUE\n");
                            break;
                        }
                        else if(flag == F_FALSE){
                            // SHICA_PRINTF("FALSE\n");
                            isFalse = 1;
                            break;
                        }
                    }
                }
                // FIXME: Array-free or somthing need
#if MSGC
                GC_POP(code);
#else
                // free(code);
#endif  
        if(!isFalse){
            //protect t:thread
            gc_pushRoot((void*)&t);
            oop data = newArray(2);
            Array_push(data,_newInteger(t->Thread.vd->VarTI.v_i1));
            Array_push(data,_newInteger(t->Thread.vd->VarTI.v_i1));
            gc_popRoots(1);
            enqueue(t->Thread.queue,data);
        }
    }
    return t;
}
#else
oop eve_test(oop t){
    SHICA_PRINTF("eve_test\n");
    return t;
}
#endif



oop eve_loop(oop t){
    if(t->Thread.flag == 0){
        //protect t:thread
        gc_pushRoot((void*)&t);
        oop data = newArray(2);
        Array_push(data,_newInteger(1));
        gc_popRoots(1);
        enqueue(t->Thread.queue,data);
    }
    return t;
}

#if SBC
oop eve_timer(oop t){
    time_t current_time = time(NULL);
    if(current_time - t->Thread.vd->VarTI.v_t1 >= t->Thread.vd->VarTI.v_i2){
        t->Thread.vd->VarTI.v_t1 = current_time;
        t->Thread.vd->VarTI.v_i1  += t->Thread.vd->VarTI.v_i2;
        //eval cond
#if MSGC
                GC_PUSH(oop, code,newThread(Default,10,0));//FIXME: this is  not good for memory
#else   
                oop code = newThread(Default,20,0);
#endif//Add instruction of JUDGE, if it is ture, FLAG some, else some....
                int isFalse = 0;
                code->Thread.pc = t->Thread.base + t->Thread.loc_cond[0];
                Array_push(code->Thread.stack,new_Basepoint(0));
                Array_push(code->Thread.stack,_newInteger(t->Thread.vd->VarTI.v_i1));
                for(;;){
                    FLAG flag = sub_execute(code,nil);
                    if(flag == F_TRUE)break;
                    else if(flag == F_FALSE){
                        isFalse = 1;
                        break;
                    }
                }
                // FIXME: Array-free or somthing need
#if MSGC
                GC_POP(code);
#else
                // free(code);
#endif  
        if(!isFalse){
            //protect t:thread
            gc_pushRoot((void*)&t);
            oop data = newArray(2);
            Array_push(data,_newInteger(t->Thread.vd->VarTI.v_i1));
            gc_popRoots(1);
            enqueue(t->Thread.queue,data);
        }
    }
    return t;
}
#else
oop eve_timer(oop t){
    SHICA_PRINTF("eve_timer\n");
    return t;
}
#endif

#if SBC
#include <termios.h> //keyboard input
oop eve_keyget(oop t){
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
        return 0;
    } else {
        // 標準入力からの読み取りが利用可能な場合、1バイト読み取る
        if (read(STDIN_FILENO, &buf, 1) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 読み取りが非同期にブロックされている場合
                return 0;
            } else {
                perror("read");
                exit(EXIT_FAILURE);
            }
        }
        // 読み取った文字を返す
        //protect t:thread
        gc_pushRoot((void*)&t);
        oop data = newArray(2);
        Array_push(data,_newInteger(buf));
        gc_popRoots(1);
        enqueue(t->Thread.queue,data);
        return t;
    }
}
#else
oop eve_keyget(oop t){
    SHICA_PRINTF("eve_keyget\n");
    return t;
}
#endif



//STDLIB EVENT
//FOR SETTING
oop Event_stdlib(int eve_num,oop stack,int stk_size){
    //cheack: protect stack, but it is already protected
    gc_pushRoot((void*)&stack);
    //protect t:new thread
    GC_PUSH(oop,t,0);
    switch(eve_num){
        case TEST_E:{
#if SBC     
            t = newThread(VarTI,stk_size,1);
            t->Thread.vd->VarTI.v_t1 = time(NULL);
            t->Thread.vd->VarTI.v_i1  = 0;
            t->Thread.vd->VarTI.v_i2  = 1;
            t->Thread.func = &eve_test;
            t->Thread.loc_cond[0] =  _Integer_value(Array_pop(stack)); //first argument condition
            t->Thread.loc_cond[1] =  _Integer_value(Array_pop(stack)); //second argument condition
#else
            t = newThread(Default,stk_size);
            t->Thread.vd->Default.count = 0;
            t->Thread.func = &eve_test;
#endif
            break;
        }
        case LOOP_E:{
            t = newThread(Default,stk_size,1);
            t->Thread.vd->Default.count = 0;
            t->Thread.func = &eve_loop;
            break;
        }
        case TIMERSEC_E:{
#if SBC     
            t = newThread(VarTI,stk_size,1);
            t->Thread.vd->VarTI.v_t1 = time(NULL);
            t->Thread.vd->VarTI.v_i1  = 0;
            t->Thread.vd->VarTI.v_i2  = _Integer_value(Array_pop(stack));
            t->Thread.func = &eve_timer;
            t->Thread.loc_cond[0] =  _Integer_value(Array_pop(stack)); //first argument condition
#else
            t = newThread(Default,stk_size);
            t->Thread.vd->Default.count = 0;
            t->Thread.func = &eve_timer;
#endif
            break;
        }
        case KEYGET_E:{
            t = newThread(Default,stk_size,1);
            t->Thread.func = &eve_keyget;
            while(0!=t->Thread.func(t))dequeue(t);
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_stdlib eve[%d]\n",eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return t;
}

#endif