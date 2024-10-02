#ifndef STDLIB_COMPILE_C
#define STDLIB_COMPILE_C
#include "lib_compile.c"
#include "../library.h"
#include "../stdlib.h"

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
    newEventFunc(timer_sec,STDLIB, TIMERSEC_E,1,list(_Integer),1,list(Operator)); /*FIXME: Operater should be 1*/
    newEventFunc(keyget,   STDLIB, KEYGET_E  ,1,list(_Integer),1,list(Undefined));
    return;
}
#endif