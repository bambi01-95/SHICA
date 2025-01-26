
#ifndef COMMUNICATE_COMPILE_C
#define COMMUNICATE_COMPILE_C

#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/communicate.h"

void setting_communicate(){
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    newEventFuncCont(wifiReceived,COMMUNICATELIB, COMMUNICATE_WiFi_RECEIVE_E, 3, list(_Integer,_Integer,_Integer), 1, list(Undefined));

    //Primitive
                /*funcname,  libnum, funcnum, return_type, num_args, args_type */
    newPrimitive(wifiSendVal, COMMUNICATELIB, COMMUNICATE_WiFi_SEND_P, Undefined, 2, _Integer, _Integer);
    newPrimitive(wifiBldGroup, COMMUNICATELIB, COMMUNICATE_WiFi_BUILD_GROUP_P, Undefined, 4, String,_Integer,_Integer,String);
    newPrimitive(wifiLeaveGroup, COMMUNICATELIB, COMMUNICATE_WiFi_LEAVE_GROUP_P, Undefined, 0);
    return;
}

#endif