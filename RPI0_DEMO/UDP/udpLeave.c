#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>

#define BROADCAST_PORT 60000
#if 1
#define BROADCAST_ADDR "172.28.79.255"
#else
#define BROADCAST_ADDR "192.168.1.255" // サブネットに合わせて設定
#endif
#define BUF_SIZE 16
#define MAX_GROUP 127 

#include "agent.c"

/*
        Debug
*/
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

/*
        Communication
*/
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


int init(int *sockfd, struct sockaddr_in *addr, int is_broadcast) {
    int yes = 1;

    // ソケットの作成
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // ブロードキャストを有効にする（必要なら）
    if (is_broadcast) {
        if (setsockopt(*sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
            perror("setsockopt(SO_BROADCAST)");
            close(*sockfd);
            return -1;
        }
    }

    // アドレス構造体の初期化
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(BROADCAST_PORT);
    addr->sin_addr.s_addr = is_broadcast ? inet_addr(BROADCAST_ADDR) : INADDR_ANY;

    return 0;
}

// グループ参加リクエストを送信する関数
agent_p joinGroupRrequet(char groupID, char *groupKey, agent_p agent) {
    int sockfd;
    struct sockaddr_in broadcast_addr;
    socklen_t broadcast_addr_len = sizeof(broadcast_addr);

    // 初期化処理
    if (init(&sockfd, &broadcast_addr, 1) < 0) {
        return agent;
    }

    // 送信データの準備
    char send_data[BUF_SIZE] = {
        REQUEST_JOIN, // Request
        groupID,      // Group ID
        0,            // My ID
        0,            // Request Member ID
        0,            // Size of Member
        groupKey[0], groupKey[1], groupKey[2], groupKey[3] // Key
    };

    // ブロードキャストパケット送信
#if DEBUG
    DEBUG_LOG("Send Request");
#endif
    if(sendto(sockfd, send_data, BUF_SIZE - 1, 0,
               (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
        perror("sendto");
    }

    time_t start = time(NULL);
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE); // 受信バッファの初期化


    while (time(NULL) - start < 5) {
        int ret = recvfrom(sockfd, buf, BUF_SIZE - 1, MSG_DONTWAIT,
                           (struct sockaddr *)&broadcast_addr, &broadcast_addr_len);
        if (ret < 0) {
            continue;
        }
#if DEBUG   
        DEBUG_LOG("Receive Data");
#endif
        if (buf[2] == groupID || memcmp(buf + DATA_GROUP_KEY, groupKey, 4) == 0) {
            switch(buf[0]){
                case REQUEST_TO_BE_MEMBER:{
                    agent = createAgent(AgentMember);
                    agent->base.groupID = groupID;
                    agent->base.myID = buf[2];
                    agent->reader.sizeOfMember = buf[5];
                    agent->reader.groupKey = strdup(groupKey);
                    printf("Joint My ID: %d\n", agent->base.myID);
                    break;
                }
                case REQUEST_REJECT:{
                    printf("Reject\n");
                    return agent;
                }
            }
        }
        
    }
    close(sockfd);

    if (agent == NULL) {
        agent = createAgent(AgentReader);
        agent->base.groupID = groupID;
        agent->base.myID = 0;
        agent->reader.sizeOfMember = 1;
        agent->reader.groupKey = strdup(groupKey);
        printf("I'm Reader\n");
    }

    return agent;
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

// グループ管理を行う関数
int isBind = 0;
agent_p groupManage(agent_p agent,struct sockaddr_in *sender_addr,int sockfd) {

    char buf[BUF_SIZE];
    int groupID = agent->base.groupID;
    char *groupKey = agent->reader.groupKey;
    char sizeOfMember = agent->reader.sizeOfMember;

    socklen_t addr_len = sizeof(*sender_addr);
    int ret = recvfrom(sockfd, buf, BUF_SIZE, MSG_DONTWAIT,
                    (struct sockaddr *)sender_addr, &addr_len);

    memset(buf, 0, BUF_SIZE); // 受信バッファの初期化

    if(ret < 0){
        return agent;
    }
#if DEBUG
    DEBUG_LOG("Receive Data");
#endif
    if (buf[2] == groupID || memcmp(buf + DATA_GROUP_KEY, groupKey, 4) == 0) {
#if DEBUG
        DEBUG_LOG("Receive Request");
#endif
        switch(buf[0]){
            case REQUEST_JOIN:{
                if(sizeOfMember == MAX_GROUP){
                    buf[0] = REQUEST_REJECT;
                    buf[1] = groupID;
                    buf[2] = 0;
                    buf[3] = 0; // Request Member ID
                    buf[4] = 0; // Size of Member
                    if (sendto(sockfd, buf, 10, 0,
                            (struct sockaddr *)&sender_addr, sizeof(sender_addr)) < 0) {
                        perror("sendto");
                    }
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
                if (sendto(sockfd, buf, BUF_SIZE - 1, 0,
                        (struct sockaddr *)&sender_addr, sizeof(sender_addr)) < 0) {
                    perror("sendto");
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
    
    printf("\nFrom IP: %s ID%3d\n", inet_ntoa(sender_addr->sin_addr), buf[2]);
    printMember(sizeOfMember);
    
    return agent;
}


int main(int argc, char *argv[])
{
    int groudID = 111;
    char *groupKey = "KUAS";
    union Agent *agent = NULL;
    int isSend = 0;


    while(argc > 1){
        if(strcmp(argv[1],"-r") == 0){
            isSend = 1;
            break;
        }
        argc--;
        argv++;
    }
#if DEBUG
    DEBUG_LOG("join Request");
#endif
    if(isSend){
        agent = joinGroupRrequet(groudID,groupKey,agent);
    }

    int start = time(NULL);
    
        switch(agent->base.type){
            case AgentMember:
                // agent = triBroadMSG(agent);
                // time_t start = time(NULL);
                // while(time(NULL) - start < 10);
                // LeaveGroupRequest(agent);
                break;
            case AgentReader:{
                int sockfd; 
                struct sockaddr_in recv_addr;
                struct sockaddr_in sender_addr;
                // 初期化処理
                if (init(&sockfd, &recv_addr, 0) < 0) {
                    return -1;
                }
                if(init(&sockfd,&sender_addr,0) < 0){
                    return -1;
                }
                // ポートへバインド
                if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
                    perror("bind");
                    close(sockfd);
                    return -1;
                }
                while(1){
                    agent = groupManage(agent,&recv_addr,sockfd);
                }

            }
            default:
                printf("I'm Undefined\n");
                break;
        }
    

    return 0;
}
