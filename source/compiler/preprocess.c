#ifndef PREPROCESS_C
#define PREPROCESS_C

#include "object.c"

//stateごとに実行定義されているグローバルイベントリスト
oop STATE_EVENT_LISTS  = 0;
//stateごとに定義されているローカルイベントリスト
oop STATE_DEF_LOCAL_EVENT_LISTS   = 0;

oop EVENT_TABLE[32];      //use for colloect event id
int sizeOfEventTable = 0;

int insertEventTable(oop id){
    for(int i=0;i<sizeOfEventTable;i++){
        if(EVENT_TABLE[i]==id){
            return i;
        }
    }
    if(sizeOfEventTable==32){
        fatal("event table is full\n");
        return -1;
    }
    EVENT_TABLE[sizeOfEventTable] = id;
    return sizeOfEventTable++;
}

//NEED TO BE MORE SMART
struct State_Event_Binary_Data{
    oop stateName;
    int eventBinarylist;
}*STATE_EVENT_BINARY_DATA;

int sizeOfStateEventBinaryData = 0;
int insertEventBinaryData(oop stateName,int eventbinarylist){
    STATE_EVENT_BINARY_DATA = realloc(STATE_EVENT_BINARY_DATA,sizeof(struct State_Event_Binary_Data)*(sizeOfStateEventBinaryData+1));
    STATE_EVENT_BINARY_DATA[sizeOfStateEventBinaryData].stateName = stateName;
    STATE_EVENT_BINARY_DATA[sizeOfStateEventBinaryData].eventBinarylist = eventbinarylist;
    return sizeOfStateEventBinaryData++;
}

oop sortEventListByStateTable(oop stateName,oop eventList){
    oop new = newArray(get(eventList,Array,size));
    oop *ele = get(eventList,Array,elements);
    int eventBinaryData = 0;
    for(int i=0;i<sizeOfEventTable;i++){
        for(int j=0;j<get(eventList,Array,size);j++){
            oop id = get(ele[j],Pair,a);
            if(EVENT_TABLE[i]==id){
                Array_push(new,get(eventList,Array,elements)[j]);
                eventBinaryData |= (1<<i);
            }
        }
    }
    free(eventList);
    insertEventBinaryData(stateName,eventBinaryData);
    return new;
}



#if DEBUG
void printBinary(int n){
    for(int i=0;i<32;i++){
        if(n & (1<<i)){
            printf("1");
        }else{
            printf("0");
        }
    }
    printf("\n");
}
void printEventTable(){
    for(int i=0;i<sizeOfEventTable;i++){
        printf("%s\n",get(EVENT_TABLE[i],Symbol,name));
    }
}
void printEventBinaryData(){
    printEventTable();
    for(int i=0;i<sizeOfStateEventBinaryData;i++){
        printf("%s\n",get(STATE_EVENT_BINARY_DATA[i].stateName,Symbol,name));
        printBinary(STATE_EVENT_BINARY_DATA[i].eventBinarylist);
    }
}
#endif


oop findIdFromList(oop id,oop list){
    oop *tmp = list->Array.elements;
    int size = list->Array.size;
    for(int i=0;i<size;i++){
        oop pair = tmp[i];
        oop eventId = get(pair,Pair,a)->Pair.a;
        if(id == eventId){
            return get(pair,Pair,a)->Pair.b;
        }
    }
    return nil;
}

oop preprocess(oop exp,oop trees){
    switch(getType(exp)){

        case SetVarEvent:{
#if DEBUG
            DEBUG_LOG("SETVAR EVENT\n");
#endif
            oop varId = get(exp, SetVarEvent, id);insertEventTable(varId);
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

            oop globalEvent = newArray(1);
            oop sttLocalEvent = nil;
            int eventIndex = 0;
            int eventPosIndex =0; 
            for(int i=0;i<size; i++){
                oop statement = events[i];
                switch(getType(statement)){
                    case SetVarL:{
                        eventPosIndex++;
#if DEBUG
                        DEBUG_LOG("->SetVarL\n");
#endif
                        break;
                    }
                    case SetVarEvent:{//local event
                        eventPosIndex++;
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
                                    Array_push(globalEvent,newPair(function,_newInteger(eventIndex)));
                                    eventIndex++;
                                }
                            }
                        }else{
                            if(getType(get(id,Symbol,value))==DupEvent){//global event

                                oop judge = findIdFromList(id,globalEvent);
                                if(judge==nil){
                                    Array_push(globalEvent,newPair(id,_newInteger(eventIndex)));
                                    eventIndex++;
                                }
                                eveFunc = get(get(id,Symbol,value),DupEvent,eventFunc);
                            }
                            else if(getType(get(id,Symbol,value))!=EventFunc){//local event
#if DEBUG
                                DEBUG_ERROR("line %d: %s is not EventFunc\n",__LINE__,get(id,Symbol,name));
                                exit(1);
#endif
                                oop dupEve = findIdFromList(id,sttLocalEvent); //it shluld be change: 
                                if(dupEve!=nil){
                                    eveFunc = get(dupEve,DupEvent,eventFunc);
                                }
                            }else{//normal event
                                oop judge = findIdFromList(id,globalEvent);
                                if(judge==nil){
                                    insertEventTable(id);
                                    Array_push(globalEvent,newPair(id,_newInteger(eventIndex)));
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
            // sttLocalEvent = rePair(sttLocalEvent,nil);
            globalEvent = sortEventListByStateTable(stateName,globalEvent);
            exp->State.events = Array_put_elements(events,globalEvent,eventPosIndex);
            printlnObject(exp,2);
            // exp = Array_put_array(exp,sttLocalEvent,eventPosIndex+(globalEvent->Array.size));
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