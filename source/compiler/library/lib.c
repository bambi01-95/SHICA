#ifndef LIB_C
#define LIB_C
#include "../object.c"


char *typelist(int arg1, ...){
    va_list args;
    va_start(args, arg1);
    char list[32]; //maximam 32 arguments
    int i = 0;
    list[i++] = arg1;
    for(;;){
        int arg = va_arg(args, int);
        if(arg == END)break;
        list[i++] = arg;
    }
    list[i] = 0;
    va_end(args);
    char* ret = strdup(list);
    return ret;
}

#define list(...)  {__VA_ARGS__} //FIXME: using typelist instead of list

oop _setEventFunc(oop name,char lib_num,char eve_num,char eventType,char* args_type_array,char size_of_args_type_array,char * pin_num,char  size_of_pin_num){
    oop prim = newObject(EventFunc);
    prim->EventFunc.event_type   = eventType;
    prim->EventFunc.lib_num   = lib_num;
    prim->EventFunc.eve_num  = eve_num;
    char *argsType = (char *)malloc(size_of_args_type_array);
    prim->EventFunc.args_type_array = args_type_array;
    prim->EventFunc.size_of_args_type_array  = size_of_args_type_array;
    
    prim->EventFunc.pin_num_type = pin_num;
    prim->EventFunc.size_of_pin_num = size_of_pin_num;
    prim->EventFunc.pin_exps = (pin_num == 0) ? 0 : (oop *)malloc(size_of_pin_num);
    name->Symbol.value = prim;
    prim->EventFunc.ownFunclist = nil;
    return prim;
}

//not always active event: event type: 0
#define setEventFunc(name, lib_num, eve_num, size_of_args_type_array, args_type_array,size_of_pins,pins) ({ \
    static char name##__[size_of_args_type_array] = args_type_array; \
    static char name##__p[size_of_pins] =  pins ; \
    if(name##__[0]==Undefined){ \
        _setEventFunc(intern(#name), lib_num, eve_num,0,0,0,0,0);\
    } \
    else if(name##__p[0] == Undefined) _setEventFunc(intern(#name), lib_num, eve_num, 0, name##__,size_of_args_type_array,0, 0 );\
    else _setEventFunc(intern(#name), lib_num, eve_num,0, name##__,size_of_args_type_array, name##__p, size_of_pins); \
})

//always active event: event type: 1
#define setEventFuncSub(name, lib_num, eve_num, size_of_args_type_array, args_type_array,size_of_pins,pins) ({ \
    static char name##__[size_of_args_type_array] = args_type_array; \
    static char name##__p[size_of_pins] =  pins ; \
    if(name##__[0]==Undefined){ \
        _setEventFunc(intern(#name), lib_num, eve_num,1,0,0,0,0);\
    } \
    else if(name##__p[0] == Undefined) _setEventFunc(intern(#name), lib_num, eve_num,1, name##__,size_of_args_type_array,0, 0 );\
    else _setEventFunc(intern(#name), lib_num, eve_num,1, name##__,size_of_args_type_array, name##__p, size_of_pins); \
})

oop _newEventPrim(oop eventName,oop name,char lib_num,char func_num,char* args_type_array,char size_of_args_type_array,char return_type){
    oop prim = newObject(Primitive);
    prim->Primitive.lib_num   = lib_num;
    prim->Primitive.func_num  = func_num;
    prim->Primitive.return_type = return_type;
    prim->Primitive.args_type_array = args_type_array;
    prim->Primitive.size_of_args_type_array  = size_of_args_type_array;
    oop eventFunc = get(eventName,Symbol,value);
    if(getType(eventFunc)!=EventFunc){
        fatal("line %d: [%s] is not Event Funciton",get(eventName,Symbol,name));
    }
    //FIXME 
    eventName->Symbol.value->EventFunc.ownFunclist = newPair(newPair(name,prim),get(eventFunc,EventFunc,ownFunclist));
    // printlnObject(eventFunc->EventFunc.ownFunclist,0);
    return prim;
}

#define newEventPrim(eventName,name, lib_num, func_num, return_type, size_of_args_type_array,  args_type_array) ({ \
    _newEventPrim(intern(#eventName),intern(#name), lib_num, func_num, args_type_array, size_of_args_type_array , return_type);\
})


// ライブラリ用のプログラミングコード
oop _newPrimitive(oop name,char lib_num,char func_num,char* args_type_array,char size_of_args_type_array,char return_type){
    oop prim = newObject(Primitive);
    prim->Primitive.lib_num   = lib_num;
    prim->Primitive.func_num  = func_num;
    prim->Primitive.return_type = return_type;
    prim->Primitive.args_type_array = args_type_array;
    prim->Primitive.size_of_args_type_array  = size_of_args_type_array;
    name->Symbol.value = prim;
    return prim;
}

#define newPrimitive(name, lib_num, func_num, return_type, size_of_args_type_array,  ...) ({ \
    static char name##__[size_of_args_type_array] = {__VA_ARGS__}; \
    _newPrimitive(intern(#name), lib_num, func_num,name##__, size_of_args_type_array , return_type);\
})

//ADD LIB if you make
#include "stdlib-compile.c"
#include "gpiolib-compile.c"
#include "communicate-compile.c"

//#include ".h"
void setting_lib(){
    setting_stdlib();
    setting_communicate();
#if RPI
    setting_gpiolib();
#endif
    return;
}
#endif