#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

#ifndef BROADCAST_PORT
#define BROADCAST_PORT 60000
#endif

#ifndef BROADCAST_ADDR
#define BROADCAST_ADDR "172.28.79.255"
#endif

#ifndef BUF_SIZE
#define BUF_SIZE          16
#endif

#define DEBUG 1
void _DEBUG_LOG(int line, const char *fmt, ...) {
    printf("line  %3d: ",line);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    putchar('\n'); 
}
#define DEBUG_LOG(...) _DEBUG_LOG(__LINE__,##__VA_ARGS__)

#include "agent.c"
#include "broadcast.c"

typedef enum RequestType{
    REQUEST_UNDEFINED,
    REQUEST_JOIN,
    REQUEST_LEAVE,
    REQUEST_TO_BE_MEMBER,
    REQUEST_TO_BE_READER,
    REQUEST_SUCCESS,
    REQUEST_REJECT,
    REQUEST_MOVE,
}request_t;

typedef enum RequestDataIndex{
    DATA_REQUEST_TYPE,
    DATA_GROUP_ID,
    DATA_MY_ID,
    DATA_REQUEST_MEMEBER_ID,
    DATA_SIZE_OF_MEMBER,
    DATA_GROUP_KEY,
}RequestDataIndex;


agent_p joinGroupRrequet(int soocket,struct sockaddr_in *broadcast_addr,char *requestMsg){
#define TIMEOUT 2
    agent_p agent = NULL;
    //send request message 
    int ret = send_broadcast_nonblocking(soocket,broadcast_addr,requestMsg,strlen(requestMsg));
    time_t start = time(NULL);
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE); // 受信バッファの初期化

    while(time(NULL) - start < TIMEOUT){
        int ret = receive_broadcast_nonblocking(soocket,buf,BUF_SIZE);
        if (ret < 0) {
            continue;
        }
        printf("Receive Data\n");
        if (buf[1] == requestMsg[1] || memcmp(buf + DATA_GROUP_KEY, requestMsg + DATA_GROUP_KEY, 4) == 0) {
            switch(buf[0]){
                case REQUEST_TO_BE_MEMBER:{
                    agent = createAgent(AgentMember);
                    agent->base.groupID = buf[1];
                    agent->base.myID    = buf[2];
                    agent->reader.sizeOfMember = buf[5];
                    agent->reader.groupKey = strdup(requestMsg + DATA_GROUP_KEY);
                    printf("JOIN Group My ID: %d\n", agent->base.myID);
                    return agent;
                }
                case REQUEST_REJECT:{
                    printf("Join Request Rejected\n");
                    return agent;
                }
            }
        }
    }

    agent = createAgent(AgentReader);
    agent->base.groupID = requestMsg[1];
    agent->base.myID = 0;
    agent->reader.sizeOfMember = 1;
    agent->reader.groupKey = strdup(requestMsg + DATA_GROUP_KEY);
    printf("MAKE Group My ID: %d\n", agent->base.myID);  
    return agent;
#undef TIMEOUT
}


void printMember(char sizeOfMember){
    for (char i = 7; i >= 0; i--) {
        if ((sizeOfMember & (1U << i)) != 0) {
            putchar('1');
        }else{
            putchar('0');
        }
    }
    printf("\n");
}


agent_p groupManage(agent_p agent,struct sockaddr_in *broadcast_addr,int sockfd) {
    // グループIDとグループキーの取得
    int groupID = agent->base.groupID;
    char *groupKey = agent->reader.groupKey;
    char sizeOfMember = agent->reader.sizeOfMember;

    // 受信バッファの初期化
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);

    // データ受信
    int ret = receive_broadcast_nonblocking(sockfd, buf, BUF_SIZE);
    if (ret < 0) {
        return agent;
    }

    // データの解析
    if (buf[1] == groupID && memcmp(buf + DATA_GROUP_KEY, groupKey, 4) == 0) {
        switch(buf[0]){
            case REQUEST_JOIN:{
                if(sizeOfMember == MAX_GROUP){
                    buf[0] = REQUEST_REJECT;
                    buf[1] = groupID;
                    buf[2] = 0;
                    buf[3] = 0; // Request Member ID
                    buf[4] = 0; // Size of Member
                    memcpy(buf + DATA_GROUP_KEY, groupKey, 4);
                    send_broadcast_nonblocking(sockfd,broadcast_addr, buf, BUF_SIZE);
                    break;
                }
                char newID = 0;
                for (newID = 0; newID < 8; newID++) {
                    if ((sizeOfMember & (1U << newID)) == 0) { 
                        printMember(sizeOfMember);
                        agent->reader.sizeOfMember |= 1U << newID;
                        sizeOfMember = agent->reader.sizeOfMember;
                        printMember(sizeOfMember);
                        break;
                    }
                }
                buf[0] = REQUEST_TO_BE_MEMBER;
                buf[1] = groupID;
                printf("New ID: %d\n", newID);
                buf[2] = newID;
                buf[3] = (1U << newID);
                buf[4] = sizeOfMember;
                ret = send_broadcast_nonblocking(sockfd, broadcast_addr, buf, BUF_SIZE);
                if (ret < 0) {
                    perror("send_broadcast_nonblocking");
                }else{
                    printf("Broadcast sent successfully\n");
                }
                break;
            }
            case REQUEST_LEAVE:{
                agent->reader.sizeOfMember &= ~(1U << buf[2]);
                sizeOfMember = agent->reader.sizeOfMember;
                break;
            }
        }
    }
    return agent;
}


int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in broadcast_addr, recv_addr;

    // 送信ソケット作成とノンブロッキング化
    if (create_broadcast_socket(&sockfd, &broadcast_addr) == 0) {
        set_nonblocking(sockfd);
    }

    int groudID = 111;
    char *gKey = "KUAS";
    union Agent *agent = NULL;

    char requestMsg[BUF_SIZE] = {
        REQUEST_JOIN, // Request
        groudID, // Group ID
        0, // My ID
        0, // Request Member ID
        0, // Size of Member
        gKey[0], gKey[1], gKey[2], gKey[3] // Key
    };

    while(argc > 1){
        if(strcmp(argv[1],"-r") == 0){
            agent = joinGroupRrequet(sockfd,&broadcast_addr,requestMsg);
            break;
        }
        argc--;
        argv++;
    }
    if(agent == NULL){
        printf("Agent is NULL\n");
        return -1;
    }

    switch(agent->base.type){
        case AgentMember:
            printf("Agent Type: Member\n");
            break;
        case AgentReader:
            printf("Agent Type: Reader\n");
            // 受信ソケット作成とノンブロッキング化
            if (create_receive_socket(&sockfd, &recv_addr) == 0) {
                set_nonblocking(sockfd);
            }
            while(agent->base.type == AgentReader){
                agent = groupManage(agent,&recv_addr,sockfd);
            }
            break;
        default:
            printf("Agent Type: Undefined\n");
            break;
    }

    return 0;
}