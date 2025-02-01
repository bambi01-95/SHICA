#include "../executor/setting.h" //need to consider the compile time

#ifndef PRINT_H
#define PRINT_H

#include "./object.c"
#include "../common/inst.c"
void m(int i){
    while(i--)putchar(' ');
}

void printlnObject(oop node, int indent)
{
    switch (getType(node)) {
	case Undefined:	m(indent);SHICA_PRINTF("nil\n");				break;
    case _String :   m(indent);SHICA_PRINTF("s %s\n", getChild(node, _String, value));   break;
    case END:{
        SHICA_PRINTF("END\n");
        break;
    }
    case Array:{
        int size = node->Array.size;
        m(indent);SHICA_PRINTF("Array %d\n",size);
        for(int i = 0;i<size;i++){
            printlnObject(node->Array.elements[i],indent+1);
        }
        break;
    }
    case Queue:{
        int size = node->Queue.size;
        int head = node->Queue.head;
        m(indent);SHICA_PRINTF("QUEUE %d\n",size);
        for(int i=0;i<size;i++){
            printlnObject(node->Queue.elements[(head + i) % QUEUE_SIZE],indent+1);
        }
        break;
    }
    case Core:{
        m(indent);SHICA_PRINTF("Core\n");
        m(indent);SHICA_PRINTF("size %d\n",node->Core.size);
        for(int i=0;i<node->Core.size;i++){
            printlnObject(node->Core.threads[i],indent+1);
        }
        SHICA_PRINTF("end\n");
        break;
    }
    case Thread:{
        m(indent);SHICA_PRINTF("Thread\n");
        m(indent);SHICA_PRINTF("stack:\n");printlnObject(node->Thread.stack,indent+2);
        m(indent);SHICA_PRINTF("queue:\n");printlnObject(node->Thread.queue,indent+2);
        break;
    }
    case _Integer:m(indent);SHICA_PRINTF("i %d\n",_Integer_value(node));break;
    case _Long:   m(indent);SHICA_PRINTF("l %lld\n",getChild(node,_Long,value));break;
    case _Float:  m(indent);SHICA_PRINTF("f %f\n",_Float_value(node));break;
    case _Double: m(indent);SHICA_PRINTF("d %lf\n",getChild(node,_Double,value));break;
    case _BasePoint: m(indent);SHICA_PRINTF("b %d\n",node->_BasePoint.adress);break;
	default:{	
#if DEBUG
        SHICA_PRINTF("%s\n",TYPENAME[node->type]);assert(!"this cannot happen print");	
#else
        SHICA_PRINTF("this cannot happen printlnObject\n");
#endif		
        break;
    }
    }
}

oop printCode(oop program){
    int pc = 0;
    oop stack = newArray(10);
    for(;;){
        SHICA_PRINTF("%3d ",pc);
        oop inst = Array_get(program,pc++);
        if(getType(inst)!=_Integer){
#if DEBUG
            SHICA_PRINTF("[%s] ",TYPENAME[getType(inst)]);
#else
            SHICA_PRINTF("this cannot happen printCode\n");
#endif
            printlnObject(inst,0);
        }
        switch(_Integer_value(inst)){

            case TRANS:  SHICA_PRINTF("TRANS     %3d\n",_Integer_value(Array_get(program,pc++)));continue;
            case i_load: SHICA_PRINTF("i_load    %3d\n",_Integer_value(Array_get(program,pc++)));continue;
            case l_load: SHICA_PRINTF("l_load    %3lld\n",Array_get(program,pc++)->_Long.value);continue;
            case f_load: SHICA_PRINTF("f_load    %3f\n",_Float_value(Array_get(program,pc++)));continue;
            case d_load: SHICA_PRINTF("d_load    %3f\n",Array_get(program,pc++)->_Double.value);continue;
            case c_load: SHICA_PRINTF("i_load    %3c\n",_Char_value(Array_get(program,pc++)));continue;
            case s_load: SHICA_PRINTF("s_load    %s\n", Array_get(program,pc++)->_String.value);continue;
            case il_load: SHICA_PRINTF("il_load    %d\n", _Integer_value(Array_get(program,pc++)));continue;
            case i_EQ:   SHICA_PRINTF("i_EQ\n"); continue; 
            case i_NE:   SHICA_PRINTF("i_NE\n"); continue; 
            case i_LT:   SHICA_PRINTF("i_LT\n"); continue; 
            case i_LE:   SHICA_PRINTF("i_LE\n"); continue; 
            case i_GE:   SHICA_PRINTF("i_GE\n"); continue; 
            case i_GT:   SHICA_PRINTF("i_GT\n"); continue; 
            case i_ADD:  SHICA_PRINTF("i_ADD\n");continue;
            case i_SUB:  SHICA_PRINTF("i_SUB\n");continue;
            case i_MUL:  SHICA_PRINTF("i_MUL\n");continue;
            case i_DIV:  SHICA_PRINTF("i_DIV\n");continue;
            case i_MOD:  SHICA_PRINTF("i_MOD\n");continue;
            case l_EQ:   SHICA_PRINTF("l_EQ\n");continue; 
            case l_NE:   SHICA_PRINTF("l_NE\n");continue; 
            case l_LT:   SHICA_PRINTF("l_LT\n");continue; 
            case l_LE:   SHICA_PRINTF("l_LE\n");continue; 
            case l_GE:   SHICA_PRINTF("l_GE\n");continue; 
            case l_GT:   SHICA_PRINTF("l_GT\n");continue; 
            case l_ADD:  SHICA_PRINTF("l_ADD\n");continue;
            case l_SUB:  SHICA_PRINTF("l_SUB\n");continue;
            case l_MUL:  SHICA_PRINTF("l_MUL\n");continue;
            case l_DIV:  SHICA_PRINTF("l_DIV\n");continue;
            case l_MOD:  SHICA_PRINTF("l_MOD\n");continue;
            case f_EQ:   SHICA_PRINTF("f_EQ\n");continue; 
            case f_NE:   SHICA_PRINTF("f_NE\n");continue; 
            case f_LT:   SHICA_PRINTF("f_LT\n");continue; 
            case f_LE:   SHICA_PRINTF("f_LE\n");continue; 
            case f_GE:   SHICA_PRINTF("f_GE\n");continue; 
            case f_GT:   SHICA_PRINTF("f_GT\n");continue; 
            case f_ADD:  SHICA_PRINTF("f_ADD\n");continue;
            case f_SUB:  SHICA_PRINTF("f_SUB\n");continue;
            case f_MUL:  SHICA_PRINTF("f_MUL\n");continue;
            case f_DIV:  SHICA_PRINTF("f_DIV\n");continue;
            case d_EQ:   SHICA_PRINTF("d_EQ\n");continue; 
            case d_NE:   SHICA_PRINTF("d_NE\n");continue; 
            case d_LT:   SHICA_PRINTF("d_LT\n");continue; 
            case d_LE:   SHICA_PRINTF("d_LE\n");continue; 
            case d_GE:   SHICA_PRINTF("d_GE\n");continue; 
            case d_GT:   SHICA_PRINTF("d_GT\n");continue; 
            case d_ADD:  SHICA_PRINTF("d_ADD\n");continue;
            case d_SUB:  SHICA_PRINTF("d_SUB\n");continue;
            case d_MUL:  SHICA_PRINTF("d_MUL\n");continue;
            case d_DIV:  SHICA_PRINTF("d_DIV\n");continue;
            case s_EQ:   SHICA_PRINTF("d_EQ\n");continue; 
            case s_NE:   SHICA_PRINTF("d_NE\n");continue; 
            case s_LT:   SHICA_PRINTF("d_LT\n");continue; 
            case s_LE:   SHICA_PRINTF("d_LE\n");continue; 
            case s_GE:   SHICA_PRINTF("d_GE\n");continue; 
            case s_GT:   SHICA_PRINTF("d_GT\n");continue; 
            case s_ADD:  SHICA_PRINTF("d_ADD\n");continue;
            case MKCORE:{
                SHICA_PRINTF("MKCORE    %3d\n",_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case COPYCORE:{
                SHICA_PRINTF("COPYCORE ");//T
                int indexOfGlobalMemory = _Integer_value(Array_get(program,pc++));
                int jumpRelPos = _Integer_value(Array_get(program,pc++));
                SHICA_PRINTF("%3d  %3d (to %d)\n",indexOfGlobalMemory,jumpRelPos,pc+jumpRelPos);
                continue;
            }
            case SETCORE:{
                int lib_num = _Integer_value(Array_get(program,pc++));
                int func_num = _Integer_value(Array_get(program,pc++));
                int num_init_val = _Integer_value(Array_get(program,pc++));
                SHICA_PRINTF("SETCORE %3d  %3d  %3d\n",lib_num,func_num,num_init_val);
                continue;
            }
            case SETSUBCORE:{
                int lib_num = _Integer_value(Array_get(program,pc++));
                int func_num = _Integer_value(Array_get(program,pc++));
                int num_init_val = _Integer_value(Array_get(program,pc++));
                SHICA_PRINTF("SETSUBCORE %3d  %3d  %3d\n",lib_num,func_num,num_init_val);
                continue;
            }
            case MKTHREAD:{
                oop numThread = Array_get(program,pc++);
                SHICA_PRINTF("MKTHREAD %3d\n",_Integer_value(numThread));
                continue;
            }
            case SETTHREAD: {
                oop aRelPos        = Array_get(program,pc++);
                oop cRelPos        = Array_get(program,pc++);
                SHICA_PRINTF("SETTHREAD %3d  %3d\n",_Integer_value(aRelPos),_Integer_value(cRelPos));
                continue;
            }
            case STARTIMP: SHICA_PRINTF("STARTIMP\n");continue;
            case EOE:    SHICA_PRINTF("EOE\n");continue;
            case EOC:    SHICA_PRINTF("EOC\n");continue;
            case EOA:    SHICA_PRINTF("EOA\n");continue;
            case COND:   SHICA_PRINTF("COND\n");continue;

            case CALL:{
                SHICA_PRINTF("CALL      ");//T
                oop num_arg = Array_get(program,pc++);
                oop index = Array_get(program,pc++);
                SHICA_PRINTF("%3d  %3d\n",_Integer_value(num_arg),_Integer_value(index));
                continue;
            }
            case CALL_P:{
                SHICA_PRINTF("CALL_P     ");//T
                oop lib_num  = Array_get(program,pc++);
                oop func_num = Array_get(program,pc++);
                oop num_args = Array_get(program,pc++);
                SHICA_PRINTF("%3d  %3d  %3d\n",_Integer_value(lib_num),_Integer_value(func_num),_Integer_value(num_args));
                continue;
            }
            case CALL_A:{
                SHICA_PRINTF("CALL_A     ");//T
                oop lib_num  = Array_get(program,pc++);
                oop func_num = Array_get(program,pc++);
                oop num_args = Array_get(program,pc++);
                SHICA_PRINTF("%3d  %3d  %3d\n",_Integer_value(lib_num),_Integer_value(func_num),_Integer_value(num_args));
                continue;
            }
            case GET:{
                SHICA_PRINTF("Get       ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case GET_L:{
                SHICA_PRINTF("Get_L     ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case GET_G:{
                SHICA_PRINTF("GET_G     ");
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);
                continue;
            }
            case DEFINE:{
                SHICA_PRINTF("DEFINE    ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case DEFINE_L:{
                SHICA_PRINTF("DEFINE_L    ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case DEFINE_G:{
                SHICA_PRINTF("DEFINE_G  ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }  
            case DEFINE_List:{
                SHICA_PRINTF("DEFINE_List  ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case GLOBAL:{
                SHICA_PRINTF("GLOBAL\n");
                continue;
            }
            case GLOBAL_END:{
                SHICA_PRINTF("GLOBAL_END\n");
                continue;
            }
            case ENTRY:{
                SHICA_PRINTF("ENTRY     %3d\n",_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case SETQ:{
                SHICA_PRINTF("SETQ\n");
                continue;
            }
            case RET:{
                SHICA_PRINTF("RET       ");
                SHICA_PRINTF("  X\n");
                continue;
            }
            case MSUB:{
                SHICA_PRINTF("MSUB      ");
                printlnObject(Array_get(program,pc++),1);
                continue;
            }
            case MPOP:{
                SHICA_PRINTF("MPOP\n");
                continue;
            }
            case MPICK:{
                SHICA_PRINTF("MPICK      ");
                int i = _Integer_value(Array_get(program,pc++));
                int j = _Integer_value(Array_get(program,pc++));
                SHICA_PRINTF("%3d %3d\n",i,j);
                continue;
            }
            case MSET:{
                SHICA_PRINTF("MSET      ");
                SHICA_PRINTF("%3d\n",_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case JUMPF:{
                SHICA_PRINTF("jumpF     ");//T
                int offset = _Integer_value(Array_get(program,pc++)); 
                SHICA_PRINTF("%3d\n",offset);//T
                continue;
            }
            case JUMP:{
                SHICA_PRINTF("jump      ");
                int offset = _Integer_value(Array_get(program,pc++)); 
                SHICA_PRINTF("%3d\n",offset);
                continue;       
            }
            case i_PRINT:{
                SHICA_PRINTF("i PRINT\n");
                continue;
            }
            case l_PRINT:{
                SHICA_PRINTF("l PRINT\n");
                continue;
            }
            case f_PRINT:{
                SHICA_PRINTF("f PRINT\n");
                continue;
            }
            case d_PRINT:{
                SHICA_PRINTF("d PRINT\n");
                continue;
            }
            case c_PRINT:{
                SHICA_PRINTF("c PRINT\n");
                continue;
            }
            case s_PRINT:{
                SHICA_PRINTF("s PRINT\n");
                continue;
            }
            case HALT:{
                SHICA_PRINTF("HALT\n");
                return nil;
            }
            default:{
                printf("%s line %d this is not happen \n",__FILE__,__LINE__);
            }
        }
    }
}
#endif