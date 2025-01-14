#ifndef GENERATE_C
#define GENERATE_C
#include "object.c"
#include "../common/inst.c"
#include "../common/memory.c"



#define genByte(A,B) ({\
    for(int i=0;i<B;i++){\
        _genByte(A[i]);\
    }\
})


#define genData(S,D) ({ \
    unsigned char buffer[S]; \
    memcpy(buffer, &D, S); \
    genByte(buffer,S); \
})

void genOp(int data){
    _genByte((byte)data);
}

const unsigned int SIZE_INST   = sizeof(unsigned char);
const unsigned int SIZE_INT    = sizeof(int);            //size of int
const unsigned int SIZE_LONG   = sizeof(long long int);  //size of long long int
const unsigned int SIZE_FLOAT  = sizeof(float);          //size of float
const unsigned int SIZE_DOUBLE = sizeof(double);         //size of double
#define genChar(A) _genByte((byte)A)

void genInt(int data){
    genData(SIZE_INT,data);
}

void genLong(long long int data){
    genData(SIZE_LONG,data);
}

void genFloat(float data){
    genData(SIZE_FLOAT,data);
}

void genDouble(double data){
    genData(SIZE_DOUBLE,data);
}

void genString(char *data){
    genByte(data,strlen(data));
    _genByte('\0');
}



/*
    end memory
*/


oop CodeWrite(oop program){
    int pc = 0;
    oop stack = newArray(10);
    for(;;){
#if TEST
        // printf("%3d\n",pc);
#endif
        oop inst = Array_get(program,pc++);
        if(getType(inst)!=_Integer){
            printf("[%s] ",TYPENAME[getType(inst)]);
            printlnObject(inst,0);
        }
        int op = _Integer_value(inst);
        genOp(op);
        switch(op){
            case TRANS: genInt(_Integer_value(Array_get(program,pc++)));continue;
            case i_load:genInt(_Integer_value(Array_get(program,pc++)));continue;
            case l_load:genLong(Array_get(program,pc++)->_Long.value);continue;
            case f_load: genFloat(_Float_value(Array_get(program,pc++)));continue;
            case d_load: genDouble(Array_get(program,pc++)->_Double.value);continue;
            case c_load: genChar(_Char_value(Array_get(program,pc++)));continue;
            case s_load: genString(Array_get(program,pc++)->String.value);continue;
            case il_load:genInt(_Integer_value(Array_get(program,pc++)));continue;
            case i_EQ:   continue; 
            case i_NE:   continue; 
            case i_LT:   continue; 
            case i_LE:   continue; 
            case i_GE:   continue; 
            case i_GT:   continue; 
            case i_ADD:  continue;
            case i_SUB:  continue;
            case i_MUL:  continue;
            case i_DIV:  continue;
            case i_MOD:  continue;
            case i_BAND: continue;
            case i_BOR:  continue;
            case i_LSH:  continue;
            case i_RSH:  continue;
            case l_EQ:   continue; 
            case l_NE:   continue; 
            case l_LT:   continue; 
            case l_LE:   continue; 
            case l_GE:   continue; 
            case l_GT:   continue; 
            case l_ADD:  continue;
            case l_SUB:  continue;
            case l_MUL:  continue;
            case l_DIV:  continue;
            case l_MOD:  continue;
            case f_EQ:   continue; 
            case f_NE:   continue; 
            case f_LT:   continue; 
            case f_LE:   continue; 
            case f_GE:   continue; 
            case f_GT:   continue; 
            case f_ADD:  continue;
            case f_SUB:  continue;
            case f_MUL:  continue;
            case f_DIV:  continue;
            case d_EQ:   continue; 
            case d_NE:   continue; 
            case d_LT:   continue; 
            case d_LE:   continue; 
            case d_GE:   continue; 
            case d_GT:   continue; 
            case d_ADD:  continue;
            case d_SUB:  continue;
            case d_MUL:  continue;
            case d_DIV:  continue;

            case s_EQ:   continue; 
            case s_NE:   continue; 
            case s_LT:   continue; 
            case s_LE:   continue; 
            case s_GE:   continue; 
            case s_GT:   continue; 
            case s_ADD:  continue;
            case MKCORE:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case MKTHREAD:{
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case SETTHREAD:{
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case STARTIMP: continue;
            case EOE:      continue;
            case EOC:      continue;
            case EOA:      continue;
            case COND:     continue;
            case CALL:{
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case CALL_P:{
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case CALL_A:{
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;         
            }
            case GET:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case GET_L:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case GET_G:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case DEFINE:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case DEFINE_L:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case DEFINE_G:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }  
            case DEFINE_List:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }  
            case ENTRY:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case GLOBAL:{
                continue;
            }
            case GLOBAL_END:{
                continue;
            }
            case SETQ:{
                continue;
            }
            case RET:{
                continue;
            }
            case MSUB:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case MPOP:{
                continue;
            }
            case MPICK:{
                genInt(_Integer_value(Array_get(program,pc++)));
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case MSET:{
                genInt(_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case JUMPF:{
                genInt(_Integer_value(Array_get(program,pc++))); 
                continue;
            }
            case JUMP:{
                genInt(_Integer_value(Array_get(program,pc++))); 
                continue;       
            }
            case i_PRINT:{
                continue;
            }
            case l_PRINT:{
                continue;
            }
            case f_PRINT:{
                continue;
            }
            case d_PRINT:{
                continue;
            }
            case c_PRINT:{
                continue;
            }
            case s_PRINT:{
                continue;
            }
            case HALT:{
                return nil;
            }
            default:{
#if DEBUG
                printf("%s line %d this is not happen %s\n",__FILE__,__LINE__,INSTNAME[op]);
#else
                printf("%s line %d this is not happen \n",__FILE__,__LINE__);
#endif
            }
        }
    }
}
#endif