#if RPI

#ifndef GPIOLIB_EXECUTE_C
#define GPIOLIB_EXECUTE_C

#include "../object.c"
#include "../../common/liblist/gpiolib.h"

#include <pigpio.h>

#if SBC //event()
oop eve_gpio_read(oop core){
    uint32_t gpio_states = gpioRead_Bits_0_31();
    if(gpio_states > 0){
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
                Array_push(data,_newInteger(gpio_states));
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

oop Event_gpiolib(int eve_num,oop stack){
    //cheack: protect stack, but it is already protected
    gc_pushRoot((void*)&stack);
    //protect t:new thread
    GC_PUSH(oop,core,0);
    switch(eve_num){
        case GPIO_READ_E:{
            core = newCore(Default);
            core->Core.vd->Default.count = 0;
            core->Core.func = &eve_gpio_read;
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_gpiolib eve[%d]\n",eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return core;
}



//Normal Function

void gpiolib_initialise(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int value = gpioInitialise();
    Array_push(mstack,_newInteger(value));
    return;
}

void gpiolib_setMode(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int pin = api();
    int mode =api();
    gpioSetMode(pin,mode);
    return;
}

void gpiolib_write(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int pin = api();
    int value =api();
    gpioWrite(pin,value);
    return;
}

void gpiolib_read(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int pin = api();
    int value = gpioRead(pin);
    Array_push(mstack,_newInteger(value));
    return;
}

void gpiolib_setPullUpDown(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int pin = api();
    int pud =api();
    gpioSetPullUpDown(pin,pud);
    return;
}

//Need to change, its affect to other process
void gpiolib_delay(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int value = api();
    gpioDelay(value);
    return;
}

void gpiolib_terminate(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    gpioTerminate();
    return;
}

void lib_gpiolib(oop process,oop GM){
    getInt(mpc);int lib_num = int_value;
    switch(lib_num){
        case GPIO_INITIALISE_P:gpiolib_initialise(process,GM);return;
        case GPIO_SET_MODE_P:gpiolib_setMode(process,GM);return;
        case GPIO_WRITE_P:gpiolib_write(process,GM);return;
        case GPIO_READ_P:gpiolib_read(process,GM);return;
        case GPIO_SET_PULL_UP_DOWN_P:gpiolib_setPullUpDown(process,GM);return;
        case GPIO_DELAY_P:gpiolib_delay(process,GM);return;
        case GPIO_TERMINATE_P:gpiolib_terminate(process,GM);return;
        default:{
#if DEBUG
            DEBUG_ERROR("this is not happen, lib_gpiolib\n");
            exit(1);
#else
            SHICA_PRINTF("Call_Primitive_GPIO %d\n",lib_num);
            exit(1);
#endif
        }
    }
    return;
}
#endif //GPIOLIB_H
#endif //RPI