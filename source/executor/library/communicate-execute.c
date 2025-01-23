
#ifndef COMMUNICATE_EXECUTE_C
#define COMMUNICATE_EXECUTE_C

#include "../object.c"
#include "../../common/liblist/communicate.h"


void eve_wifi_receive(oop core){
    return;
}



oop Event_communicate(int eve_num,oop stack,int numThread){
    switch(core->Core.eve_num){
        case COMMUNICATE_WiFi_RECEIVE_E:{
            eve_wifi_receive(core);
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_communicate eve[%d]\n",core->Core.eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return core;
}



//Normal Function



void communicate_wifi_send(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    return;
}

void communicate_wifi_build_group(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    return;
}

void lib_communicate(oop process,oop GM){
    getInt(mpc);int func_num = int_value;
    switch(func_num){
        case COMMUNICATE_WiFi_SEND_P:{
            communicate_wifi_send(process,GM);
            break;
        }
        case COMMUNICATE_WiFi_BUILD_GROUP_P:{
            communicate_wifi_build_group(process,GM);
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Primitive_communicate func[%d]\n",func_num);
            exit(1);
        }
    }
    return;
}
#endif //COMMUNICATE_EXECUTE_C