#ifndef STDLIB_COMPILE_C
#define STDLIB_COMPILE_C
#include "lib.c"
#include "../../common/liblist/library.h"
#include "../../common/liblist/stdlib.h"

void setting_stdlib(){
    //Primitive Function
    /*          (name, lib_num, func_num, return_type, size_of_args_type_array,  args_type )  */
    newPrimitive(output,      STDLIB, OUTPUT_P,Undefined     ,1,_Integer);
    newPrimitive(itoc  ,      STDLIB, ITOC_P  ,_Char         ,1,_Integer);
    newPrimitive(stop,        STDLIB, EXIT_P  ,Undefined     ,1,_Integer);
    newPrimitive(appendchar,  STDLIB, APPENDCHAR_P  , String ,2,_Char,String);
    //Event Function
                /*funcname,  libnum, funcnum, num_args, args_type, num_of_pin,　pin_value */
    setEventFunc(event,    STDLIB, TEST_E    ,2,list(_Integer,_Integer),1,list(Undefined));
    setEventFunc(loop,     STDLIB, LOOP_E    ,1,list(Undefined),1,list(Undefined));
    setEventFunc(timerSec,STDLIB, TIMERSEC_E,1,list(_Integer),1,list(_Integer)); /*FIXME: Operater should be 1*/
        newEventPrim(timerSec, reset,    STDLIB, TIMERSEC_RESET_P, Undefined, 0, _Integer);
    setEventFunc(keyget,   STDLIB, KEYGET_E  ,1,list(_Integer),1,list(Undefined));
    return;
}

#endif