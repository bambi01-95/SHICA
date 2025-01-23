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


void keyCheck(char *key,char *msg){
    for(int i = 0; i < 4; i++){
        if(key[i] != msg[i]){
            printf("key[%d] is %c\n",i,key[i]);
            printf("msg[%d] is %c\n",i,msg[i]);
        }
    }
}


#ifndef BROADCAST_PORT
#define BROADCAST_PORT 60000
#endif

#ifdef BROADCAST_ADDR
#undef BROADCAST_ADDR
#endif
#define BROADCAST_ADDR "192.168.1.255"

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


#define DATA_REQUEST_TYPE        0x00
#define DATA_GROUP_ID            0x01
#define DATA_MY_ID               0x02
#define DATA_REQUEST_MEMEBER_ID  0x03
#define DATA_SIZE_OF_MEMBER      0x04
#define DATA_GROUP_KEY           0x05

#define SIZE_OF_DATA_GROUP_KEY   0x04


#ifdef BUF_SIZE
#undef BUF_SIZE
#endif
#define BUF_SIZE                16

#include "broadcast.c"
struct SocketInfo{
    int recv_sockfd, send_sockfd;
    struct sockaddr_in recv_addr, broadcast_addr, sender_addr;
    socklen_t addr_len;
    char own_ip[INET_ADDRSTRLEN];
};


#include "agent.c"

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




agent_p joinGroupRrequet(struct SocketInfo *socketInfo, char *requestbuf){
#define TIMEOUT 2
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    memcpy(buf,requestbuf,BUF_SIZE);

    // グループ参加リクエストの送信
    int ret = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buf, BUF_SIZE);
    if (ret < 0) {
        perror("send_broadcast_nonblocking");
        return NULL;
    }

    // グループ参加リクエストの受信
    time_t start = time(NULL);
    while(time(NULL) - start < TIMEOUT){
        ssize_t ret = recvfrom(socketInfo->recv_sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&socketInfo->sender_addr, &socketInfo->addr_len);
        if (ret > 0) {
            buf[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(socketInfo->sender_addr.sin_addr);
            // 自分自身の送信データを無視
            if (strcmp(sender_ip, socketInfo->own_ip) == 0) {
                DEBUG_LOG("same ip\n");
                continue;
            }
            printf("\nReceived from %s: %s\n", sender_ip, buf);
            if(buf[DATA_GROUP_ID] == requestbuf[DATA_GROUP_ID] && memcmp(buf + DATA_GROUP_KEY, requestbuf + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) == 0){
                switch(buf[DATA_REQUEST_TYPE]){
                    case REQUEST_TO_BE_MEMBER:{
                        agent_p agent = createAgent(AgentMember);
                        agent->base.myID = buf[DATA_MY_ID];
                        agent->base.groupID = buf[DATA_GROUP_ID];
                        agent->member.groupKey = (char *)malloc(SIZE_OF_DATA_GROUP_KEY + 1);
                        memcpy(agent->member.groupKey,buf + DATA_GROUP_KEY,SIZE_OF_DATA_GROUP_KEY + 1);
                        DEBUG_LOG("Join Group Success: my id is %d\n",agent->base.myID);
                        return agent;
                    }
                    case REQUEST_REJECT:{
                        DEBUG_LOG("Join Group Reject\n");
                        return NULL;
                    }
                }
                break;
            }
        }
    }
    // タイムアウト
    agent_p agent = createAgent(AgentReader);
    agent->base.myID = 0;
    agent->base.groupID = requestbuf[DATA_GROUP_ID];
    agent->reader.sizeOfMember = (1U);
    agent->reader.groupKey = (char *)malloc(SIZE_OF_DATA_GROUP_KEY + 1);
    memcpy(agent->reader.groupKey,requestbuf + DATA_GROUP_KEY,SIZE_OF_DATA_GROUP_KEY + 1);
    return agent;
#undef TIMEOUT
}

agent_p leaveGroupRequest(agent_p agent,struct SocketInfo *socketInfo){

#define TIMEOUT 2
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    switch(agent->base.type){
        case AgentMember:{
            DEBUG_LOG("REQUEST_LEAVE\n");
            buffer[DATA_REQUEST_TYPE] = REQUEST_LEAVE;
            buffer[DATA_GROUP_ID] = agent->base.groupID;
            buffer[DATA_MY_ID]    = agent->base.myID;
            buffer[DATA_REQUEST_MEMEBER_ID] = 0; //to reader
            memcpy(buffer + DATA_GROUP_KEY, agent->member.groupKey, SIZE_OF_DATA_GROUP_KEY);
            break;
        }
        case AgentReader:{
            DEBUG_LOG("REQUEST_TO_BE_READER\n");
            buffer[DATA_REQUEST_TYPE] = REQUEST_TO_BE_READER;
            buffer[DATA_GROUP_ID] = agent->base.groupID;
            buffer[DATA_MY_ID]    = agent->base.myID;
            char nextReaderId = 0;
            char list = agent->reader.sizeOfMember;
            while(list){
                if((list & 0x01) == 0){
                    break;
                }
                nextReaderId++;
                list >>= 1;
            }
            buffer[DATA_REQUEST_MEMEBER_ID] = nextReaderId; //to reader
            buffer[DATA_SIZE_OF_MEMBER] = agent->reader.sizeOfMember;
            memcpy(buffer + DATA_GROUP_KEY, agent->reader.groupKey, SIZE_OF_DATA_GROUP_KEY);
            break;
        }
    }

    // グループ脱退リクエストの送信
    int ret = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
    if (ret < 0) {
        perror("send_broadcast_nonblocking");
        return NULL;
    }

    // グループ脱退リクエストの受信
    time_t start = time(NULL);
    while(time(NULL) - start < TIMEOUT){
        ssize_t ret = recvfrom(socketInfo->recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&socketInfo->sender_addr, &socketInfo->addr_len);
        if (ret > 0) {
            buffer[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(socketInfo->sender_addr.sin_addr);
            // 自分自身の送信データを無視
            if (strcmp(sender_ip, socketInfo->own_ip) == 0) {
                continue;
            }
            printf("\nReceived from %s: %s\n", sender_ip, buffer);
            if(buffer[DATA_GROUP_ID] == agent->base.groupID && memcmp(buffer + DATA_GROUP_KEY, agent->reader.groupKey, SIZE_OF_DATA_GROUP_KEY) == 0){
                switch(buffer[DATA_REQUEST_TYPE]){
                    case REQUEST_SUCCESS:{
                        DEBUG_LOG("Leave Group Success\n");
                        return NULL;
                    }
                    default:{
                        DEBUG_LOG("UNSPUPPORTED REQUEST %d\n",buffer[DATA_REQUEST_TYPE]);
                        break;
                    }
                }
                break;
            }else{
            printAgentData(agent);
                if(agent->base.groupID != buffer[DATA_GROUP_ID]){
                DEBUG_LOG("UNSPUPPORTED GROUP %d (!= %d)\n",buffer[DATA_GROUP_ID],agent->base.groupID);
                }
                buffer[DATA_GROUP_KEY + SIZE_OF_DATA_GROUP_KEY] = '\0';
                if(memcmp(agent->reader.groupKey, buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) != 0){
                DEBUG_LOG("UNSPUPPORTED GROUP KEY %s (!=%s)\n",buffer + DATA_GROUP_KEY,agent->reader.groupKey);
                keyCheck(agent->reader.groupKey,buffer + DATA_GROUP_KEY);
                }
            }
        }
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


agent_p groupManage(agent_p agent,struct SocketInfo *socketInfo){
        // メッセージ受信
        char buffer[BUF_SIZE];
        ssize_t ret = recvfrom(socketInfo->recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&socketInfo->sender_addr, &socketInfo->addr_len);
        if (ret > 0) {
            buffer[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(socketInfo->sender_addr.sin_addr);
            // 自分自身の送信データを無視
            if (strcmp(sender_ip, socketInfo->own_ip) == 0) {
                return agent;
            }
            printf("\nReceived from %s: %s\n", sender_ip, buffer);


            if(agent->base.groupID == buffer[DATA_GROUP_ID] && memcmp(agent->reader.groupKey, buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) == 0){
                switch(buffer[DATA_REQUEST_TYPE]){
                    case REQUEST_JOIN:{
                        DEBUG_LOG("REQUEST_JOIN");
                        int list = agent->reader.sizeOfMember;
                        int newId = 0;
                        while(list){
                            if((list & 0x01) == 0){
                                break;
                            }
                            newId++;
                            list >>= 1;
                        }
                        if(newId == 8){
                            //send REJECT  
                            buffer[DATA_REQUEST_TYPE] = REQUEST_REJECT;
                            int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                            if (sent < 0) {
                                perror("sendto");
                            } else {
                                printf("Replied to %s: REJECT\n", sender_ip);
                            }
                        }
                        else{
                            //send TO_BE_MEMBER
                            agent->reader.sizeOfMember |= (1 << newId);
                            buffer[DATA_REQUEST_TYPE] = REQUEST_TO_BE_MEMBER;
                            buffer[DATA_MY_ID] = agent->base.myID;
                            buffer[DATA_REQUEST_MEMEBER_ID] = (1 << newId);
                            buffer[DATA_SIZE_OF_MEMBER] = agent->reader.sizeOfMember | (1 << newId);
                            int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                            if (sent < 0) {
                                perror("sendto");
                            } else {
                                printf("Replied to %s: TO_BE_MEMBER given ID %d\n", sender_ip,newId);
                            }
                            // printMember(buffer[DATA_SIZE_OF_MEMBER]);
                        }
                        break;
                    }

                    case REQUEST_LEAVE:{
                        DEBUG_LOG("REQUEST_LEAVE\n");
                        agent->reader.sizeOfMember &= ~(1 << buffer[DATA_MY_ID]); 
                        //send SUCCESS
                        buffer[DATA_REQUEST_TYPE] = REQUEST_SUCCESS;
                        buffer[DATA_MY_ID] = agent->base.myID;
                        int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                        if (sent < 0) {
                            perror("sendto");
                        } else {
                            printf("Replied to %s: SUCCESS\n", sender_ip);
                        }
                        break;
                    }
                    default:{
                        DEBUG_LOG("UNSPUPPORTED REQUEST %d\n",buffer[DATA_REQUEST_TYPE]);
                        break;
                    }
                }
            }else{
            printAgentData(agent);
                if(agent->base.groupID != buffer[DATA_GROUP_ID]){
                DEBUG_LOG("UNSPUPPORTED GROUP %d (!= %d)\n",buffer[DATA_GROUP_ID],agent->base.groupID);
                }
                buffer[DATA_GROUP_KEY + SIZE_OF_DATA_GROUP_KEY] = '\0';
                if(memcmp(agent->reader.groupKey, buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) != 0){
                DEBUG_LOG("UNSPUPPORTED GROUP KEY %s (!=%s)\n",buffer + DATA_GROUP_KEY,agent->reader.groupKey);
                keyCheck(agent->reader.groupKey,buffer + DATA_GROUP_KEY);
                }
            }
        }
        usleep(100000); // 100ms待機してループを回す
        return agent;
}




agent_p triWifiReceive(agent_p agent, struct SocketInfo *SocketInfo){
    char buffer[BUF_SIZE];
    ssize_t ret = recvfrom(SocketInfo->recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&SocketInfo->sender_addr, &SocketInfo->addr_len);
    if (ret > 0) {
        buffer[ret] = '\0'; // Null終端
        char *sender_ip = inet_ntoa(SocketInfo->sender_addr.sin_addr);
        // 自分自身の送信データを無視
        if (strcmp(sender_ip, SocketInfo->own_ip) == 0) {
            return agent;
        }
        printf("Received from %s: %s\n", sender_ip, buffer);
        if((agent->base.groupID == buffer[DATA_GROUP_ID]) && (memcmp(agent->reader.groupKey, buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) == 0)){
            switch(buffer[DATA_REQUEST_TYPE]){
                case REQUEST_TO_BE_READER:{
                    DEBUG_LOG("REQUEST_TO_BE_READER\n");
                    agent_p newAgent = createAgent(AgentReader);
                    newAgent->base.myID = 0;
                    newAgent->base.groupID = buffer[DATA_GROUP_ID];
                    newAgent->reader.sizeOfMember = buffer[DATA_SIZE_OF_MEMBER] & ~(1 << agent->base.myID);
                    newAgent->reader.groupKey = strdup(agent->reader.groupKey);

                    buffer[DATA_REQUEST_TYPE] = REQUEST_SUCCESS;
                    buffer[DATA_MY_ID]        = agent->base.myID;
                    int sent = send_broadcast_nonblocking(SocketInfo->send_sockfd, &SocketInfo->broadcast_addr, buffer, BUF_SIZE);
                    if (sent < 0) {
                        perror("sendto");
                    } else {
                        printf("Replied to %s: SUCCESS\n", sender_ip);
                    }
                    return newAgent;
                }
                default:{
                    DEBUG_LOG("UNSPUPPORTED REQUEST %d\n",buffer[DATA_REQUEST_TYPE]);
                    break;
                }
            }
        }else{
            printAgentData(agent);
            if(agent->base.groupID != buffer[DATA_GROUP_ID]){
                DEBUG_LOG("UNSPUPPORTED GROUP %d (!= %d)\n",buffer[DATA_GROUP_ID],agent->base.groupID);
            }
            buffer[DATA_GROUP_KEY + SIZE_OF_DATA_GROUP_KEY] = '\0';
            if(memcmp(agent->reader.groupKey, buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) != 0){
                DEBUG_LOG("UNSPUPPORTED GROUP KEY %s (!=%s)\n",buffer + DATA_GROUP_KEY,agent->member.groupKey);
                keyCheck(agent->reader.groupKey,buffer + DATA_GROUP_KEY);
            }
        }
    }
    return agent;
}



int main(int argc, char *argv[])
{
    int Flag = 0;
    if(argc == 2){
        if(strcmp(argv[1],"-d") == 0){
            Flag = 1;
        }
    }

    struct SocketInfo socketInfo;
    char buffer[BUF_SIZE];

    socketInfo.addr_len = sizeof(socketInfo.sender_addr);

    // 自身のネットワークIPアドレスを取得
    if (get_network_ip(socketInfo.own_ip, sizeof(socketInfo.own_ip)) != 0) {
        fprintf(stderr, "Failed to get own IP address\n");
        return -1;
    }
    printf("Own IP: %s\n", socketInfo.own_ip);

    // 受信ソケット作成とノンブロッキング化
    if (create_receive_socket(&socketInfo.recv_sockfd, &socketInfo.recv_addr) == 0) {
        set_nonblocking(socketInfo.recv_sockfd);
    } else {
        return -1;
    }

    // 送信ソケット作成とブロードキャストアドレス設定
    if (create_broadcast_socket(&socketInfo.send_sockfd, &socketInfo.broadcast_addr) != 0) {
        close(socketInfo.recv_sockfd);
        return -1;
    }

    agent_p agent = NULL;
    char requestMsg[BUF_SIZE];
    memset(requestMsg, 0, BUF_SIZE);
    requestMsg[DATA_REQUEST_TYPE] = REQUEST_JOIN;
    requestMsg[DATA_GROUP_ID] = 1;
    requestMsg[DATA_MY_ID] = 0;
    requestMsg[DATA_SIZE_OF_MEMBER] = 1;
    memcpy(requestMsg + DATA_GROUP_KEY, "abcd", 4);


    agent = joinGroupRrequet(&socketInfo,requestMsg);
    printAgentData(agent);
    if(agent == NULL){
        close(socketInfo.recv_sockfd);
        close(socketInfo.send_sockfd);
        DEBUG_LOG("Failed to join group\n");
        return -1;
    }
    time_t start = time(NULL);
    if(Flag){
        while(1){
            switch(agent->base.type){
                case AgentMember:{
                    DEBUG_LOG("\nAgentMember\n");
                    while(time(NULL) - start < 10){
                        agent = triWifiReceive(agent,&socketInfo);
                    }
                    agent = leaveGroupRequest(agent,&socketInfo);
                    printf("Leave Group\n");
                    return 0;
                }
                case AgentReader:{
                    DEBUG_LOG("\nAgentReader\n");
                    while(agent->base.type == AgentReader){
                        agent = groupManage(agent,&socketInfo);
                    }
                    break;
                }
            }
        }
    }else{
        while(1){
            switch(agent->base.type){
                case AgentMember:{
                    DEBUG_LOG("\nAgentMember\n");
                    while(agent->base.type == AgentMember){
                        agent = triWifiReceive(agent,&socketInfo);
                    }
                    printf("will be reader\n");
                    break;
                }
                case AgentReader:{
                    DEBUG_LOG("\nAgentReader\n");
                    while(time(NULL) - start < 10){
                        agent = groupManage(agent,&socketInfo);
                    }
                    agent = leaveGroupRequest(agent,&socketInfo);
                    printf("Leave Group\n");
                    return 0;
                }
            }
        }
    }

    return 0;
    while (1) {
        // メッセージ受信
        ssize_t ret = recvfrom(socketInfo.recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&socketInfo.sender_addr, &socketInfo.addr_len);
        if (ret > 0) {
            buffer[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(socketInfo.sender_addr.sin_addr);

            // 自分自身の送信データを無視
            if (strcmp(sender_ip, socketInfo.own_ip) == 0) {
                continue;
            }

            printf("Received from %s: %s\n", sender_ip, buffer);

            // 受信したアドレスに "world" を送信
            ssize_t sent = sendto(socketInfo.send_sockfd, "world", 5, 0, (struct sockaddr *)&socketInfo.sender_addr, socketInfo.addr_len);
            if (sent < 0) {
                perror("sendto");
            } else {
                printf("Replied to %s: world\n", sender_ip);
            }

            // "join" をブロードキャストで送信
            ssize_t broadcast_sent = send_broadcast_nonblocking(socketInfo.send_sockfd, &socketInfo.broadcast_addr, "join", 4);
            if (broadcast_sent < 0) {
                printf("Failed to broadcast join\n");
            } else {
                printf("Broadcasted: join\n");
            }
        }

        usleep(100000); // 100ms待機してループを回す
    }

    close(socketInfo.recv_sockfd);
    close(socketInfo.send_sockfd);

    return 0;
}