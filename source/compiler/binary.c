#ifndef BINARY_C
#define BINARY_C
#include "setting.h"
#include "object.c"

//NEED TO BE MORE SMART
struct State_Event_Binary_Data{
    oop stateName;
    int eventBinarylist;
};

//stateごとに定義されているローカルイベントリスト
oop STATE_DEF_LOCAL_EVENT_LISTS   = 0;

oop EVENT_TABLE[32];      //use for colloect event id
int sizeOfEventTable = 0;

struct State_Event_Binary_Data* STATE_EVENT_BINARY_DATA = 0;    
int sizeOfStateEventBinaryData = 0;

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

//for local event
int getEventListIndex(oop id, oop list){
    assert(getType(list)==Array);
    oop *elements = get(list,Array,elements);
    int size = get(list,Array,size);
    for(int i=0;i<size;i++){
        if(get(elements[i],Pair,a)==id){
            return (1<<(i+1))|1;
        }
    }
    return -1;
}
//for global event
int getEventTableIndex(oop id){
    for(int i=0;i<sizeOfEventTable;i++){
        if(EVENT_TABLE[i]==id){
            return (1<<i);
        }
    }
    return -1;
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
    printf("Event Table\n");
    for(int i=0;i<sizeOfEventTable;i++){
        printf("%02d: %s()\n",i,get(EVENT_TABLE[i],Symbol,name));
    }
    putchar('\n');
}
void printEventBinaryData(){
    printEventTable();
    printf("State Event Binary Data\n");
    for(int i=0;i<sizeOfStateEventBinaryData;i++){
        printf("%s\n",get(STATE_EVENT_BINARY_DATA[i].stateName,Symbol,name));
        printf("--> ");
        printBinary(STATE_EVENT_BINARY_DATA[i].eventBinarylist);
    }
    putchar('\n');
}
#endif


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
            oop eventId = get(ele[j],Pair,a);
            if(EVENT_TABLE[i]==eventId){
                Array_push(new,ele[j]->Pair.b);
                eventBinaryData |= (1<<i);
            }
        }
    }
#if DEBUG
    printBinary(eventBinaryData);
#endif
    free(eventList);
    insertEventBinaryData(stateName,eventBinaryData);
    return new;
}


oop findIdFromList(oop id,oop list){
    assert(getType(list)==Array);
    oop *elements = get(list,Array,elements);
    int size = get(list,Array,size);
    for(int i=0;i<size;i++){
        if(get(elements[i],Pair,a)==id){
            return get(elements[i],Pair,b);
        }
    }
    return nil;
}




#endif // BINARY_C