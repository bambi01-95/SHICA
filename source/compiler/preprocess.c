#ifndef PREPROCESS_C
#define PREPROCESS_C

#include "object.c"

oop STATE_TABLE = 0;

oop preprocess(oop exp,oop trees){
    switch(getType(exp)){

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
                get(varId,Symbol,value) = newDupEvent(dupEventFunc,nil); 
            }else if(params==nil || body==nil){
                fatal("line %d: definition error: %s\n",get(exp,SetVarEvent,line),get(eventFuncId,Symbol,name));
            }else{
                get(event,Event,id) = varId;
                get(varId,Symbol,value) = newDupEvent(copyEventFunc(eventFunc),event); 
            }
            return 0;
        }

        case State:{
            int size = get(exp,State,size);
            oop stateName = get(exp,State,id);
            oop *events = get(exp,State,events);

            oop globalEvent = nil;
            int eventIndex = 0;
            for(int i=0;i<size; i++){
                oop statement = events[i];
                switch(getType(statement)){
                    case SetVarG:{
#if DEBUG
                    DEBUG_ERROR("this is not support now\n");
#endif
                        break;
                    }
                    case SetVarL:{
                        break;
                    }
                    case SetVarEvent:{
                        break;
                    }
                    case Event:{
                        oop id = get(statement,Event,id);
                        if(getType(get(id,Symbol,value))==DupEvent){
                            globalEvent = newPair(newPair(id,_newInteger(eventIndex)),globalEvent);
                        }
                        eventIndex++;
                        break;
                    }
                    case Call:{
                        oop id = get(statement,Call,function);
                        oop function = get(id,Symbol,value);
                        if(getType(function)!=DupEvent){
                            fatal("line %d: %s is not DupEvent\n",get(id,Symbol,name));
                        }
                        globalEvent = newPair(newPair(id,_newInteger(eventIndex)),globalEvent);
                        eventIndex++;
                        break;
                    }
                    default:{
#if DEBUG
                    DEBUG_ERROR("line %d not apper type %s\n",__LINE__,TYPENAME[getType(statement)]);
#endif
                    }
                }
            }
            globalEvent = rePair(globalEvent,nil);
            Array_push(STATE_TABLE,newPair(stateName,globalEvent));
            Array_push(trees,exp);
            break;
        }
        case END:{
            Array_push(trees,exp);
            return sys_false;
        }
        default:{
            Array_push(trees,exp);
            return nil;
        }
    }
    return trees;
}


#endif // PREPROCESS_C