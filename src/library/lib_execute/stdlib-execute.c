#ifndef STDLIB_EXECUTE_C
#define STDLIB_EXECUTE_C
#include "../../object/object.c"
#include "../stdlib.h"

#include "../../executer/executer.c"

/*
    threads: multi function thread space
    argsCond: それぞれのconditionの内容が入っている
*/

// int getNonBocking();
// oop eventFunction(oop threads, oop argsCond){
//     int data = getNonBocking();
//     for(threads;;){
//         if
//     }
// }

oop eve_loop(oop t){
    if(t->Thread.flag == 0){
        oop data = newArray(2);
        Array_push(data,new_Basepoint(1));//1st rbp, 2nd.. event args, 
        Array_push(data,_newInteger(1));
        enqueue(t,data);
    }
    return t;
}

oop eve_timer(oop t){
    time_t current_time = time(NULL);
    if(current_time - t->Thread.vd->VarTI.v_t1 >= t->Thread.vd->VarTI.v_i2){
        t->Thread.vd->VarTI.v_t1 = current_time;
        t->Thread.vd->VarTI.v_i1  += t->Thread.vd->VarTI.v_i2;
        oop data = newArray(2);
        Array_push(data,new_Basepoint(1));//1st rbp, 2nd.. event args, 
        Array_push(data,_newInteger(t->Thread.vd->VarTI.v_i1));
        enqueue(t,data);
    }
    return t;
}

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
        oop data = newArray(2);
        Array_push(data,new_Basepoint(1));//1st rbp, 2nd.. event args, 
        Array_push(data,_newInteger(buf));
        enqueue(t,data);
        return t;
    }
}

//STDLIB EVENT
oop Event_stdlib(int eve_num,oop stack){
    switch(eve_num){
        case LOOP_E:{
            oop t = newThread(Default);
            t->Thread.vd->Default.count = 0;
            t->Thread.func = &eve_loop;
            return t;
        }
        case TIMERSEC_E:{
            oop t = newThread(VarTI);
            t->Thread.vd->VarTI.v_t1 = time(NULL);
            t->Thread.vd->VarTI.v_i1  = 0;
            t->Thread.vd->VarTI.v_i2  = _Integer_value(Array_pop(stack));
            t->Thread.func = &eve_timer;
            return t;
        }
        case KEYGET_E:{
            oop t = newThread(Default);
            t->Thread.func = &eve_keyget;
            while(0!=t->Thread.func(t))dequeue(t);
            return t;
        }
        default:{
            fprintf(stderr,"this is not happen Event_stdlib eve[%d]\n",eve_num);
            exit(1);
        }
    }
    printf("Event_stdlib(): not happen..\n");
    return 0;
}
#endif