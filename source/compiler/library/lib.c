#ifndef LIB_C
#define LIB_C
#include "../object.c"
#define list(...)  {__VA_ARGS__}

oop _setEventFunc(oop name,char lib_num,char eve_num,char eventType,char* args_type_array,char size_of_args_type_array,char * pin_num,char  size_of_pin_num){
    oop prim = newObject(EventFunc);
    prim->EventFunc.lib_num   = eventType;
    prim->EventFunc.lib_num   = lib_num;
    prim->EventFunc.eve_num  = eve_num;
    char *argsType = (char *)malloc(size_of_args_type_array);
    prim->EventFunc.args_type_array = args_type_array;
    prim->EventFunc.size_of_args_type_array  = size_of_args_type_array;
    
    prim->EventFunc.pin_num_type = pin_num;
    prim->EventFunc.size_of_pin_num = size_of_pin_num;
    prim->EventFunc.pin_exps = (pin_num == 0) ? 0 : (oop *)malloc(size_of_pin_num);
    name->Symbol.value = prim;
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
#define setEventFuncCont(name, lib_num, eve_num, size_of_args_type_array, args_type_array,size_of_pins,pins) ({ \
    static char name##__[size_of_args_type_array] = args_type_array; \
    static char name##__p[size_of_pins] =  pins ; \
    if(name##__[0]==Undefined){ \
        _setEventFunc(intern(#name), lib_num, eve_num,1,0,0,0,0);\
    } \
    else if(name##__p[0] == Undefined) _setEventFunc(intern(#name), lib_num, eve_num,1, name##__,size_of_args_type_array,0, 0 );\
    else _setEventFunc(intern(#name), lib_num, eve_num,1, name##__,size_of_args_type_array, name##__p, size_of_pins); \
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