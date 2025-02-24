#if ROS
#ifndef ROSLIB_COMPILE_C
#define ROSLIB_COMPILE_C

#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/roslib.h"

void setting_communicate(){
    //Event Function
    //setEventFunc(funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value) */
    //rosSub(str msg) init rosSub(int queueSize, str topic, str msgType)
    setEventFuncSub(rosSub,ROSLIB, ANY_ROS_EVENT_E, 1, list(_String), 4, list(_Intger,_String,_String));
        //int rosSub.publish(str msg)
        newEventPrim(rosSub, publish, ROSLIB, COMMUNICATE_WiFi_GROUP_BROADCAST_P, _Integer, 1, typelist(_String, END));

    //Primitive(funcname,  libnum, funcnum, return_type, num_args, args_type) */
    //void spinOnce(void)
    newPrimitive(spinOnce/*funcname*/,ROSLIB, ANY_ROS_FUNC, Undefined, 0, Undefined);
    return;
}

#endif //ROSLIB_COMPILE_C
#endif //ROS