#ifndef PRINT_H
#define PRINT_H

#include "./object.c"
#include "../common/inst.c"
void putIndent(int indent){
    for(int i = 0;i<indent;i++){
        printf("  ");
    }
}

void printlnObject(oop node, int indent){
    
    switch (getType(node)) {
	case Undefined:	putIndent(indent);printf("nil\n");				break;
	case Integer:	putIndent(indent);printf("%s\n", get(node, Integer,number));	break;
	case Symbol :	putIndent(indent);printf("%s\n", get(node, Symbol,name));		break;
    case Float :    putIndent(indent);printf("%s\n",get(node,   Float,number));     break;
    case String :   putIndent(indent);printf("%s\n", get(node, String, value));   break;// c5
    case Key:       putIndent(indent);printf("%s\n", get(node, Key,    pass));    break;
	case Pair: {
	    putIndent(indent);printf("Pair\n");
	    printlnObject(get(node, Pair,a), indent+1);
	    printlnObject(get(node, Pair,b), indent+1);
	    break;
	}
	case Function: {
	    putIndent(indent);printf("function()\n");
	    printlnObject(get(node, Function,parameters), indent+2);
	    printlnObject(get(node, Function,body), indent+1);
	    break;
	}
    case DupEvent:{
        putIndent(indent);printf("DupEvent\n");
        oop eventFunc = get(node,DupEvent,eventFunc);
        putIndent(indent);printf("event func[%d][%d]",get(eventFunc,EventFunc,lib_num),get(eventFunc,EventFunc,eve_num) );
        putIndent(indent);printf("event of %s\n", get(node, DupEvent,event)->Event.id->Symbol.name);
        break;
    }
	case Binop: {
	    switch (get(node, Binop,op)) {
		case NE:  putIndent(indent);printf("NE\n"); break;
		case EQ:  putIndent(indent);printf("EQ\n"); break;
		case LT:  putIndent(indent);printf("LT\n"); break;
		case LE:  putIndent(indent);printf("LE\n"); break;
		case GE:  putIndent(indent);printf("GE\n"); break;
		case GT:  putIndent(indent);printf("GT\n"); break;
		case ADD: putIndent(indent);printf("ADD\n"); break;
		case SUB: putIndent(indent);printf("SUB\n"); break;
		case MUL: putIndent(indent);printf("MUL\n"); break;
		case DIV: putIndent(indent);printf("DIV\n"); break;
		case MOD: putIndent(indent);printf("MOD\n"); break;
        case BAND: putIndent(indent);printf("AND\n"); break;
        case BOR:  putIndent(indent);printf("OR\n"); break;
        case LSH:  putIndent(indent);printf("LSH\n"); break;
        case RSH:  putIndent(indent);printf("RSH\n"); break;
		default:  assert(!"this cannot happen binop");
	    }
	    printlnObject(get(node, Binop,lhs), indent+1);
	    printlnObject(get(node, Binop,rhs), indent+1);
	    break;
	}
	case Unyop: {
	    switch (get(node, Unyop,op)) {
		case NEG: putIndent(indent);printf("NEG\n"); break;
		default:  assert(!"this cannot happen unyop");
	    }
	    printlnObject(get(node, Unyop,rhs), indent+1);
	    break;
	}
	case GetVar: {
	    putIndent(indent);printf("GetVar %s\n", get(get(node, GetVar,id), Symbol,name));
	    break;
	}
	case SetVar: {
	    putIndent(indent);printf("SetVar %s\n", get(get(node, SetVar,id), Symbol,name));
	    printlnObject(get(node, SetVar,rhs), indent+1);
	    break;
	}
	case Call: {
	    putIndent(indent);printf("Call\n");
	    printlnObject(get(node, Call,function), indent+1);
	    printlnObject(get(node, Call,arguments), indent+1);
	    break;
	}
	case Print: {
	    putIndent(indent);printf("Print\n");
	    printlnObject(get(node, Print,arguments), indent+1);
	    break;
	}
	case If: {
	    putIndent(indent);printf("If\n");
	    printlnObject(get(node, If,condition), indent+1);
	    printlnObject(get(node, If,statement1), indent+1);
	    printlnObject(get(node, If,statement2), indent+1);
	    break;
	}
	case While: {
	    putIndent(indent);printf("While\n");
	    printlnObject(get(node, While,condition), indent+1);
	    printlnObject(get(node, While,statement), indent+1);
	    break;
	}
	case Block: {putIndent(indent);
	    printf("Block...\n");
	    break;
	}
    case Event:{putIndent(indent);
        printf("Event\n");
        break;
    }
    case State:{putIndent(indent);
        printf("State\n");
        break;
    }
    case Run:{putIndent(indent);
        printf("Run\n");
        break;
    }
    case Continue:{putIndent(indent);
        printf("Continue\n");
        break;
    }
    case Break:{putIndent(indent);
        printf("Break\n");
        break;
    }
    case Return:{putIndent(indent);
        printf("Return\n");
        break;
    }
    case END:{putIndent(indent);
        printf("END\n");
        break;
    }
    case Array:{putIndent(indent);
        int size = node->Array.size;
        printf("Array\n");
        for(int i = 0;i<size;i++){
            printlnObject(node->Array.elements[i],indent+1);
        }
        break;
    }
    case Assoc:{putIndent(indent);
        printf("type %2d, index %3d",node->Assoc.kind,node->Assoc.index);
        printlnObject(node->Assoc.symbol,indent);
        break;
    }
    case _Integer:putIndent(indent);printf("%d\n",_Integer_value(node));break;
    case _Long:   putIndent(indent);printf("%lld\n",get(node,_Long,value));break;
    case _Float:  putIndent(indent);printf("%f\n",_Float_value(node));break;
    case _Double: putIndent(indent);printf("%lf\n",get(node,_Double,value));break;
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

            case TRANS: {
                printf("Trans     ");//T
                oop relPosNextState  = Array_get(program,pc++);
                oop numOfNextStateEvent = Array_get(program,pc++);
                printf("%3d  %3d\n",_Integer_value(relPosNextState),_Integer_value(numOfNextStateEvent));
                continue;
            }
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
            case i_BAND: printf("i_BAND\n");continue;
            case i_BOR:  printf("i_BOR\n");continue;
            case i_LSH:  printf("i_LSH\n");continue;
            case i_RSH:  printf("i_RSH\n");continue;
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
            case c_EQ:   printf("c_EQ\n");continue;
            case c_NE:   printf("c_NE\n");continue;
            case c_LT:   printf("c_LT\n");continue;
            case c_LE:   printf("c_LE\n");continue;
            case c_GE:   printf("c_GE\n");continue;
            case c_GT:   printf("c_GT\n");continue;
            case c_ADD:  printf("c_ADD\n");continue;
            case c_SUB:  printf("c_SUB\n");continue;
            case c_MUL:  printf("c_MUL\n");continue;
            case c_DIV:  printf("c_DIV\n");continue;
            
            case s_EQ:   printf("d_EQ\n");continue; 
            case s_NE:   printf("d_NE\n");continue; 
            case s_LT:   printf("d_LT\n");continue; 
            case s_LE:   printf("d_LE\n");continue; 
            case s_GE:   printf("d_GE\n");continue; 
            case s_GT:   printf("d_GT\n");continue; 
            case s_ADD:  printf("d_ADD\n");continue;
            case MKCORE: printf("MKCORE    %3d\n",_Integer_value(Array_get(program,pc++)));continue;
            case COPYCORE:{
                printf("COPYCORE \t\t\t");//T
                oop indexOfGlobalMemory  = Array_get(program,pc++);
                oop jumpRelPos = Array_get(program,pc++);
                printf("%3d  %3d\n",_Integer_value(indexOfGlobalMemory),_Integer_value(jumpRelPos));
                continue;
            }
            case SETCORE:{
                printf("SETCORE   ");//T
                oop lib_num  = Array_get(program,pc++);
                oop func_num = Array_get(program,pc++);
                oop numInitVals = Array_get(program,pc++);
                printf("%3d  %3d  %3d\n",_Integer_value(lib_num),_Integer_value(func_num),_Integer_value(numInitVals));
                continue;
            }
            case SETSUBCORE:{
                printf("SETSUBCORE ");//T
                oop lib_num  = Array_get(program,pc++);
                oop func_num = Array_get(program,pc++);
                oop numInitVals = Array_get(program,pc++);
                printf("%3d  %3d  %3d\n",_Integer_value(lib_num),_Integer_value(func_num),_Integer_value(numInitVals));
                continue;
            }
            case MKTHREAD:{
                printf("MKTHREAD ");//T
                oop numOfThreads = Array_get(program,pc++);
                printf("%3d\n",_Integer_value(numOfThreads));
                continue;
            }
            case SETTHREAD: {
                oop actLoc =    Array_get(program,pc++);
                oop condLoc =  Array_get(program,pc++);
                printf("SETTHREAD %3d %3d\n",_Integer_value(condLoc),_Integer_value(actLoc));
                continue;
            }
            case STARTIMP: printf("STARTIMP\n");continue;
            case EOE:    printf("EOE\n");continue;
            case EOC:    printf("EOC\n");continue;
            case EOA:    printf("EOA\n");continue;
            case COND:   printf("COND\n");continue;

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
            case CALL_A:{
                printf("CALL_A     ");//T
                
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
default:{
                printf("%s line %d this is not happen \n",__FILE__,__LINE__);
            }
        }
    }
}
#endif