#ifndef LIB_C
#define LIB_C

#include "../../common/liblist/library.h"
#include "./stdlib-execute.c"

#if RPI
#include "./gpiolib-execute.c"
#endif

oop Call_Primitive(oop process,oop GM){
    getInt(mpc);int lib_num = int_value;
    switch(lib_num){
        case STDLIB:{
            lib_stdlib(process,GM);
            break;
        }
        case USERLIB:{
            SHICA_PRINTF("this is not supportted\n");
            break;
        }
        case COMMUNICATELIB:{
            lib_communicate(process,GM);
            break;
        }
#if RPI
        case GPIOLIB:{
            lib_gpiolib(process,GM);
            break;
        }
#endif
        default:{
            SHICA_PRINTF("Call_Primitive %d\n",lib_num);
            exit(1);
        }
    }
    return nil;
}

#endif