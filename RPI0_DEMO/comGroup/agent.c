
#ifndef AGENT_C
#define AGENT_C

#include <stdlib.h>

#define MAX_GROUP 8

typedef enum AgentType{
    AgentBase,
    AgentReader,
    AgentMember,
    AgentVisitor,
}agent_t;

typedef union Agent *agent_p;



struct AgentBase{
    agent_t type;
    char myID;
    char groupID;
};

struct AgentReader{
    struct AgentBase base;
    char *groupKey;
    unsigned char sizeOfMember;
};

struct AgentMember{
    struct AgentBase base;
    char *groupKey;
};
struct AgentVisitor{
    struct AgentBase base;
};

union Agent{
    struct AgentBase base;
    struct AgentReader reader;
    struct AgentMember member;
};

agent_p _createAgent(agent_t type,int size){
    agent_p agent = (agent_p)malloc(sizeof(union Agent));
    if(agent == NULL){
        return NULL;
    }
    agent->base.type = type;

    return agent;
}
#define createAgent(TYPE) _createAgent(TYPE,sizeof(struct TYPE))

agent_p _check(agent_p node,enum AgentType type, char *file, int line)
{
    if (node->base.type != type) {
        printf("%s line %d: expected type %d got type %d\n", file, line, type, node->base.type);
        exit(1);
    }
    return node;
}
#define getA(PTR, TYPE, FIELD)	(_check((PTR), TYPE, __FILE__, __LINE__)->TYPE.FIELD)

void printAgentData(agent_p agent){
    switch(agent->base.type){
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
    }
}
#endif