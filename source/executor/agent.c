
#ifndef AGENT_C
#define AGENT_C

#include <stdlib.h>
#include "./object.c"
#include "./lib/msgc.c"
#include "./lib/extstr.c"
#define MAX_GROUP 8

typedef enum AgentType{
    AgentBase,
    AgentReader,
    AgentMember,
    AgentVisitor,
}agent_t;

typedef union Agent *agent_p;



struct AgentBase{
    enum Type type; //for gc
    agent_t agent_type;
    char myID;
    char groupID;
};

struct AgentReader{
    struct AgentBase base;
    char groupKey[4];//check me
    unsigned char sizeOfMember;
};

struct AgentMember{
    struct AgentBase base;
    char groupKey[4];//check me
};
struct AgentVisitor{
    struct AgentBase base;
};

#define STRUCT_AGENT_TYPE 0b000
union Agent{
    enum Type type;
    struct AgentBase base;
    struct AgentReader reader;
    struct AgentMember member;
    struct AgentVisitor visitor;
};

agent_p _createAgent(agent_t type,int size){
#if MSGC
    agent_p agent = (agent_p)gc_alloc(size);
    agent->base.type = registerExternType(STRUCT_AGENT_TYPE);
#else
    agent_p agent = (agent_p)malloc(sizeof(union Agent));
#endif
    if(agent == NULL){
        return NULL;
    }
    agent->base.agent_type = type;
    return agent;
}
#define createAgent(TYPE) _createAgent(TYPE,sizeof(struct TYPE))

agent_p _check(agent_p node,enum AgentType type, char *file, int line)
{
    if (node->base.agent_type != type) {
        printf("%s line %d: expected type %d got type %d\n", file, line, type, node->base.type);
        exit(1);
    }
    return node;
}
#define getA(PTR, TYPE, FIELD)	(_check((PTR), TYPE, __FILE__, __LINE__)->TYPE.FIELD)

char *getAgentGroupKey(agent_p agent){
    switch(agent->base.agent_type){
        case AgentMember:{
            return agent->member.groupKey;
        }
        case AgentReader:{
            return agent->reader.groupKey;
        }
        case AgentVisitor:
        default:{
            printf("%s line %d UNKNOWN\n",__FILE__,__LINE__);
        }
    }
    return 0;
}

void setAgentGroupKey(agent_p agent,char *groupKey){
    switch(agent->base.agent_type){
        case AgentMember:{
            memcpy(agent->member.groupKey,groupKey,4);
            break;
        }
        case AgentReader:{
            memcpy(agent->reader.groupKey,groupKey,4);
            break;
        }
        case AgentVisitor:{
            break;
        }
        default:{
            printf("%s line %d UNKNOWN\n",__FILE__,__LINE__);
        }
    }
}


void printAgentData(agent_p agent){
    switch(agent->base.agent_type){
        case AgentMember:{
            printf("AgentMember\n");
            printf("myID:%d\n",agent->base.myID);
            printf("groupID:%d\n",agent->base.groupID);
            printf("groupKey:%*s\n",4,agent->member.groupKey);
            break;
        }
        case AgentReader:{
            printf("AgentReader\n");
            printf("myID:%d\n",agent->base.myID);
            printf("groupID:%d\n",agent->base.groupID);
            printf("groupKey:%*s\n",4,agent->reader.groupKey);
            printf("sizeOfMember:%d\n",agent->reader.sizeOfMember);
            break;
        }
        case AgentVisitor:{
            printf("AgentVisitor\n");
            break;
        }
        default:{
            printf("%s line %d UNKNOWN\n",__FILE__,__LINE__);
        }
    }
}
#endif