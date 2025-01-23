
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
    char sizeOfMember;
    char *groupKey;
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

#endif