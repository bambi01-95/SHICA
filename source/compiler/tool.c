#ifndef PRINT_H
#define PRINT_H

#include "./object.c"
#include "../common/inst.c"
void printlnObject(oop node, int indent)
{
    printf("%*s", indent*2, "");
    switch (getType(node)) {
	case Undefined:	printf("nil\n");				break;
	case Integer:	printf("%s\n", get(node, Integer,number));	break;
	case Symbol :	printf("%s\n", get(node, Symbol,name));		break;
    case Float :    printf("%s\n",get(node,   Float,number));     break;
    case String :   printf("%s\n", get(node, String, value));   break;// c5
    case Key:       printf("%s\n", get(node, Key,    pass));    break;
	case Pair: {
	    printf("Pair\n");
	    printlnObject(get(node, Pair,a), indent+1);
	    printlnObject(get(node, Pair,b), indent+1);
	    break;
	}
	case Function: {
	    printf("function()\n");
	    printlnObject(get(node, Function,parameters), indent+2);
	    printlnObject(get(node, Function,body), indent+1);
	    break;
	}
	case Binop: {
	    switch (get(node, Binop,op)) {
		case NE:  printf("NE\n"); break;
		case EQ:  printf("EQ\n"); break;
		case LT:  printf("LT\n"); break;
		case LE:  printf("LE\n"); break;
		case GE:  printf("GE\n"); break;
		case GT:  printf("GT\n"); break;
		case ADD: printf("ADD\n"); break;
		case SUB: printf("SUB\n"); break;
		case MUL: printf("MUL\n"); break;
		case DIV: printf("DIV\n"); break;
		case MOD: printf("MOD\n"); break;
		default:  assert(!"this cannot happen binop");
	    }
	    printlnObject(get(node, Binop,lhs), indent+1);
	    printlnObject(get(node, Binop,rhs), indent+1);
	    break;
	}
	case Unyop: {
	    switch (get(node, Unyop,op)) {
		case NEG: printf("NEG\n"); break;
		default:  assert(!"this cannot happen unyop");
	    }
	    printlnObject(get(node, Unyop,rhs), indent+1);
	    break;
	}
	case GetVar: {
	    printf("GetVar %s\n", get(get(node, GetVar,id), Symbol,name));
	    break;
	}
	case SetVar: {
	    printf("SetVar %s\n", get(get(node, SetVar,id), Symbol,name));
	    printlnObject(get(node, SetVar,rhs), indent+1);
	    break;
	}
	case Call: {
	    printf("Call\n");
	    printlnObject(get(node, Call,function), indent+1);
	    printlnObject(get(node, Call,arguments), indent+1);
	    break;
	}
	case Print: {
	    printf("Print\n");
	    printlnObject(get(node, Print,argument), indent+1);
	    break;
	}
	case If: {
	    printf("If\n");
	    printlnObject(get(node, If,condition), indent+1);
	    printlnObject(get(node, If,statement1), indent+1);
	    printlnObject(get(node, If,statement2), indent+1);
	    break;
	}
	case While: {
	    printf("While\n");
	    printlnObject(get(node, While,condition), indent+1);
	    printlnObject(get(node, While,statement), indent+1);
	    break;
	}
	case Block: {
	    printf("Block...\n");
	    break;
	}
    case Event:{
        printf("Event\n");
        break;
    }
    case State:{
        printf("State\n");
        break;
    }
    case Run:{
        printf("Run\n");
        break;
    }
    case Continue:{
        printf("Continue\n");
        break;
    }
    case Break:{
        printf("Break\n");
        break;
    }
    case Return:{
        printf("Return\n");
        break;
    }
    case END:{
        printf("END\n");
        break;
    }
    case Array:{
        int size = node->Array.size;
        printf("Array\n");
        for(int i = 0;i<size;i++){
            printlnObject(node->Array.elements[i],indent+1);
        }
        break;
    }
    case Assoc:{
        printf("type %2d, index %3d",node->Assoc.kind,node->Assoc.index);
        printlnObject(node->Assoc.symbol,indent);
        break;
    }
    case _Integer:printf("%d\n",_Integer_value(node));break;
    case _Long:   printf("%lld\n",get(node,_Long,value));break;
    case _Float:  printf("%f\n",_Float_value(node));break;
    case _Double: printf("%lf\n",get(node,_Double,value));break;
	default:	printf("%s\n",TYPENAME[node->_type_]);assert(!"this cannot happen print");			break;
    }
}

oop printCode(oop program){
    int pc = 0;
    oop stack = newArray(10);
    for(;;){
        printf("%3d ",pc);
        oop inst = Array_get(program,pc++);
        if(getType(inst)!=_Integer){
            printf("[%s] ",TYPENAME[getType(inst)]);
            printlnObject(inst,0);
        }
        switch(_Integer_value(inst)){

            case TRANS:  printf("TRANS     %3d\n",_Integer_value(Array_get(program,pc++)));continue;
            case i_load: printf("i_load    %3d\n",_Integer_value(Array_get(program,pc++)));continue;
            case l_load: printf("l_load    %3lld\n",Array_get(program,pc++)->_Long.value);continue;
            case f_load: printf("f_load    %3f\n",_Float_value(Array_get(program,pc++)));continue;
            case d_load: printf("d_load    %3f\n",Array_get(program,pc++)->_Double.value);continue;
            case c_load: printf("i_load    %3c\n",_Char_value(Array_get(program,pc++)));continue;
            case s_load: printf("s_load    %s\n", Array_get(program,pc++)->String.value);continue;
            case il_load: printf("il_load    %d\n", _Integer_value(Array_get(program,pc++)));continue;
            case i_EQ:   printf("i_EQ\n"); continue; 
            case i_NE:   printf("i_NE\n"); continue; 
            case i_LT:   printf("i_LT\n"); continue; 
            case i_LE:   printf("i_LE\n"); continue; 
            case i_GE:   printf("i_GE\n"); continue; 
            case i_GT:   printf("i_GT\n"); continue; 
            case i_ADD:  printf("i_ADD\n");continue;
            case i_SUB:  printf("i_SUB\n");continue;
            case i_MUL:  printf("i_MUL\n");continue;
            case i_DIV:  printf("i_DIV\n");continue;
            case i_MOD:  printf("i_MOD\n");continue;
            case l_EQ:   printf("l_EQ\n");continue; 
            case l_NE:   printf("l_NE\n");continue; 
            case l_LT:   printf("l_LT\n");continue; 
            case l_LE:   printf("l_LE\n");continue; 
            case l_GE:   printf("l_GE\n");continue; 
            case l_GT:   printf("l_GT\n");continue; 
            case l_ADD:  printf("l_ADD\n");continue;
            case l_SUB:  printf("l_SUB\n");continue;
            case l_MUL:  printf("l_MUL\n");continue;
            case l_DIV:  printf("l_DIV\n");continue;
            case l_MOD:  printf("l_MOD\n");continue;
            case f_EQ:   printf("f_EQ\n");continue; 
            case f_NE:   printf("f_NE\n");continue; 
            case f_LT:   printf("f_LT\n");continue; 
            case f_LE:   printf("f_LE\n");continue; 
            case f_GE:   printf("f_GE\n");continue; 
            case f_GT:   printf("f_GT\n");continue; 
            case f_ADD:  printf("f_ADD\n");continue;
            case f_SUB:  printf("f_SUB\n");continue;
            case f_MUL:  printf("f_MUL\n");continue;
            case f_DIV:  printf("f_DIV\n");continue;
            case d_EQ:   printf("d_EQ\n");continue; 
            case d_NE:   printf("d_NE\n");continue; 
            case d_LT:   printf("d_LT\n");continue; 
            case d_LE:   printf("d_LE\n");continue; 
            case d_GE:   printf("d_GE\n");continue; 
            case d_GT:   printf("d_GT\n");continue; 
            case d_ADD:  printf("d_ADD\n");continue;
            case d_SUB:  printf("d_SUB\n");continue;
            case d_MUL:  printf("d_MUL\n");continue;
            case d_DIV:  printf("d_DIV\n");continue;
            case s_EQ:   printf("d_EQ\n");continue; 
            case s_NE:   printf("d_NE\n");continue; 
            case s_LT:   printf("d_LT\n");continue; 
            case s_LE:   printf("d_LE\n");continue; 
            case s_GE:   printf("d_GE\n");continue; 
            case s_GT:   printf("d_GT\n");continue; 
            case s_ADD:  printf("d_ADD\n");continue;
            case THREAD: printf("thread    %3d\n",_Integer_value(Array_get(program,pc++)));continue;
            case EOE:    printf("EOE\n");continue;

            case CALL:{
                printf("CALL      ");//T
                oop num_arg = Array_get(program,pc++);
                oop index = Array_get(program,pc++);
                printf("%3d  %3d\n",_Integer_value(num_arg),_Integer_value(index));
                continue;
            }
            case CALL_P:{
                printf("CALL_P     ");//T
                oop lib_num  = Array_get(program,pc++);
                oop func_num = Array_get(program,pc++);
                oop num_args = Array_get(program,pc++);
                printf("%3d  %3d  %3d\n",_Integer_value(lib_num),_Integer_value(func_num),_Integer_value(num_args));
                continue;
            }
            case CALL_E:{
                printf("CALL_E     ");//T
                oop lib_num  = Array_get(program,pc++);
                oop func_num = Array_get(program,pc++);
                oop num_args = Array_get(program,pc++);
                printf("%3d  %3d  %3d\n",_Integer_value(lib_num),_Integer_value(func_num),_Integer_value(num_args));
                continue;
            }
            case GET:{
                printf("Get       ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case GET_L:{
                printf("Get_L     ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case GET_G:{
                printf("GET_G     ");
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);
                continue;
            }
            case DEFINE:{
                printf("DEFINE    ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case DEFINE_L:{
                printf("DEFINE_L    ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }
            case DEFINE_G:{
                printf("DEFINE_G  ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }  
            case DEFINE_List:{
                printf("DEFINE_List  ");//T
                oop symbol = Array_get(program,pc++);
                printlnObject(symbol,1);//T
                continue;
            }  
            case GLOBAL:{
                printf("GLOBAL\n");
                continue;
            }
            case GLOBAL_END:{
                printf("GLOBAL_END\n");
                continue;
            }
            case ENTRY:{
                printf("ENTRY     %3d\n",_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case SETQ:{
                printf("SETQ\n");
                continue;
            }
            case RET:{
                printf("RET       ");
                printf("  X\n");
                continue;
            }
            case MSUB:{
                printf("MSUB      ");
                printlnObject(Array_get(program,pc++),1);
                continue;
            }
            case MPOP:{
                printf("MPOP\n");
                continue;
            }
            case MPICK:{
                printf("MPICK      ");
                int i = _Integer_value(Array_get(program,pc++));
                int j = _Integer_value(Array_get(program,pc++));
                printf("%3d %3d\n",i,j);
                continue;
            }
            case MSET:{
                printf("MSET      ");
                printf("%3d\n",_Integer_value(Array_get(program,pc++)));
                continue;
            }
            case JUMPF:{
                printf("jumpF     ");//T
                int offset = _Integer_value(Array_get(program,pc++)); 
                printf("%3d\n",offset);//T
                continue;
            }
            case JUMP:{
                printf("jump      ");
                int offset = _Integer_value(Array_get(program,pc++)); 
                printf("%3d\n",offset);
                continue;       
            }
            case i_PRINT:{
                printf("i PRINT\n");
                continue;
            }
            case l_PRINT:{
                printf("l PRINT\n");
                continue;
            }
            case f_PRINT:{
                printf("f PRINT\n");
                continue;
            }
            case d_PRINT:{
                printf("d PRINT\n");
                continue;
            }
            case c_PRINT:{
                printf("c PRINT\n");
                continue;
            }
            case s_PRINT:{
                printf("s PRINT\n");
                continue;
            }
            case HALT:{
                printf("HALT\n");
                return nil;
            }
        }
    }
}
#endif