
#ifndef COMMUNICATE_COMPILE_C
#define COMMUNICATE_COMPILE_C

#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/communicate.h"

void setting_communicate(){
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    newEventFunc(COMMUNICATE_WiFi_RECEIVE_E, COMMUNICATE, 0, 0, 0, 0, 0);
    //Primitive
                /*funcname,  libnum, funcnum, return_type, num_args, args_type */
    newPrimitive(COMMUNICATE_WiFi_SEND_P, COMMUNICATE, 0, Undefined, 0, 0);
    newPrimitive(COMMUNICATE_WiFi_BUILD_GROUP_P, COMMUNICATE, 1, Undefined, 0, 0);
    return;
}