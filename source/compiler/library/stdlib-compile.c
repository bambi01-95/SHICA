#ifndef STDLIB_COMPILE_C
#define STDLIB_COMPILE_C
#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/stdlib.h"

void setting_stdlib(){
    //Primitive Function
    /*          (name, lib_num, func_num, return_type, size_of_args_type_array,  args_type )  */
    newPrimitive(output, STDLIB, OUTPUT_P,Undefined ,1,_Integer);
    newPrimitive(itoc  , STDLIB, ITOC_P  ,_Char     ,1,_Integer);
    newPrimitive(exit,   STDLIB, EXIT_P  ,Undefined ,1,_Integer);
    newPrimitive(appendchar,  STDLIB, APPENDCHAR_P  , String ,2,_Char,String);
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    newEventFunc(loop,     STDLIB, LOOP_E    ,1,list(Undefined),1,list(Undefined));
    newEventFunc(timerSec,STDLIB, TIMERSEC_E,1,list(_Integer),1,list(Operator)); /*FIXME: Operater should be 1*/
    newEventFunc(keyget,   STDLIB, KEYGET_E  ,1,list(_Integer),1,list(Undefined));
    return;
}

#endif