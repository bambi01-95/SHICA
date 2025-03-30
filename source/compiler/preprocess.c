#ifndef PREPROCESS_C
#define PREPROCESS_C

#include "object.c"
#include "binary.c"



oop preprocess(oop exp,oop trees){
    switch(getType(exp)){

        case SetVarEvent:{ // global event funciton
#if DEBUG
            DEBUG_LOG("SETVAR EVENT\n");
#endif
            oop varId = get(exp, SetVarEvent, id);insertEventTable(varId);
            oop eventhandler = get(exp, SetVarEvent, rhs);
            oop eventFuncId = get(eventhandler, Event, id);
            oop eventFunc = get(eventFuncId,Symbol,value);
            if(getType(eventFunc)!=EventFunc){
                fatal("line %d: [%s] is not Event Funciton",get(exp,SetVarEvent,line),get(eventFuncId,Symbol,name));
            }

            oop params    = get(eventhandler, Event, parameters);
            oop body      = get(eventhandler, Event, body);

            if(params==nil && body==nil){                //event e = event();
                get(varId,Symbol,value) = newDupEvent(copyEventFunc(eventFunc),nil);
            }else{                                      //event e = event(params){body};
                get(eventhandler,Event,id) = varId;
                get(varId,Symbol,value) = newDupEvent(copyEventFunc(eventFunc),eventhandler); 
            }
            return 0;
        }

        case State:{
#if DEBUG
            DEBUG_LOG("STATE\n");
#endif
            int size = get(exp,State,size);
            oop stateName = get(exp,State,id);
            oop *events = get(exp,State,events);

            oop globalEvent   = newArray(1);
            oop localEvent    = newArray(0);
            oop defLocalEvent = newArray(0);

            int eventPosIndex =0;  //point to starting postion of eventhandler

            for(int i=0;i<size; i++){
                oop statement = events[i];

                switch(getType(statement)){
                    case SetVarL:{//int a = 10;
                        eventPosIndex++;
                        break;
                    }
                    case SetVarEvent:{//event e = event();
                        eventPosIndex++;
                        oop varId = get(statement, SetVarEvent, id);
                        oop eventhandler = get(statement, SetVarEvent, rhs);
                        oop eventFuncId = get(eventhandler, Event, id);
                        oop eventFunc = get(eventFuncId,Symbol,value);
                        fatal_cond(getType(eventFunc)!=EventFunc,"line %d: %s is not Event Funciton\n", get(statement,SetVarEvent,line),get(eventFuncId,Symbol,name));
                        oop params    = get(eventhandler, Event, parameters);
                        oop body      = get(eventhandler, Event, body);
                        if(params==nil && body==nil){
                            oop dupEventFunc = copyEventFunc(eventFunc);
                            Array_push(defLocalEvent,newPair(varId,newDupEvent(dupEventFunc,nil)));
                        }else if(params==nil || body==nil){
                            fatal("line %d: definition error: %s\n",get(statement,SetVarEvent,line),get(eventFuncId,Symbol,name));
                        }else{
                            fatal("line %d: definition error: %s\n",get(statement,SetVarEvent,line),get(eventFuncId,Symbol,name));
                        }
                        break;
                    }
                    
                    case Event:{
#if DEBUG
                        DEBUG_LOG("->Event\n");
#endif
                        oop eventId = get(statement,Event,id);
                        oop eveFunc = nil;

                        if(eventId == entry_sym){
                            //FIXME: to optimize this, it should deal 'init event()' inside some scope!
                            oop body = get(statement,Event,body);
                            int size = get(body,Block,size);
                            for(int i=0;i<size;i++){
                                oop stm = get(body,Block,statements)[i];
                                if(getType(stm)==Call && get(stm,Call,callType)==1){//init eventFunc()
                                    oop functionId = get(stm,Call,function);
                                    if(getType(get(functionId,Symbol,value))==EventFunc || getType(get(functionId,Symbol,value))==DupEvent){
                                        Array_push(globalEvent,newPair(eventId,functionId));
                                    }else if(findIdFromList(functionId,defLocalEvent)==nil){
                                        fatal("%s is not defined\n",get(functionId,Symbol,name));
                                    }
                                }
                            }
                            eventPosIndex++;
                        }
                        else if(eventId == exit_sym){
#if DEBUG
                            DEBUG_ERROR("this is not happen\n");
#endif
                        }
                        else
                        { 
                            switch(getType(get(eventId,Symbol,value))){
                                case DupEvent:{
                                    insertEventTable(eventId);
                                    Array_push(globalEvent,newPair(eventId,statement));
                                    break;
                                }
                                case EventFunc:{
                                    insertEventTable(eventId);
                                    Array_push(globalEvent,newPair(eventId,statement));
                                    break;
                                }
                                default:{
                                    oop dupEve = findIdFromList(eventId,defLocalEvent); //it shluld be change: 
                                    fatal_cond(dupEve==nil,"%s is not defined\n",get(eventId,Symbol,name));
                                    Array_push(localEvent,statement);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case Call:{//Event()
#if DEBUG
                        DEBUG_LOG("->Call\n");
#endif
                        oop eventId = get(statement,Call,function);
                        oop function = get(eventId,Symbol,value);
                        fatal_cond(getType(function)!=DupEvent,"%s is not Event funciton\n",get(eventId,Symbol,name));
                        Array_push(globalEvent,newPair(eventId,statement));
                        break;
                    }
                    default:{
#if DEBUG
                        DEBUG_ERROR("line %d not apper type %s\n",__LINE__,TYPENAME[getType(statement)]);
#endif
                    }
                }
            }
            Array_push(STATE_DEF_LOCAL_EVENT_LISTS,newPair(stateName,defLocalEvent));
            globalEvent = sortEventListByStateTable(stateName,globalEvent);
            events = Array_put_elements(events,globalEvent,eventPosIndex);
            exp->State.events = Array_put_elements(events,localEvent,eventPosIndex+(globalEvent->Array.size));
/*
|definition of state local variable |
|definition of state local event    |
|definition of entry event function |
|call of event function             | globalEvent
|call of state local event function | sttLocalEvent
*/

            Array_push(trees,exp);
            break;
        }
        case Event:{

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