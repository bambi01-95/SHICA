#if RPI

#ifndef GPIOLIB_H
#define GPIOLIB_H


#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/gpiolib.h"

void setting_stdlib(){
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    newEventFunc(triReadGPIO, GPIOLIB_E, GPIO_READ ,1,list(_Integer),1,list(Undefined));


    //Primitive Function
    /*          (name, lib_num, func_num, return_type, size_of_args_type_array,  args_type )  */
    newPrimitive(gpioSetPullUpDown, GPIOLIB_P, GPIO_SET_PULL_UP_DOWN_P, Undefined, 2, _Integer, _Integer);
    newPrimitive(gpioDelay, GPIOLIB_P, GPIO_DELAY_P, Undefined, 1, _Integer);
    newPrimitive(gpioWrite, GPIOLIB_P, GPIO_WRITE_P, Undefined, 2, _Integer, _Integer);
    newPrimitive(gpioSetMode, GPIOLIB_P, GPIO_SET_MODE_P, Undefined, 2, _Integer, _Integer);
    newPrimitive(gpioTerminate, GPIOLIB_P, GPIO_TERMINATE_P, Undefined, 1, _Integer);
    newPrimitive(gpioRead, GPIOLIB_P, GPIO_READ_P, Undefined, 1, _Integer);

    return;
}

#endif
#endif