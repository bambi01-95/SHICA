#ifndef PREPROCESS_C
#define PREPROCESS_C

#include "object.c"

//stateごとに実行定義されているグローバルイベントリスト
oop STATE_GLOBAL_EVENT_LISTS  = 0;
//stateごとに定義されているローカルイベントリスト
oop STATE_DEF_LOCAL_EVENT_LISTS   = 0;

/*
 STATE_GLOBAL_EVENT_LISTS

    Pair
        a:stateName
        b:Pair
            event and index (PAIR)
            Pair
                event and index (PAIR)
                Pair
                    event and index (PAIR)
                    nil
    Pair
        a:bstateName
        b:Pair
            event and index (PAIR)
            Pair
                event and index (PAIR)
                Pair
                    event and index (PAIR)
                    nil
*/
oop findIdFromList(oop id,oop list){
    oop tmp = list;
    while(tmp!=nil){
        oop eventId = get(tmp,Pair,a)->Pair.a;
        if(id == eventId){
            return get(tmp,Pair,a)->Pair.b;
        }
        tmp = get(tmp,Pair,b);
    }
    return nil;
}

oop preprocess(oop exp,oop trees){
    switch(getType(exp)){

        case SetVarEvent:{
#if DEBUG
            DEBUG_LOG("SETVAR EVENT\n");
#endif
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
#if DEBUG
            DEBUG_LOG("STATE\n");
#endif
            int size = get(exp,State,size);
            oop stateName = get(exp,State,id);
            oop *events = get(exp,State,events);

            oop globalEvent = nil;
            oop sttLocalEvent = nil;
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
#if DEBUG
                        DEBUG_LOG("->SetVarL\n");
#endif
                        break;
                    }
                    case SetVarEvent:{//local event
                    #if DEBUG   
                        DEBUG_LOG("->SETVAR EVENT\n");
                    #endif
                        oop varId = get(statement, SetVarEvent, id);
                        oop event = get(statement, SetVarEvent, rhs);
                        oop eventFuncId = get(event, Event, id);
                        oop eventFunc = get(eventFuncId,Symbol,value);
                        if(getType(eventFunc)!=EventFunc){
                            fatal("line %d: [%s] is not Event Funciton",get(statement,SetVarEvent,line),get(eventFuncId,Symbol,name));
                        }
                        oop params    = get(event, Event, parameters);
                        oop body      = get(event, Event, body);
                        if(params==nil && body==nil){
                            oop dupEventFunc = copyEventFunc(eventFunc);
                            sttLocalEvent = newPair(newPair(varId,newDupEvent(dupEventFunc,nil)),sttLocalEvent);
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
                        oop id = get(statement,Event,id);
                        oop eveFunc = nil;
                        if(id == entry_sym){//need to optimize
                            oop body = get(statement,Event,body);
                            int size = get(body,Block,size);
                            for(int i=0;i<size;i++){
                                oop stm = get(body,Block,statements)[i];
                                if(getType(stm)==Call && get(stm,Call,callType)==1){//init eventFunc()
                                    oop function = get(stm,Call,function);//id
                                    globalEvent = newPair(newPair(function,_newInteger(eventIndex)),globalEvent);
                                    eventIndex++;
                                }
                            }
                        }else{
                            if(getType(get(id,Symbol,value))==DupEvent){//global event
                                oop judge = findIdFromList(id,globalEvent);
                                if(judge==nil){
                                    globalEvent = newPair(newPair(id,_newInteger(eventIndex)),globalEvent);
                                    eventIndex++;
                                }
                                eveFunc = get(get(id,Symbol,value),DupEvent,eventFunc);
                            }
                            else if(getType(get(id,Symbol,value))!=EventFunc){//local event
                            oop dupEve = findIdFromList(id,sttLocalEvent);
                            if(dupEve!=nil){
                                eveFunc = get(dupEve,DupEvent,eventFunc);
                            }
                            }else{//global event
                                oop judge = findIdFromList(id,globalEvent);
                                if(judge==nil){
                                    globalEvent = newPair(newPair(id,_newInteger(eventIndex)),globalEvent);
                                    eventIndex++;
                                }
                                eveFunc = get(id,Symbol,value);
                            }
                        }

                        break;
                    }
                    case Call:{
#if DEBUG
                        DEBUG_LOG("->Call\n");
#endif
                        oop id = get(statement,Call,function);
                        oop function = get(id,Symbol,value);
                        if(getType(function)!=DupEvent){
                            fatal("line %d: %s is not DupEvent\n",get(id,Symbol,name));
                        }
                        oop eveFunc = get(function,DupEvent,eventFunc);
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
            sttLocalEvent = rePair(sttLocalEvent,nil);
            Array_push(STATE_GLOBAL_EVENT_LISTS   ,newPair(stateName,globalEvent));
            Array_push(STATE_DEF_LOCAL_EVENT_LISTS,newPair(stateName,sttLocalEvent));
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