#ifndef PREPROCESS_C
#define PREPROCESS_C

#include "object.c"

oop preprocess(oop exp,oop trees){
    switch(getType(result)){

        case SetVarEvent:{
            oop varId = get(exp, SetVarEvent, id);
            oop event = get(exp, SetVarEvent, rhs);
            oop eventFuncId = get(event, Event, id);
            oop eventFunc = get(eventFuncId,Symbol,value);
            if(getType(eventFunc)!=EventFunc){
                fatal("line %d: [%s] is not Event Funciton",get(exp,SetVarEvent,line),get(eventFuncId,Symbol,name));
            }
            oop params    = get(event, Event, parameters);
            oop body      = get(event, Event, body);
            if(params==nil && body==nil){
                oop dupEventFunc = copyEventFunc(eventFunc);
                get(varId,Symbol,value) = dupEventFunc; 
            }else if(params==nil || body==nil){
                fatal("line %d: definition error: %s\n",get(exp,SetVarEvent,line),get(eventFuncId,Symbol,name));
            }else{
                printlnObject(varId,2);
                get(varId,Symbol,value) = newDupEvent(varId,copyEventFunc(eventFunc),event); 
            }
            return 0;
        }

        case State:{
            int size = get(exp,State,size);
            oop stateName = get(exp,State,id);
            oop *events = get(exp,State,events);

            oop globalEvent = newArray(0);
            for(int i=0;i<size; i++){
                oop statement = events[i];
                switch(getType(statement)){
                    case SetVarG:{
#if DEBUG
                    DEBUG_ERROR("this is not support now\n");
#endif
                        break;
                    }
                    case SetVarL:
                    case Event:break;
                    case Call:{
                        oop id = get(statement,Call,function);
                        oop function = get(id,Symbol,value);
                        if(getType(function)!=DupEvent){
                            fatal("line %d: %s is not DupEvent\n",get(id,Symbol,name));
                        }
                    }
                    default:{
#if DEBUG
                    DEBUG_ERROR("line %d not apper type %s\n",__LINE__,TYPENAME[getType(statement)]);
#endif
                    }
                }
            }
            
            break;
        }
        default:{
            Array_push(trees,result);
        }
    }
    return trees;
}


#endif // PREPROCESS_C