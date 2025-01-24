
#ifndef COMMUNICATE_EXECUTE_C
#define COMMUNICATE_EXECUTE_C

#include "../object.c"
#include "../../common/liblist/communicate.h"


oop eve_wifi_receive(oop core){
    SHICA_PRINTF("eve_wifi_receive\n");
    //corret 
    exit(0);
    for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
        oop thread = core->Core.threads[thread_i];
        if(thread->Thread.flag == 0){
#if MSGC
            gc_pushRoot((void*)&core);
            oop data = newArray(2);
            Array_push(data,_newInteger(1));
            gc_popRoots(1);
            enqueue(thread->Thread.queue,data);
#else
            oop data = newArray(2);
            Array_push(data,_newInteger(1));
            enqueue(thread->Thread.queue,data);
#endif
        }
    }
    return core;
}



oop Event_communicate(int eve_num,oop stack,int numThread){
    gc_pushRoot((void*)&stack);
    //protect t:new thread
    GC_PUSH(oop,core,0);
    switch(eve_num){
        case COMMUNICATE_WiFi_RECEIVE_E:{
            core = newCore(Default,numThread);
            core->Core.vd->Default.count = 0;
            core->Core.func = &eve_wifi_receive;
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_communicate eve[%d]\n",eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return core;
}



//Normal Function



void communicate_wifi_send(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int sendToId = api();
    int value = api();
    SHICA_PRINTF("sendToId:%d value:%d\n",sendToId,value);
    return;
}

void communicate_wifi_build_group(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    char* ipAddr = aps();
    int   portNum = api();
    SHICA_PRINTF("portNum:%d ipAddr:%s\n",portNum,ipAddr);
    return;
}

void lib_communicate(oop process,oop GM){
    getInt(mpc);int func_num = int_value;
    switch(func_num){
        case COMMUNICATE_WiFi_SEND_P:{
            communicate_wifi_send(process,GM);
            return;
        }
        case COMMUNICATE_WiFi_BUILD_GROUP_P:{
            communicate_wifi_build_group(process,GM);
            return;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Primitive_communicate func[%d]\n",func_num);
            exit(1);
        }
    }
    return;
}
#endif //COMMUNICATE_EXECUTE_C