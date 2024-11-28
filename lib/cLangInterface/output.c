

//------< START SHICA WRAP EXE> -------------------

/* math module */

#include <stdio.h>
#include "myMath.h"


int comp(int a,int b){
    if(a>b){
        return a;
    }
    else{
        return b;
    }
}

int add(int a){
    return a;
}

int pow(int a){
    int b = 0;
    while(a--){
        b += a;
    }
    return b;
}

float pi();

float sqrt(float a,float b);

enum{
     __COMP__P,
     __ADD__P,
     __POW__P,
     __PI__P,
     __SQRT__P,
}MATH;


void math_comp(oop process, oop GM){
     int a = getint();
     int b = getint();
     Array_push(mstack, comp(a,b));
     return;
}

void math_add(oop process, oop GM){
     int a = getint();
     Array_push(mstack, add(a));
     return;
}

void math_pow(oop process, oop GM){
     int a = getint();
     Array_push(mstack, pow(a));
     return;
}

void math_pi(oop process, oop GM){
     Array_push(mstack, pi());
     return;
}

void math_sqrt(oop process, oop GM){
     float a = getfloat();
     float b = getfloat();
     Array_push(mstack, sqrt(a,b));
     return;
}

void user_lib_math(oop process,oop GM){
     getInt(mpc);
     int func_num = int value;
     switch(func_num){
         case __COMP__P: math_comp(process, GM); break;
         case __ADD__P: math_add(process, GM); break;
         case __POW__P: math_pow(process, GM); break;
         case __PI__P: math_pi(process, GM); break;
         case __SQRT__P: math_sqrt(process, GM); break;
         defualt:DEBUG_ERROR("this is not happen");
     }
}


//------< END  SHICA WRAP EXE> -------------------


//------< START SHICA WRAP COMP> -------------------


enum{
     __COMP__P,
     __ADD__P,
     __POW__P,
     __PI__P,
     __SQRT__P,
}MATH;

void setting_userlib_math(){
     newPrimitive(comp,USERLIB,COMP_P,Integer,Integer,Integer);
     newPrimitive(add,USERLIB,ADD_P,Integer,Integer);
     newPrimitive(pow,USERLIB,POW_P,Integer,Integer);
     newPrimitive(pi,USERLIB,PI_P,Float);
     newPrimitive(sqrt,USERLIB,SQRT_P,Float,Float,Float);
}


//------< END SHICA WRAP COMP> -------------------
