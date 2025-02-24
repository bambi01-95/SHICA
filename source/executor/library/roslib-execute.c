#if ROS

#ifndef ROSLIB_EXECUTE_C
#define ROSLIB_EXECUTE_C

#include "../object.c"
#include "../../common/liblist/roslib.h"


#if SBC //event()
oop eve_roslib_subscribe(oop core){
    char* rosdata = 0;//call ros::subscribe() here!
    if(rosdata > 0){
        int isOnce = 0;
        evalEventArgsThread->Thread.stack->Array.size = 1;//1:basepoint
        for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
            int isFalse = 0;
            oop thread = core->Core.threads[thread_i];
            //<引数の評価>/<Evaluation of arguments>
            if(thread->Thread.condRelPos != 0){
                if(isOnce == 0){
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(gpio_states));
                    isOnce = 1;
                }else
                {
                    evalEventArgsThread->Thread.stack->Array.size = 2;
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
                Array_push(data,_newInteger(rosdata));
                gc_popRoots(1);
                enqueue(thread->Thread.queue,data);
            }
        }
    }
    return core;
}
#else
oop eve_gpio_read(oop core){
    SHICA_PRINTF("eve_test\n");
    return core;
}
#endif

#if SBC
void init_ros_subscribe(oop subcore) {
    // 引数チェック
    oop *elements = subcore->SubCore.var->FixArray.elements; 
    int pin  = _Integer_value(elements[0]);
    char* mode  = getChild(elements[1],_String,value);
    char* pull  = getChild(elements[2],_String,value);

    // 引数チェック
    if (mode == NULL || pull == NULL) {
        SHICA_FPRINTF(stderr, "Invalid argument\n");
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
    }
    //call ros::init() her!
    return;
}

oop Event_roslib(int eve_num,oop stack){
    //cheack: protect stack, but it is already protected
    gc_pushRoot((void*)&stack);
    //protect t:new thread
    GC_PUSH(oop,core,0);
    switch(eve_num){
        case ANY_ROS_EVENT_E:{
            oop subcore = newSubCore(3);
            subcore->SubCore.func = &eve_roslib_subscribe;
            subcore->SubCore.var->FixArray.elements[0] = Array_pop(stack);//Integer  pin
            subcore->SubCore.var->FixArray.elements[1] = Array_pop(stack);//Integer  mode
            subcore->SubCore.var->FixArray.elements[2] = Array_pop(stack);//Integer  pull
            core = subcore;
            init_ros_subscribe(subcore);
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_roslib eve[%d]\n",eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return core;
}



//Normal Function
void roslib_onceSpine(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int value = 0;//call ros::onceSpin() here!
    Array_push(mstack,_newInteger(value));
    return;
}

void lib_roslib(oop process,oop GM){
    getInt(mpc);int lib_num = int_value;
    switch(lib_num){
        case     ANY_ROS_FUNC_P:roslib_onceSpine(process,GM);return;
        case ANY_ROS_FUNC_OF_EVENT_P:
        default:{
#if DEBUG
            DEBUG_ERROR("this is not happen, lib_roslib\n");
            exit(1);
#else
            SHICA_PRINTF("Call_Primitive_ROS %d\n",lib_num);
            exit(1);
#endif
        }
    }
    return;
}
#endif //GPIOLIB_H
#endif //RPI