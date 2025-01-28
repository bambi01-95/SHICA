
#ifndef COMMUNICATE_COMPILE_C
#define COMMUNICATE_COMPILE_C

#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/communicate.h"

void setting_communicate(){
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    //wifiReceived(str addr, str msg)::init wifiBrReceived("addr", portNum)
    newEventFunc(wifiReceived,COMMUNICATELIB, COMMUNICATE_WiFi_RECEIVE_E, 2, list(String,String), 2, list(String,_Integer));
    //wifiBrReceived(str addr,str msg)::init wifiBrReceived("addr", portNum)
    newEventFunc(wifiBrReceived,COMMUNICATELIB, COMMUNICATE_WiFi_BROADCAST_RECEIVE_E, 2, list(String,String), 2, list(String,_Integer));
    //wifiGloupReceived(str addr,int id, int str)::no init but use wifiBldGroup() function
    newEventFunc(wifiGroupReceived,COMMUNICATELIB, COMMUNICATE_WiFi_GROUP_RECEIVE_E, 3, list(_Integer,_Integer,_Integer), 1, list(Undefined));
    //newEventFuncCont(wifiReceived,COMMUNICATELIB, COMMUNICATE_WiFi_RECEIVE_E, 3, list(_Integer,_Integer,_Integer), 1, list(Undefined));
    
    //Primitive
                /*funcname,  libnum, funcnum, return_type, num_args, args_type */
    //wifiSend(str addr,int port, str msg)
    newPrimitive(wifiSend,       COMMUNICATELIB, COMMUNICATE_WiFi_SEND_P, Undefined, 3, String, _Integer,String);
    //wifiBroadcast(str srt,int port, str msg)
    newPrimitive(wifiBroadcast,  COMMUNICATELIB, COMMUNICATE_WiFi_BROADCAST_P, Undefined, 3, String, _Integer,String);
    //wifiBuildGroup(str toId, int val)
    newPrimitive(wifiBroadcastGroup,    COMMUNICATELIB, COMMUNICATE_WiFi_GROUP_BROADCAST_P, Undefined, 2, _Integer, _Integer);
    //wifiBldGroup(str addr,int port,int GroupId, str Key)
    newPrimitive(wifiBldGroup,   COMMUNICATELIB, COMMUNICATE_WiFi_BUILD_GROUP_P, Undefined, 4, String,_Integer,_Integer,String);
    //wifiLeaveGroup()
    newPrimitive(wifiLeaveGroup, COMMUNICATELIB, COMMUNICATE_WiFi_LEAVE_GROUP_P, Undefined, 0);
    return;
}

#endif