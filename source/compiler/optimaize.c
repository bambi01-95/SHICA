#include "object.c"
#include "../common/inst.c"

#ifndef OPTIMAIZE_C
#define OPTIMAIZE_C

enum instrac Binop_oprand(enum Type type,enum binop binop,int line){
    switch(type){
        case _Integer:{
            switch(binop){
                case AND: return i_AND;
                case OR: return i_OR;
                case EQ: return i_EQ;
                case NE: return i_NE;
                case LT: return i_LT;
                case LE: return i_LE;
                case GE: return i_GE;               
                case GT: return i_GT;
                case ADD:return i_ADD;
                case SUB:return i_SUB;
                case MUL:return i_MUL;
                case DIV:return i_DIV;
                case MOD:return i_MOD;
                default: printf("this cannot happen in Binop_oprand Integer\n");
            }
            break;
        }
        case _Long:{
            switch(binop){
                case AND: fatal("line %d oprand error: logic and cannot apply Long type\n",line);
                case OR:  fatal("line %d oprand error: logic or cannot apply Long type\n",line);
                case EQ: return l_EQ; 
                case NE: return l_NE; 
                case LT: return l_LT; 
                case LE: return l_LE; 
                case GE: return l_GE;                  
                case GT: return l_GT; 
                case ADD:return l_ADD; 
                case SUB:return l_SUB; 
                case MUL:return l_MUL; 
                case DIV:return l_DIV; 
                case MOD:return l_MOD; 
                default: printf("this cannot happen Binop_oprand Long\n");
            }
            break;
        }
        case _Float:{
            switch(binop){
                case AND: fatal("line %d oprand error: logic and cannot apply Float type\n",line);
                case OR:  fatal("line %d oprand error: logic or cannot apply Float type\n",line);
                case EQ:  return f_EQ;
                case NE:  return f_NE;
                case LT:  return f_LT;
                case LE:  return f_LE;
                case GE:  return f_GE;                
                case GT:  return f_GT;
                case ADD: return f_ADD;
                case SUB: return f_SUB;
                case MUL: return f_MUL;
                case DIV: return f_DIV;
                case MOD: fatal("line %d oprand error: MOD cannot apply Float type\n",line);
                default: printf("this cannot happen Binop_oprand Float\n");
            }
            break;
        }
        case _Double:{
            switch(binop){
                case AND: fatal("line %d oprand error: logic and cannot apply Double type\n",line);
                case OR:  fatal("line %d oprand error: logic or cannot apply Double type\n",line);
                case EQ: return d_EQ; 
                case NE: return d_NE; 
                case LT: return d_LT; 
                case LE: return d_LE; 
                case GE: return d_GE;                  
                case GT: return d_GT; 
                case ADD:return d_ADD;
                case SUB:return d_SUB;
                case MUL:return d_MUL;
                case DIV:return d_DIV;
                case MOD: fatal("line %d oprand error: MOD cannot apply Double type\n",line);
                default: printf("this cannot happen Binop_oprand Double\n");
            }
            break;
        }
        case String:{            //now
            switch(binop){
                case AND: fatal("line %d oprand error: logic and cannot apply String type\n",line);
                case OR:  fatal("line %d oprand error: logic or cannot apply String type\n",line);
                case EQ: return s_EQ; 
                case NE: return s_NE; 
                case LT: return s_LT; 
                case LE: return s_LE; 
                case GE: return s_GE;                  
                case GT: return s_GT; 
                case ADD:return s_ADD;
                case SUB:fatal("line %d oprand error: logic or cannot apply String type\n",line);
                case MUL:fatal("line %d oprand error: logic or cannot apply String type\n",line);
                case DIV:fatal("line %d oprand error: logic or cannot apply String type\n",line);
                case MOD: fatal("line %d oprand error: MOD cannot apply Double type\n",line);
                default: printf("this cannot happen Binop_oprand Double\n");
            }
            break;
        }
        case _Char:{
            fprintf(stderr,"not yet\n");
            exit(1);
        }
        default:{
            fprintf(stderr,"%s has not been applied to calculations\n",TYPENAME[type]);
            exit(1);
        }
    }
    fprintf(stderr,"%s has not been applied MOD\n",TYPENAME[type]);
    exit(1);
    return 0;
}

// break stack
int *b_stack = 0;
int   b_size = 0;
int   b_max  = 0;

void b_push(int i){
    // minus number is mark
    if(i>0 && b_size==0){
        fprintf(stderr,"break is defined outside loop\n");
        exit(1);
    }
    if(b_size >= b_max){
        b_stack = realloc(b_stack,sizeof(*b_stack) *( b_size + 1));
        b_max++;
    }
    b_stack[b_size++] = i;
}

int b_pop(){
    if(b_size==0){
        fprintf(stderr,"break out of stack\n");
        exit(1);
    }
    return b_stack[--b_size];
}

// continue stack
int *c_stack = 0;
int  c_size  = 0;
int  c_max   = 0;

void c_push(int i){
    // minus number is mark
    if(i>0 && c_size==0){
        fprintf(stderr,"continue is defined outside loop\n");
        exit(1);
    }
    if(c_size>=c_max){
        c_stack = realloc(c_stack,sizeof(*c_stack) *( c_size + 1));
    }
    c_stack[c_size++] = i;
}
int c_pop(){
    if(c_size==0){
        fprintf(stderr,"continue out of stack\n");
        exit(1);
    }
    return c_stack[--c_size];
}


//find key in the alist (variable name table).
oop assoc(oop key, oop alist)
{
    if (getType(alist) == Array) {
        int size = alist->Array.size;
        for(int i=0; i<size; i++){
            oop head = alist->Array.elements[i];
            if(head == nil)continue;
            if(head->Assoc.symbol == key) return head;
        }
    }
    return nil;
}

//remove assoc form end to size of vnt (variable name table)
oop kill_assoc(oop vnt,int end){
    assert(getType(vnt) == Array);
    int size = vnt->Array.size;
    for(int i=size-1; i==end;i--){
        vnt->Array.elements[i] = nil;
    }
    return nil;
}


void manage(oop program,enum Type type){
    int size  = 0;
    switch(type){
        case _Integer:
        case _Float:  program->Array.number += 3;break;
        case _Long:
        case _Double: program->Array.number += 7;break;
        default:
            fatal("manage error: unsupported type\n");
    }
    return;
}
/* array put - setnum*/
#define INTSIZE 4 
#define OPESIZE 1 

oop compile(oop,oop,oop,enum Type);
#define emit(X)         Array_push(program,X)
#define emitI(A)        emit(_newInteger(A))
#define emitII(OP,A)    emitI(OP); emitI(A); manage(program,_Integer)
#define emitOII(OP,A,B) emitI(OP); emitI(A); manage(program,_Integer); emitI(B); manage(program,_Integer); 
#define emitOIII(OP,A,B,C) emitI(OP); emitI(A); manage(program,_Integer); emitI(B); manage(program,_Integer); emitI(C); manage(program,_Integer); 
#define emitIO(OP,A,T)  emitI(OP); emit(A);  manage(program,T);
#define emitIS(OP,A,S)  emitI(OP); emit(A);  program->Array.number+=S 

#define compO(O)    compile(program,O,vnt,Undefined)
#define compOT(O,T) compile(program,O,vnt,T)
#define compPara(O,L)    compileParams(program,O,vnt,L)

#define compStatement(A,T)({ \
    if(isEntry==0){ \
        size  = vnt->Array.size; \
        compOT(A,T);/* if */ \
        kill_assoc(vnt,size); \
    }else{ \
        size  = Local_VNT->Array.size; \
        compOT(A,T);/* if */ \
        kill_assoc(Local_VNT,size);/*kill*/ \
    } \
})
oop state_Pair = 0; //state (id index)
oop Global_VNT = 0; //Gloval variable name table
oop Local_VNT  = 0; //STATE Local variable name table
int isEntry    = 0; //if defining entry() event, it is 1, otherwise 0.



void compileParams(oop program,oop params,oop vnt,int line){
    int index = 0;
    while(getType(params) == Pair){
        oop para = params->Pair.a;
        if(assoc(para->Pair.b,vnt)!=nil)
            fatal("line %d type error: cannot apply same symbol in parameter\n",line);
        if(assoc(para->Pair.b,Global_VNT)!=nil)
            fatal("line %d variable error: %s parameter in Global variable\n",line,get(para->Pair.b,Symbol,name));
        /* LOCAL VNT */           
        oop ass  = newAssoc(para->Pair.b,getType(para->Pair.a),vnt->Array.size);
        Array_push(vnt,ass);
        /* +3: [stack]
        arg_num　
        ret_pc_num　
        rbp　
        */
        //†††
        emitOII(MPICK, index + 3, ass->Assoc.index);//rbp - (2:return adress & arg num)
        params = params->Pair.b;
        index++;
    }
}



int compArgs(oop program,oop params,oop args,oop vnt){
    if(params->_type_ == Pair && args->_type_ == Pair){
        int size = compArgs(program,params->Pair.b,args->Pair.b,vnt);
        oop para = params->Pair.a; 
        compOT(args->Pair.a,para->Pair.a->_type_);
        return size + 1;
    }
    if(params->_type_ == Pair)fatal("argument error: too few argument");
    if(args->_type_ == Pair)fatal("argument error: too many argument");
    return 0;
}



//Returns the leftmost type in the tree. (condition)
int child_type(oop exp,oop vnt){
    switch (getType(exp)) {
	case Integer:return Integer;
    case Float:return   Float;
	case Binop: {
        int l = child_type(get(exp,Binop,lhs),vnt);
        switch(l){
            case _Integer:
            case _Long:
            case _Float:
            case _Char:
            case String:
            case _Double:{
                return l;
            }
            default:break;
        }

        int r = child_type(get(exp,Binop,rhs),vnt);
        switch(l){
            case _Integer:
            case _Long:
            case _Float:
            case _Char:
            case String:
            case _Double:{
                return l;
            }
            default:break;
        }
        return l;
	}
	case GetVar:{
        oop sym = get(exp,GetVar,id);
        oop ass = assoc(sym,Global_VNT);
        if(ass==nil)ass = assoc(sym,Local_VNT);
        if(ass==nil)ass = assoc(sym,vnt);
        if(ass == nil){
            fatal("line %d variable error: Undefine variable, %s\n",exp->GetVar.line,sym->Symbol.name);
        }
        return ass->Assoc.kind;
    }

	case Call:{
        oop id = exp->Call.function;
        oop function = get(id,Symbol,value);
	    switch (getType(function)) {
            case Function:{
                return  get(function,Function,kind);
            }
            case Primitive:{
                return get(function,Primitive,return_type);
            }
            default:fatal("line %d HACK: this cannot happen call\n",exp->Call.line);
	    }
	    break;
    }
	default:{
        fatal("line %d HACK: this cannot happen chiled type");
    }
    }
    return Undefined;
}





struct Variable{
    int index;
    int variable_num;
    /*
    0:normal varialble
    1:STT local variable
    2:global varialbe
    */
};

struct Variable* newVariable(int i,int t){
    struct Variable *var = calloc(1,sizeof(struct Variable));
    var->variable_num = t;
    var->index = i;
    return var;
}

/* Find variable from global, local, normal variable name table.
    and then return global/local/vnt, and index where it is stored */
struct Variable* set_id_index(oop id,oop vnt){
    oop ass = assoc(id,Global_VNT); 
    if( ass                        !=nil)return newVariable(get(ass,Assoc,index),2);
    if((ass = assoc(id, Local_VNT))!=nil)return newVariable(get(ass,Assoc,index),1);
    if((ass = assoc(id,vnt))       !=nil)return newVariable(get(ass,Assoc,index),0);
    if(ass == nil)fatal("variable error: Undefined variable, %s",get(id,Symbol,name)); 
    return 0;
}

oop *struct_symbols = 0;
int nstruct_symbols = 0;

oop addStructTable(oop id, oop child)
{
    char *name = get(id,Symbol,name);
    // binary search for existing symbol
    int lo = 0, hi = nstruct_symbols - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int cmp = strcmp(name, get(get(struct_symbols[mid], Struct,symbol),Symbol,name));
        if      (cmp < 0) hi = mid - 1;
        else if (cmp > 0) lo = mid + 1;
        else    return struct_symbols[mid];
        // if struct child is nil, define new child
    }
    struct_symbols   = realloc(struct_symbols,   sizeof(*struct_symbols)   * (nstruct_symbols + 1));
    memmove(struct_symbols + lo + 1,
	    struct_symbols + lo,
	    sizeof(*struct_symbols) * (nstruct_symbols++ - lo));
    return struct_symbols[lo] = newStruct(id,child);
}

//FOR multi Function
struct ThreadData{
    int eventLoc;
    int size;
    int condRelPos; // <イベントアクションから条件までの相対距離>/<relative distance from event action to condition>
};
struct CoreData{
    oop id;
    int size;
    struct ThreadData **threadData;
    struct CoreData *next;
};
struct CoreData *inseartCoreData(struct CoreData *core,oop id){
    while(core!=0){
        if(core->id == id){
            core->threadData = realloc(core->threadData,sizeof(struct ThreadData*)*(core->size+1));
            return core;
        }
        if(core->next == 0)break;
        core = core->next;
    }
    struct CoreData *new = calloc(1,sizeof(struct CoreData));
    new->id = id;
    new->next = 0;
    new->size = 0;
    new->threadData = (struct ThreadData**)calloc(1,sizeof(struct ThreadData*));
    core->next = new;
    return new;
}



oop compile(oop program,oop exp, oop vnt,enum Type type) //add enum Type type
{
    switch (getType(exp)) {
	case Undefined:
	case Integer:{
        switch(type){
            case _Integer:emitIO(i_load, _newCharInteger(get(exp,Integer,number)),_Integer);break;
            case _Long:   emitIO(l_load, _newCharLong(get(exp,Integer,number))   ,_Long);   break;
            case _Char:   emitIO(c_load, _newStrChar(get(exp,Integer,number))   ,_Char);   break;
            default:{
                fprintf(stderr,"line %d type error: %s but %s, ",exp->Integer.line ,TYPENAME[type],TYPENAME[getType(exp)]);
                printlnObject(exp,0);
                exit(1);
            }
        }
        break;
    }
    case Float:{
        switch(type){
            case _Float: emitIO(f_load,_newCharFloat(get(exp,Float,number)) ,_Float);break;
            case _Double:emitIO(d_load,_newCharDouble(get(exp,Float,number)),_Double);break;
            default:{
                fprintf(stderr,"line %d type error: %s but %s, ",exp->Float.line, TYPENAME[type],TYPENAME[getType(exp)]);
                printlnObject(exp,0);
                exit(1);
            }
        }
        break;
    }
    case String:{
        switch(type){
            case String:{
                int str_size = strlen(get(exp,String,value)) ;
                if(str_size > 255)fatal("line x type error: over 255 charactor \n");
                emitIS(s_load,exp,str_size);
                break;
            }
            default:{
                fatal("line x type error: %s but %s",TYPENAME[type],TYPENAME[getType(exp)]);
            }
        }
        break;
    } 
    case _Char:{
        switch(type){
            case _Char:   emitIO(c_load, exp   ,_Char);   break;
            default:{
                fatal("line x type error: %s but %s",TYPENAME[type],TYPENAME[getType(exp)]);
                printlnObject(exp,0);
                exit(1);
            }
        }
    } 
    case Key:{
        fprintf(stderr,"key and string does not allowed now\n");
        exit(1);
        break;
    }
	case Symbol:    break;
	case Pair:	    break;
	case Function:	break;
	case Binop: {
	    compOT(get(exp, Binop,lhs),type);
	    compOT(get(exp, Binop,rhs),type);
        emitI(Binop_oprand(type,get(exp,Binop,op),exp->Binop.line));
        break;
	}
	case Unyop:{
        compOT(get(exp, Unyop,rhs), type);
	    switch (get(exp, Unyop,op)){
            case NEG:{
                switch(type){
                    case _Integer: emitII(i_load,-1)           ;emitI(i_MUL);break;
                    case _Long:    emitIO(l_load,_newLong(-1)  ,_Long)  ;emitI(l_MUL);break;
                    case _Float:   emitIO(f_load,_newFloat(-1) ,_Float) ;emitI(f_MUL);break;
                    case _Double:  emitIO(d_load,_newDouble(-1),_Double);emitI(d_MUL);break;
                    default:fatal("line %d type error: %s type cannnot apply convert negative value\n",exp->Unyop.line,TYPENAME[type]);
                }
                break;
            }
            case BINC:{
                oop id = get(get(exp,Unyop,rhs),GetVar,id); 
                struct Variable *var =  set_id_index(id,vnt);
                switch(type){
                    case _Integer: emitII(i_load,1)          ;emitI(i_ADD);break;
                    case _Long:    emitIO(l_load,_newLong(1),_Long);emitI(l_ADD);break;
                    default: fatal("line %d type error: %s type cannot apply prefix increment, %s\n",exp->Unyop.line,TYPENAME[type],get(id,Symbol,name));
                }
                emitII(DEFINE+var->variable_num,var->index);emitII(GET+var->variable_num,var->index);
                break;
            }
            
            case BDEC:{
                oop id = get(get(exp,Unyop,rhs),GetVar,id); 
                struct Variable *var = set_id_index(id,vnt);
                switch(type){
                    case _Integer: emitII(i_load,1)          ;emitI(i_SUB);break;
                    case _Long:    emitIO(l_load,_newLong(1),_Long);emitI(l_SUB);break;
                    default: fatal("line %d type error: %s type cannot apply prefix decrement, %s\n",exp->Unyop.line,TYPENAME[type],get(id,Symbol,name));
                }
                emitII(DEFINE+var->variable_num,var->index);emitII(GET+var->variable_num,var->index);
                break;
            }
            case AINC:{
                oop id = get(get(exp,Unyop,rhs),GetVar,id); 
                struct Variable * var = set_id_index(id,vnt);
                emitII(GET+var->variable_num,var->index);
                switch(type){
                    case _Integer: emitII(i_load,         1) ;emitI(i_ADD);break;
                    case _Long:    emitIO(l_load,_newLong(1),_Long) ;emitI(l_ADD);break;
                    default: fatal("line %d type error: %s type cannot apply postfix increment, %s\n",exp->Unyop.line,TYPENAME[type],get(id,Symbol,name));
                }
                emitII(DEFINE+var->variable_num,var->index);
                break;
            }
            case ADEC:{
                oop id = get(get(exp,Unyop,rhs),GetVar,id); 
                struct Variable * var = set_id_index(id,vnt);
                emitII(GET+var->variable_num,var->index);
                switch(type){
                    case _Integer: emitII(i_load,         1) ;emitI(i_SUB);break;
                    case _Long:    emitIO(l_load,_newLong(1),_Long);emitI(l_SUB);break;
                    default: fatal("line %d type error: %s type cannot apply postfix decrement, %s\n",exp->Unyop.line,TYPENAME[type],get(id,Symbol,name));
                }
                emitII(DEFINE+var->variable_num,var->index);
                break;
            }
		    default:	assert(!"this cannot happen YNOP");
	    }
	    break;
    }

	case SetVar:{
        //int a = 10 or int add(int a,int b){return a + b} or state default{ event ... }
        int t     = get(exp,SetVar,typeset); //setting type of symbolfdfdfd
        oop id    = get(exp,SetVar,id);      //get symbol
        oop value = get(exp,SetVar,rhs);     //get value, = 10 or (int a,int b){return a + b}
        switch(getType(get(id,Symbol,value))){
            case Undefined:
            case _Integer:
            case _Float:
            case _Long:
            case _Double:break;
            case Function:{
                fatal("line %d definition error: function %s is already defined\n",exp->SetVar.line,get(id,Symbol,name));
                break;
            }
            case State:{
                fatal("line %d definition error: state %s is already defined\n",exp->SetVar.line,get(id,Symbol,name));
                break;
            }
            default:fatal("HACK SetVar");
        }
        switch(getType(value)){
            case Function:{
                get(value,Function,kind) = t;
                get(id,Symbol,value) = value;
                emitII(JUMP,0);             //First, when the code is loaded, the function is ignored (JUMP).
                vnt = newArray(0);          //Create symbol name table
                int jump = program->Array.number;
                int jump_i = program->Array.size;
                //CALL num: num is this number
                get(value,Function,position) = program->Array.number;
            emitII(MSUB,0);                 //rbp for variable
            int msub_loc = program->Array.size;
                oop para = get(value,Function,parameters);
                compPara(para,exp->SetVar.line);
                // vnt = args! check them!
                compOT(get(value,Function,body),t);
            int vnt_size = vnt->Array.size;
            Array_put(program,msub_loc  - 1, _newInteger(vnt_size));//change MSUB num
                int end  = program->Array.number;
                Array_put(program,jump_i - 1, _newInteger(end-jump));//change jump num
                break;
            }
        //STATE
            case State:{
                compile(program,value,vnt,t);
                get(id,Symbol,value) = value;
                break;
            }
            default:{
                if(isEntry==0){
                    oop ass = assoc(id,Global_VNT);
                    if(ass!=nil){
                        if(t!=Undefined)fatal("line %d variable error: %s is defined in Global variable\n",exp->SetVar.line,get(id,Symbol,name));
                        compOT(value,ass->Assoc.kind);
                        emitII(DEFINE_G,ass->Assoc.index); 
                    }
                    else if((ass = assoc(id, Local_VNT))!=nil){
                        if(t!=Undefined)fatal("line %d variable error: %s is defined in Local variable\n",exp->SetVar.line,get(id,Symbol,name));
                        compOT(value,ass->Assoc.kind);
                        emitII(DEFINE_L,ass->Assoc.index); 
                    }
                    else{
                        ass = assoc(id,vnt);
                        if(ass == nil){
                            if(t==Undefined)//it is first time defining this symbol, indicate type.
                                fatal("line %d variable error: Undefined variable %s\n",exp->SetVar.line,get(exp, SetVar,id)->Symbol.name);
                            //define new symbol with type
                            ass = newAssoc(get(exp, SetVar,id),t,vnt->Array.size);
                            Array_push(vnt, ass);
                        }
                        else if(t!=Undefined){
                            fatal("line %d variable error:    %s\n",exp->SetVar.line,get(exp, SetVar,id)->Symbol.name);
                        }
                        compOT(value,ass->Assoc.kind);
                        emitII(DEFINE,ass->Assoc.index); 
                    }
                }
            //FIXME with (2025/01/04): sttローカルがうまく実行できたなら下記を消す  
                //else{ in the entry(){...}
                //     oop ass = assoc(id,Global_VNT);
                //     if(ass!=nil){
                //         if(t!=Undefined)fatal("line %d variable error: %s is defined in Global variable\n",exp->SetVar.line,get(id,Symbol,name));
                //         compOT(value,ass->Assoc.kind);
                //         emitII(DEFINE_G,ass->Assoc.index); 
                //     }
                //     else{
                //         ass = assoc(id, Local_VNT);
                //         if(ass == nil){
                //             if(t==Undefined)//it is first time defining this symbol, indicate type.
                //                 fatal("line %d variable error: Undefined variable %s\n",exp->SetVar.line,get(exp, SetVar,id)->Symbol.name);
                //             //define new symbol with type
                //             ass = newAssoc(get(exp, SetVar,id),t,Local_VNT->Array.size);
                //             Array_push(Local_VNT, ass);
                //         }
                //         else if(t!=Undefined){
                //             fatal("line %d variable error:    %s\n",exp->SetVar.line,get(exp, SetVar,id)->Symbol.name);
                //         }
                //         compOT(value,ass->Assoc.kind);
                //         emitII(DEFINE_L,ass->Assoc.index); 
                //     }
                //}
            //end of remove
            }
        }
	    break;
    }
    case SetVarL:{
        
        int t     = get(exp,SetVar,typeset); //setting type of symbolfdfdfd
        oop id    = get(exp,SetVar,id);      //get symbol
        oop value = get(exp,SetVar,rhs);     //get value, = 10 or (int a,int b){return a + b}

        oop ass = assoc(id,Global_VNT);
        if(ass!=nil){
            if(t!=Undefined)fatal("line %d variable error: %s is defined in Global variable\n",exp->SetVar.line,get(id,Symbol,name));
            compOT(value,ass->Assoc.kind);
            emitII(DEFINE_G,ass->Assoc.index); 
        }
        else{
            ass = assoc(id, Local_VNT);
            if(ass == nil){
                if(t==Undefined)//it is first time defining this symbol, indicate type.
                    fatal("line %d variable error: Undefined variable %s\n",exp->SetVar.line,get(exp, SetVar,id)->Symbol.name);
                //define new symbol with type
                ass = newAssoc(get(exp, SetVar,id),t,Local_VNT->Array.size);
                Array_push(Local_VNT, ass);
            }
            else if(t!=Undefined){
                fatal("line %d variable error:    %s\n",exp->SetVar.line,get(exp, SetVar,id)->Symbol.name);
            }
            compOT(value,ass->Assoc.kind);
            emitII(DEFINE_L,ass->Assoc.index); 
        }
        
        #if DEBUG
        //FIXME with (2025/01/04): sttローカルがうまく実行できたなら、SetVarのFIXMEを消す
        SHICA_PRINTF("SetVarL\n is it work?");
        #endif
        break;
    }

    case SetVarG:{
        emitI(GLOBAL);
        int t     = get(exp,SetVarG,typeset); //setting type of symbol
        oop id    = get(exp,SetVarG,id);      //get symbol
        oop value = get(exp,SetVarG,rhs);     //get value, = 10 or (int a,int b){return a + b}
        oop ass = assoc(get(exp, SetVarG,id),Global_VNT);
        if(ass == nil){
            if(t==Undefined){//it is first time defining this symbol, indicate type.
                fatal("line %d variable error: Undefined variable %s\n",exp->SetVarG.line,get(exp, SetVarG,id)->Symbol.name);
            }
            //define new symbol with type
            ass = newAssoc(get(exp, SetVarG,id),t,Global_VNT->Array.size);
            Array_push(Global_VNT, ass);
        }
        t = ass->Assoc.kind;
        compOT(value,t);
        emitII(DEFINE_G,ass->Assoc.index);   
        emitI(GLOBAL_END);      
        break;
    }

	case GetVar:{
        oop id = get(exp,GetVar,id);
        oop ass = assoc(id,Global_VNT);
        if(ass!=nil){
            if(ass->Assoc.kind != type)//The requested type and symbol do not match
                fatal("line %d type error: requared [%s] type but [%s] type, global variable %s\n",exp->GetVar.line,TYPENAME[type],TYPENAME[ass->Assoc.kind],id->Symbol.name);
            emitII(GET_G,ass->Assoc.index);
        }
        else if((ass=assoc(id,Local_VNT))!=nil){
            if(ass->Assoc.kind!=type)
                fatal("line %d type error: requared %s type but %s type, local variable %s",exp->GetVar.line,TYPENAME[type],TYPENAME[ass->Assoc.kind],id->Symbol.name);
            emitII(GET_L,ass->Assoc.index);
        }
        else if((ass = assoc(id,vnt))!=nil){
            if(ass->Assoc.kind!=type)
                fatal("line %d type error: requared %s type but %s type, nomal variable %s",exp->GetVar.line,TYPENAME[type],TYPENAME[ass->Assoc.kind],id->Symbol.name);
            emitII(GET,ass->Assoc.index);
        }
        else{     //sym is not define
                fatal("line %d variable error: Undefine variable, %s\n",exp->GetVar.line,id->Symbol.name);
        }
        break;
    }
    // in progress
    case SetArray:{
        int t =      get(exp,SetArray,typeset);
        oop id =     get(exp,SetArray,array);
        oop values = get(exp,SetArray,value);
        oop index =  get(exp,SetArray,index);
        oop ass   = nil;
        printf("gt %2d\n",getType(Global_VNT));
        printf("lt %2d\n",getType(Local_VNT));
        printf("t  %2d\n",getType(vnt));
        if((ass = assoc(id,Global_VNT))==nil){
            if((ass = assoc(id,Local_VNT))==nil){
                if((ass = assoc(id,vnt))==nil){
                    if(t!=Undefined){
                        //in progress
                        user_error(getType(values)!=Block,"definitnion error: list\n",get(exp,SetArray,line));
                        oop value = newList_d1(_IntegerArray);
                        get(id,Symbol,value) = value;
                        ass = newAssoc(id,t,vnt->Array.size);
                        Array_push(vnt, ass);
                        int size = get(values,Block,size);
                        for(int i=0;i<size;i++){
                            compOT(values->Block.statements[i],t);
                        }
                        compOT(index,_Integer);
                        emitII(il_load,ass->Assoc.index);
                        return exp;
                    }else{
                        user_error(1,"definition error: Undefined variable\n",get(exp,SetArray,line));
                    }
                }
            }
        }
        //　要素の変更
        if(t==Undefined){
            user_error(getType(values)==Block,"definition error: not allow muti values",get(exp,SetArray,line));
            switch(get(ass,Assoc,kind)){
                case _Integer:{
                    compOT(values,ass->Assoc.kind);
                    compOT(index,ass->Assoc.kind);
                    emitII(DEFINE_List,ass->Assoc.index);
                    break;
                }
                case _Long:
                case _Double:
                case _Float:
                default:{
                    fprintf(stderr,"line %d: type error: not match\n",get(exp,SetArray,line));
                    exit(1);
                }
            }
        }else{
            fprintf(stderr,"line %d definition error: defined variable\n",get(exp,SetArray,line));
            exit(1);
        }
        break;
    }
    case SetType:{
        oop id = get(exp, SetType,id);
        oop child = get(exp,SetType,child);
        addStructTable(id,child);
        break;
    }

    case GetArray:{
        int t =     get(exp,SetArray,typeset);
        oop id =    get(exp,SetArray,array);
        oop value = get(exp,SetArray,value);
        oop index = get(exp,SetArray,index);
        printf("%s line %d: GetArray\n",__FILE__,__LINE__);
        break;
    }

	case Call:{
        oop id = exp->Call.function;
        oop function = get(id,Symbol,value);
	    switch (getType(function)){
            case Function:{
                int t = get(function,Function,kind);
                if(type!=Undefined && type!=t){
                    fatal("line %d type error: %s type but %s type, function %s\n",exp->Call.line,TYPENAME[type],TYPENAME[t],id->Symbol.name);
                }
                oop args   = get(exp,Call,arguments);
                oop params = get(function, Function,parameters);
                int num = compArgs(program,params,args,vnt);//leg11
                int i = program->Array.number;
                            /*number of args*/ /*function location*/
                emitOII(CALL,num, get(function,Function,position) - i -(OPESIZE + INTSIZE*2));
                break;
            }
            case Primitive:{
                char t       = get(function,Primitive,return_type);
                if(type!=Undefined && type!=t){
                    fatal("line %d type error: %s type but %s type, function %s\n",exp->Call.line,TYPENAME[type],TYPENAME[t],id->Symbol.name);
                }
                char* para_t = get(function,Primitive,args_type_array);
                char para_s  = get(function,Primitive,size_of_args_type_array);
                oop   args   = get(exp,Call,arguments);
                for(int i=0; i<para_s ;i++){
                    if(args == nil){
                        fprintf(stderr,"error: PRIMITVE\n");
                        exit(1);
                    }
                    compOT(args->Pair.a, para_t[i]);
                    args = args->Pair.b;
                }
                emitOIII(CALL_P,get(function,Primitive, lib_num),get(function,Primitive,func_num),para_s);
                break;
            }
            case EventFunc:{
                // set pin or trigger conditino value.
                // in this here, not compile anything.
                if(isEntry!=1){
                    fprintf(stderr,"Definition Error: Event functions cannot be initialized except by the setup event function\n ");
                    exit(1);
                }
                oop args = get(exp,Call,arguments);
                char size = get(function,EventFunc,size_of_pin_num);
                for(int i=0; i< size;i++){//#
                    assert(getType(args->Pair.a)==Integer);
                    function->EventFunc.pin_num[i] =  atoi(args->Pair.a->Integer.number);
                    args = args->Pair.b;
                }
                break;
            }
            default:printlnObject(function,2);fatal("line %d HACK: this cannot happen CALL %s\n",exp->Call.line,TYPENAME[getType(function)]);
	    }
        
	    break;
    }

	case Print:{
        compOT(get(exp, Print,argument),get(exp,Print,kind));
        //TODO: make four type of print
        int k_type = get(exp,Print,kind);
        switch(k_type){
            case _Integer:emitI(i_PRINT);break;
            case _Long   :emitI(l_PRINT);break;
            case _Float  :emitI(f_PRINT);break;
            case _Double :emitI(d_PRINT);break;
            case String  :emitI(s_PRINT);break;
            case _Char   :emitI(c_PRINT);break;
        }
        break;
    }

	case If:{// ENTRY...
        int c_type = child_type(get(exp, If,condition),vnt);
        switch(c_type){
            case Integer: c_type = _Integer;break;
            case Float:   c_type = _Float;  break;
            default:break;
        }
        compOT(get(exp, If,condition),c_type);
        emitII(JUMPF,0);//->done or else
        int jump1 = program->Array.number;
        int jump1_i = program->Array.size;
        int size =0;
        compStatement(get(exp,If,statement1),type);
        // size  = vnt->Array.size;//kill
        // compOT(get(exp, If,statement1),type);/* if */
        // kill_assoc(vnt,size);//kill
        oop stmt2 = get(exp,If,statement2);
        //else
        if(stmt2 != sys_false){
            emitII(JUMP,0);//->done
            int jump2 = program->Array.number;
            int jump2_i = program->Array.size;
            
            int p_else = jump2; /* else */

            compStatement(stmt2,type);
            // size = vnt->Array.size;//kill
            // compOT(stmt2,type);
            // kill_assoc(vnt,size);//kill


            int p_done = program->Array.number;/* done */
            Array_put(program, jump1_i- 1, _newInteger(p_else - jump1));/* jumpf -> else */
            Array_put(program, jump2_i- 1, _newInteger(p_done - jump2));/* jump  -> done */
        }
        else{// not else
            Array_put(program, jump1_i- 1, _newInteger(program->Array.number - jump1));/* jumpf -> else */
        }
        break;
    }

	case While:{
        b_push(-1);//MEMO: negative value is mark for break
        c_push(-1);//MEMO: negative value is mark for continue
        int L1 = program->Array.number;/*cond*/
        int size = 0;
        compStatement(get(exp,While,condition),_Integer);
        // size = vnt->Array.size;//kill
        // compOT(get(exp, While,condition),_Integer);
        // kill_assoc(vnt,size);

        emitII(JUMPF,0);
        int L2 = program->Array.number;
        int L2_i = program->Array.size;
        

        compStatement(get(exp,While,statement),type);
        // size = vnt->Array.size;//kill
        // compOT(get(exp, While,statement),type);//err
        // kill_assoc(vnt,size);
        


        emitII(JUMP, 0);
        int L4 = program->Array.number;/*done*/
        int L4_i = program->Array.size;/*done*/
        Array_put(program,L2_i - 1,_newInteger(L4-L2));/* jumpf -> done */
        Array_put(program,L4_i - 1,_newInteger(L1-L4)); /* jump  -> cond */

        for(;;){
            int i = c_pop();
            if(i<0)break;
            Array_put(program,c_pop() - 1,_newInteger(L1 - i));
        }
        for(;;){
            int i = b_pop();
            if(i<0)break;
            Array_put(program,b_pop() - 1,_newInteger(L4 - i));
        }
        break;
    }

    case For:{//for(initstate,condition,updata){ statement }
        b_push(-1);//MEMO: negative value is mark for break
        c_push(-1);//MEMO: negative value is mark for continue
        int size = 0;
        if(isEntry==0) size  = vnt->Array.size; 
        else           size  = Local_VNT->Array.size; 
        
        if(get(exp,For,initstate)!=nil)
            compO(get(exp,For,initstate));//for(int i = 0,...)
        
        emitII(JUMP,0);//jump update
        int L1   = program->Array.number;
        int L1_s = program->Array.size;
        if(get(exp,For,update)!=nil)
            compOT(get(exp,For,update),Undefined);
        int L2 = program->Array.number;
        int L2_s = program->Array.size;
        if(get(exp,For,condition)!=nil)
            compOT(get(exp,For,condition),_Integer);
        else{emitII(i_load,1);}
        emitII(JUMPF,0);
        int jumpf = program->Array.number;
        int jumpf_s = program->Array.size;

        compOT(get(exp,For,statement),type);
        emitII(JUMP,0);
        int done    = program->Array.number;
        int done_s  = program->Array.size;

        if(isEntry==0)  kill_assoc(vnt,size); 
        else            kill_assoc(Local_VNT,size);
        
    Array_put(program,L1_s -1,_newInteger(L2 - L1));/* jump first update */
    Array_put(program,done_s -1,_newInteger(L1 - done));/*done -> update */
    Array_put(program,jumpf_s-1,_newInteger(done - jumpf)); /* jumf  -> done */
    kill_assoc(vnt,size);//kill env

        for(;;){
            int i = c_pop();
            if(i<0)break;
            Array_put(program,c_pop() - 1,_newInteger(L1 - i));
        }
        for(;;){
            int i = b_pop();
            if(i<0)break;
            Array_put(program,b_pop() - 1,_newInteger(done - i));
        }
        break;
        break;
    }

    case Break:{
        emitII(JUMP,0);
        b_push(program->Array.size);
        b_push(program->Array.number);
        break;
    }
    case Continue:{
        emitII(JUMP,0);
        c_push(program->Array.size);
        c_push(program->Array.number);
        break;
    }
    case Return:{
        compOT(get(exp,Return,value),type);
        emitI(RET);
        break;
    }

	case Block:{
        oop *statements = get(exp, Block,statements);
	    int  size       = get(exp, Block,size);
	    for (int i = 0;  i < size;  ++i){
            compOT(statements[i],type);
        }    
	    break;
    }

    case Event:{
        printf("%s line %d: this is not happen, check case state:\n",__FILE__,__LINE__);
        break;
    }

    case State:{//from setVar
        int size = get(exp,State,size);
        oop *events = get(exp,State,events);


        struct CoreData *core = (struct CoreData *)malloc(sizeof(struct CoreData));
        core->size = 0;
        core->id = entry_sym;
        core->next = 0;

        char Entry_bool = 0; //for ...
        unsigned char stt_val_c = 0;  //for state variable 


    //<1.変数とイベントアクションの定義>/<Definition of variables and event actions> 
        for(int i=0;i<size; i++){
            switch(getType(events[i])){
                case SetArray:
                case SetVarL:{
                    stt_val_c++;
                    compO(events[i]);
                    break;
                }
                case Event:{
                    
                        emitII(JUMP,0);
                    int jump_i = program->Array.size;
                    int jump = program->Array.number;
                    //init stt local variable
                    vnt = newArray(0);
                    int m_loc = program->Array.size;

                    oop id    = get(events[i], Event, id);
                    oop para  = get(events[i], Event, parameters);
                    oop block = get(events[i], Event, body);

                    struct CoreData* coreData = inseartCoreData(core,id);
                    struct ThreadData *threadData = (struct ThreadData *)malloc(sizeof(struct ThreadData));

                    if(id==entry_sym){
                        if(i-stt_val_c==0){//IF entry() is defined at first element of state
                            Entry_bool = 1;
                            isEntry = 1;
                        }
                        else{
                            fprintf(stderr,"entry event should be defined first...\n");
                            exit(1);
                        }
                    }
                    else{
                        oop eve   = get(id,Symbol, value);
                        int args_s =  eve->EventFunc.size_of_args_type_array;
                        char *args =  eve->EventFunc.args_type_array;

                        threadData->condRelPos = 0;
                        threadData->size       = args_s;


                        if(args_s==0 && para!=nil){
                            DEBUG_ERROR("%s line %d over parameter\n",__FILE__,__LINE__);
                        }
                        for(int j=0;j<args_s;j++){
                            if(para==nil){fprintf(stderr,"event fuction args less parameter\n");exit(1);}
                            oop a = get(para,Pair,a);
                            getType(a->EventParam.type);
                            if(args[j]!=getType(a->EventParam.type)){
                                fprintf(stderr,"event fuction args[%s]!=para[%s] error\n",TYPENAME[args[j]],TYPENAME[getType(a->EventParam.type)]);
                                exit(1);
                            }
                            if(assoc(a->EventParam.symbol,vnt)!=nil)
                                fatal("type error: cannot apply same symbol in parameter\n");
                            if(assoc(a->EventParam.symbol,Global_VNT)!=nil)
                                fatal("variable error: %s parameter in Global variable\n",get(para->Pair.b,Symbol,name));
                            
                        //<引数の追加>/<add args>    
                            oop ass  = newAssoc(a->EventParam.symbol,args[j],vnt->Array.size);
                            Array_push(vnt,ass);

                        //<Event条件のコンパイル>/<compile event condition>
                            oop cond = get(a,EventParam,cond);
                            if(cond!=nil){
                                if(threadData->condRelPos == 0){
                                    threadData->condRelPos = program->Array.number;
                                }
                                //REMOVE ME: 3line
                                // oop cond_vnt = newArray(1);
                                // Array_push(cond_vnt,newAssoc(a->EventParam.symbol,args[j],cond_vnt->Array.size));//FIXME: coond_vnt->Array.size == 0|1, maybe 0
                                //threadData->condLocs[j] = program->Array.number;
                                compile(program,cond,vnt,_Integer);
                                emitI(COND);
                            }
                            para = para->Pair.b;//move to next param
                        }//end of param
                        emitI(EOC);
                    }

                    //define Event Action
                    threadData->eventLoc = program->Array.number;
                    compO(block);
                    //2 line: make a space for varialbe
                    int m_size = vnt->Array.size;
                    Array_put(program,m_loc -1, _newInteger(m_size));
                    vnt = nil;
                    isEntry = 0;//entry() is not in the entry(){} block
                        emitI(MPOP);
                        emitI(EOE);
                    Array_put(program,jump_i -1,_newInteger(program->Array.number - jump));// jump event 
                    isEntry = 0;
                    coreData->threadData[coreData->size] = threadData;
                    coreData->size++;
                    break;
                }
                default:fatal("line %d not apper\n",__LINE__);
            }
        }//end of definition of state variable and event 

    // <2.状態遷移の呼び出し>/<state transition call>
        int stt_loc = program->Array.number;
        exp->State.index = stt_loc;

        if(Entry_bool==1){//entry()の実行
            emitII(ENTRY,core->threadData[0]->eventLoc - (stt_loc));
        }

        struct CoreData *tmp = core;
        unsigned char num_of_core = 0;
        while(tmp!=0){
            if(tmp->id!=entry_sym){
                num_of_core++;
            }
            tmp = tmp->next;
        }

        emitII(MKCORE,num_of_core);
        // emitII(THREAD,size - Entry_bool - stt_val_c);
        stt_loc = program->Array.number;

    // <3.イベント関数の呼び出し>/<Event function call>
        while(core!=0){
            if(core->id==entry_sym){
                core = core->next;
                continue;
            }
            oop eveF = get(core->id,Symbol,value);
            //ILOAD Ii: event trigger initial value, pin, ip address, etc.
            for(int pin_i=0;pin_i<eveF->EventFunc.size_of_pin_num;pin_i++){
                emitII(i_load,eveF->EventFunc.pin_num[pin_i]);
            }
            //MKTHREAD LN EN TN: library number, event number, size of thread
            emitOIII(MKTHREAD,eveF->EventFunc.lib_num,eveF->EventFunc.eve_num,core->size);

            for(int i=0;i<core->size;i++){
                struct ThreadData *threadData = core->threadData[i];
                //ILOAD Ci: event condition location
                // for(int j=0;j<threadData->size;j++){
                    
                //     if(threadData->condLocs[j]!=0){
                //         emitII(i_load,threadData->condLocs[j]  - threadData->eventLoc);        //event condition location
                //     }else{
                //         emitII(i_load,0);
                //     }
                // }
                //SETTHREAD: set thread
                DEBUG_LOG("\nstt_loc %d\neventLoc %d\n condRelPos %d\n",stt_loc,threadData->eventLoc,threadData->condRelPos);
                if(threadData->condRelPos!=0){
                    emitOII(SETTHREAD, threadData->eventLoc - stt_loc, threadData->condRelPos - threadData->eventLoc );
                }else{
                    emitOII(SETTHREAD, threadData->eventLoc - stt_loc, 0 );
                }
            }
            core = core->next;
        }
        emitI(STARTIMP);
        //ローカル変数の初期化
        Local_VNT = newArray(0);
        break;
    }//end of case State

    case Run:{
        oop id = get(exp,Run,state);
        emitII(TRANS,0);
        int L = program->Array.number;
        int L_i = program->Array.size;

        state_Pair = newPair(newPair(id,newPair(_newInteger(L_i),_newInteger(L))),state_Pair);
        break;
    }

    case END:{
        while(state_Pair->_type_ == Pair){
            oop node = state_Pair->Pair.a;
            oop id = node->Pair.a;//state that is called
            int to_index = id->Symbol.value->State.index;//†††
            oop index = node->Pair.b;//index that state call
            int from_index_i = _Integer_value(index->Pair.a);
            int from_index = _Integer_value(index->Pair.b);
            //†††
            Array_put(program,from_index_i - 1,_newInteger(to_index - from_index));
            state_Pair = state_Pair->Pair.b;
        }
        return sys_false;
    }

	default:{
        printf("%s\n",TYPENAME[exp->_type_]);
        assert(!"this cannot happen compile");
    }
    
    }
    return exp;
}


#endif