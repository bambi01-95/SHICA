#if RPI

#ifndef GPIOLIB_COMPILE_C
#define GPIOLIB_COMPILE_C


#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/gpiolib.h"

void setting_gpiolib(){
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    newEventFunc(triReadGPIO, GPIOLIB, GPIO_READ_E ,1,list(_Integer),1,list(Undefined));


    // //Primitive Function
    // /*          (name, lib_num, func_num, return_type, size_of_args_type_array,  args_type )  */
    newPrimitive(gpioSetPullUpDown, GPIOLIB, GPIO_SET_PULL_UP_DOWN_P, Undefined, 2, _Integer, _Integer);
    newPrimitive(gpioDelay,         GPIOLIB, GPIO_DELAY_P,            Undefined, 1, _Integer);
    newPrimitive(gpioWrite,         GPIOLIB, GPIO_WRITE_P,            Undefined, 2, _Integer, _Integer);
    newPrimitive(gpioSetMode,       GPIOLIB, GPIO_SET_MODE_P,         Undefined, 2, _Integer, _Integer);
    newPrimitive(gpioTerminate,     GPIOLIB, GPIO_TERMINATE_P,        Undefined, 1, _Integer);
    newPrimitive(gpioRead,          GPIOLIB, GPIO_READ_P,             Undefined, 1, _Integer);

    return;
}

#endif
#endif