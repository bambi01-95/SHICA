
#ifndef COMMUNICATE_EXECUTE_C
#define COMMUNICATE_EXECUTE_C

#include "../object.c"
#include "../../common/liblist/communicate.h"
#include "./communicate-lib/broadcast.c"
#include "../agent.c"


// request type
#define  REQUEST_UNDEFINED      0x00
#define  REQUEST_JOIN           0x01
#define  REQUEST_LEAVE          0x02
#define  REQUEST_TO_BE_MEMBER   0x03
#define  REQUEST_TO_BE_READER   0x04
#define  REQUEST_SUCCESS        0x05
#define  REQUEST_REJECT         0x06
#define  REQUEST_MOVE           0x07
#define  REQUEST_TRIGER         0x08


// broadcast data index
#define DATA_REQUEST_TYPE        0x00
#define DATA_GROUP_ID            0x01
#define DATA_MY_ID               0x02
#define DATA_REQUEST_MEMEBER_ID  0x03
#define DATA_SIZE_OF_MEMBER      0x04
#define DATA_GROUP_KEY           0x05
#define DATA_DATA                0x09

// group key size
#define SIZE_OF_DATA_GROUP_KEY   0x04

// member size
#define MAX_MEMBER_SIZE          0x08
#define ALL_MEMBER_ID            0xFF

#ifdef BUF_SIZE
#undef BUF_SIZE
#endif
#define BUF_SIZE                16

struct SocketInfo{
    int recv_sockfd, send_sockfd;
    struct sockaddr_in recv_addr, broadcast_addr, sender_addr;
    socklen_t addr_len;
    char own_ip[INET_ADDRSTRLEN];
};

struct AgentInfo{
    enum Type type;
    agent_p agent;
    struct SocketInfo *socket;
};


oop eve_wifi_receive(oop core){
    //corret 
#if SBC
        struct SocketInfo *socketInfo = MY_AGENT_INFO->socket;
        agent_p agent = MY_AGENT_INFO->agent;
        // メッセージ受信
        char buffer[BUF_SIZE];
        pirntf("receiving\n");
        ssize_t ret = recvfrom(socketInfo->recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&socketInfo->sender_addr, &socketInfo->addr_len);
        printf("ret is %d\n",ret);
        if (ret > 0) {
            buffer[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(socketInfo->sender_addr.sin_addr);
            // 自分自身の送信データを無視
            if (strcmp(sender_ip, socketInfo->own_ip) == 0) {
                return core;
            }
#if DEBUG
            DEBUG_LOG("\nReceived from %s: %s\n", sender_ip, buffer);
#endif

            if(agent->base.groupID == buffer[DATA_GROUP_ID] && memcmp(getAgentGroupKey(agent), buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) == 0){
                switch(buffer[DATA_REQUEST_TYPE]){
                    case REQUEST_JOIN:{
                        if(agent->base.type == AgentReader){
    #if DEBUG
                            DEBUG_LOG("REQUEST_JOIN");
    #endif
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
                                    SHICA_PRINTF("Replied to %s: REJECT\n", sender_ip);
                                }
                            }
                            else{
                                //send TO_BE_MEMBER
                                agent->reader.sizeOfMember |= (1U << newId);
    #if DEBUG
                                DEBUG_LOG("current Member is %d\n",agent->reader.sizeOfMember);
    #endif
                                buffer[DATA_REQUEST_TYPE] = REQUEST_TO_BE_MEMBER;
                                buffer[DATA_MY_ID]        = newId;
                                buffer[DATA_REQUEST_MEMEBER_ID] = (1 << newId);
                                buffer[DATA_SIZE_OF_MEMBER] = agent->reader.sizeOfMember | (1 << newId);
                                int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                                if (sent < 0) {
                                    perror("sendto");
                                } else {
                                    SHICA_PRINTF("Replied to %s: TO_BE_MEMBER given ID %d\n", sender_ip,newId);
                                }
                                // printMember(buffer[DATA_SIZE_OF_MEMBER]);
                            }
                        }
                        break;
                    }

                    case REQUEST_LEAVE:{
                        if(agent->base.type == AgentReader){
    #if DEBUG
                            DEBUG_LOG("REQUEST_LEAVE\n");
    #endif
                            agent->reader.sizeOfMember &= ~(1 << buffer[DATA_MY_ID]); 
                            //send SUCCESS
                            buffer[DATA_REQUEST_TYPE] = REQUEST_SUCCESS;
                            buffer[DATA_MY_ID] = agent->base.myID;
                            int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                            if (sent < 0) {
                                perror("sendto");
                            } else {
                                SHICA_PRINTF("Replied to %s: SUCCESS\n", sender_ip);
                            }
                        }
                        break;
                    }
                    case REQUEST_TO_BE_READER:{
                        if(agent->base.type == AgentMember){
                            if((buffer[DATA_REQUEST_MEMEBER_ID]>> (agent->base.myID-1) & 1) == 1){//checking for me or not
                                #if DEBUG
                                DEBUG_LOG("REQUEST_TO_BE_READER\n");
                                #endif
                                agent_p newAgent = createAgent(AgentReader);
                                newAgent->reader.sizeOfMember = buffer[DATA_SIZE_OF_MEMBER] & ~(1 << (agent->base.myID-1));//remove my id
                                newAgent->base.myID = 0;
                                newAgent->base.groupID = buffer[DATA_GROUP_ID];
                                printf("current Member is %d\n",newAgent->reader.sizeOfMember);
                                newAgent->reader.groupKey = strdup(agent->reader.groupKey);

                                buffer[DATA_REQUEST_TYPE] = REQUEST_SUCCESS;
                                buffer[DATA_MY_ID]        = agent->base.myID;
                                int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                                if (sent < 0) {
                                    perror("sendto");
                                } else {
                                    printf("Replied to %s: SUCCESS\n", sender_ip);
                                }
                                MY_AGENT_INFO->agent = newAgent;
                            }
                        }
                        return core;
                    }
                    case REQUEST_TRIGER:{
                        if((buffer[DATA_REQUEST_MEMEBER_ID]>> (agent->base.myID-1) & 1) == 1){
                            #if DEBUG
                            DEBUG_LOG("REQUEST_TRIGER\n");
                            #endif
                            //CHECK ME: with wifi_send_p
                            // Success Message

                            //protect t:thread


    /* trigger data */
    int isOnce = 0;
    evalEventArgsThread->Thread.stack->Array.size = 1;//1:basepoint
    for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
        int isFalse = 0;
        oop thread = core->Core.threads[thread_i];
        //<引数の評価>/<Evaluation of arguments>
        if(thread->Thread.condRelPos != 0){
            if(isOnce == 0){
                Array_push(evalEventArgsThread->Thread.stack,_newInteger(buffer[DATA_MY_ID]));
                if(buffer[DATA_REQUEST_MEMEBER_ID] == ALL_MEMBER_ID){
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(0));
                }else if(buffer[DATA_REQUEST_MEMEBER_ID] == ((1U) << agent->base.myID -1)){
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(1));
                }else{
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(2));
                }
                Array_push(evalEventArgsThread->Thread.stack,_newInteger(buffer[DATA_DATA]));
                isOnce = 1;
            }else
            {
                evalEventArgsThread->Thread.stack->Array.size = 4;
            }
            evalEventArgsThread->Thread.pc = thread->Thread.base + thread->Thread.condRelPos;
            for(;;){
                FLAG flag = sub_execute(evalEventArgsThread,nil);
                if(flag == F_TRUE){
                    break;
                }
                else if(flag == F_FALSE){
                    isFalse = 1;
                    break;
                }
            }
        }
        
        //<条件が満たされたときの処理>/<Processing when the condition is met>
        if(!isFalse){
            //protect t:thread
            gc_pushRoot((void*)&core);//CHECKME: is it need?
            oop data = newArray(2);
            Array_push(data,_newInteger(buffer[DATA_MY_ID]));
                if(buffer[DATA_REQUEST_MEMEBER_ID] == ALL_MEMBER_ID){
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(0));
                }else if(buffer[DATA_REQUEST_MEMEBER_ID] == ((1U) << agent->base.myID -1)){
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(1));
                }else{
                    Array_push(evalEventArgsThread->Thread.stack,_newInteger(2));
                }
            Array_push(data,_newInteger(buffer[DATA_DATA]));
            gc_popRoots(1);
            enqueue(thread->Thread.queue,data);
        }
    }


                            return core;
                        }
                    }
                    default:{
                        #if DEBUG
                        DEBUG_LOG("UNSPUPPORTED REQUEST %d\n",buffer[DATA_REQUEST_TYPE]);
                        #endif
                        break;
                    }

                }
            }else{
                printAgentData(agent);
                #if DEBUG
                if(agent->base.groupID != buffer[DATA_GROUP_ID]){
                DEBUG_LOG("UNSPUPPORTED GROUP %d (!= %d)\n",buffer[DATA_GROUP_ID],agent->base.groupID);
                }
                buffer[DATA_GROUP_KEY + SIZE_OF_DATA_GROUP_KEY] = '\0';
                if(memcmp(agent->reader.groupKey, buffer + DATA_GROUP_KEY, SIZE_OF_DATA_GROUP_KEY) != 0){
                DEBUG_LOG("UNSPUPPORTED GROUP KEY %s (!=%s)\n",buffer + DATA_GROUP_KEY,agent->reader.groupKey);
                }
                #endif
            }
        }
#else
    SHICA_PRINTF("eve_wifi_receive\n");
#endif
    return core;
}



oop Event_communicate(int eve_num,oop stack,int numThread){
    gc_pushRoot((void*)&stack);
    //protect t:new thread
    GC_PUSH(oop,core,0);
    switch(eve_num){
        case COMMUNICATE_WiFi_RECEIVE_E:{
            core = newCore(Default,numThread);
            core->Core.vd->Default.count = 0;
            core->Core.func = &eve_wifi_receive;
            break;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Event_communicate eve[%d]\n",eve_num);
            exit(1);
        }
    }
    gc_popRoots(2);
    return core;
}



//Normal Function



void communicate_wifi_send(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    int sendToId = api();
    int value = api();
    SHICA_PRINTF("sendToId:%d value:%d\n",sendToId,value);
    if(MY_AGENT_INFO == 0){
        SHICA_PRINTF("MY_AGENT_INFO is not set\n");
        return;
    }
    struct SocketInfo *socketInfo = MY_AGENT_INFO->socket;
    agent_p agent = MY_AGENT_INFO->agent;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    buf[DATA_REQUEST_TYPE] = REQUEST_TRIGER;
    buf[DATA_GROUP_ID] = agent->base.groupID;
    buf[DATA_MY_ID] = agent->base.myID;
    if(sendToId < 0){
        buf[DATA_REQUEST_MEMEBER_ID] = ALL_MEMBER_ID;//all member: 11111111
    }else{
        buf[DATA_REQUEST_MEMEBER_ID] = sendToId;//0:reader or other: member
    }   
    buf[DATA_SIZE_OF_MEMBER] = 1;
    memcpy(buf + DATA_GROUP_KEY, getAgentGroupKey(agent), 4);
    buf[DATA_DATA] = 0;

    // グループ参加リクエストの送信
    int ret = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buf, BUF_SIZE);
    if (ret < 0) {
        perror("send_broadcast_nonblocking");
        close(socketInfo->recv_sockfd);
        close(socketInfo->send_sockfd);
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
        MY_AGENT_INFO = 0;
        return;
    }
// CHECK ME:
// if it possible, check the return value of send_broadcast_nonblocking
// it is success, or not 
    return;
}

void communicate_wifi_build_group(oop process,oop GM){
    getInt(mpc);int size_args = int_value;
    char* ipAddr = aps();
    int   portNum = api();
    int   groupID = api();
    char* groupKey = aps();
#if DEBUG
    DEBUG_LOG("ipAddr:%s portNum:%d groupID:%d groupKey:%s\n",ipAddr,portNum,groupID,groupKey);
#endif

#if MSGC
    struct SocketInfo *socketInfo = (struct SocketInfo *)gc_alloc(sizeof(struct SocketInfo));
#else
    struct SocketInfo *socketInfo = (struct SocketInfo *)malloc(sizeof(struct SocketInfo));
#endif
    socketInfo->addr_len = sizeof(socketInfo->sender_addr);

    // 自身のネットワークIPアドレスを取得
    if (get_network_ip(socketInfo->own_ip, sizeof(socketInfo->own_ip)) != 0) {
        SHICA_FPRINTF(stderr, "Failed to get own IP address\n");
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
        MY_AGENT_INFO = 0;
        return ;
    }

#if DEBUG
    DEBUG_LOG("Own IP: %s\n", socketInfo->own_ip);
#endif

    // 受信ソケット作成とノンブロッキング化
    if (create_receive_socket(&socketInfo->recv_sockfd, &socketInfo->recv_addr,portNum) == 0) {
        set_nonblocking(socketInfo->recv_sockfd);
    } else {
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
        MY_AGENT_INFO = 0;
        return ;
    }

    // 送信ソケット作成とブロードキャストアドレス設定
    if (create_broadcast_socket(&socketInfo->send_sockfd, &socketInfo->broadcast_addr,ipAddr,portNum) != 0) {
        close(socketInfo->recv_sockfd);
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
        MY_AGENT_INFO = 0;
        return ;
    }

#if MSGC
    MY_AGENT_INFO = (struct AgentInfo *)gc_alloc(sizeof(struct AgentInfo));
#else
    MY_AGENT_INFO = (struct AgentInfo *)malloc(sizeof(struct AgentInfo));
#endif
    MY_AGENT_INFO->type = AgentInfo;
    MY_AGENT_INFO->socket = socketInfo;
  
#define TIMEOUT 2
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    buf[DATA_REQUEST_TYPE] = REQUEST_JOIN;
    buf[DATA_GROUP_ID] = groupID;
    buf[DATA_MY_ID] = 0;
    buf[DATA_REQUEST_MEMEBER_ID] = 0;
    buf[DATA_SIZE_OF_MEMBER] = 1;
    memcpy(buf + DATA_GROUP_KEY, groupKey, 4);
    buf[DATA_DATA] = 0;
    
    // グループ参加リクエストの送信
    int ret = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buf, BUF_SIZE);
    printf("send_broadcast_nonblocking\n");
    if (ret < 0) {
        printf("send_broadcast_nonblocking\n");
        agent_p agent = createAgent(AgentReader);
        agent->base.myID = 0;
        agent->base.groupID = groupID;
        agent->reader.sizeOfMember = (1U);
        agent->reader.groupKey = (char *)malloc(SIZE_OF_DATA_GROUP_KEY + 1);
        memcpy(agent->reader.groupKey, groupKey,SIZE_OF_DATA_GROUP_KEY + 1);
        MY_AGENT_INFO->agent = agent;
        // perror("send_broadcast_nonblocking");
        // close(socketInfo->recv_sockfd);
        // close(socketInfo->send_sockfd);
        // SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        // exit(1);
        // MY_AGENT_INFO = 0;
        // return;
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
#if DEBUG
                DEBUG_LOG("same ip\n");
#endif
                continue;
            }
            printf("\nReceived from %s: %s\n", sender_ip, buf);
            if(buf[DATA_GROUP_ID] == groupID && memcmp(buf + DATA_GROUP_KEY, groupKey, SIZE_OF_DATA_GROUP_KEY) == 0){
                switch(buf[DATA_REQUEST_TYPE]){
                    case REQUEST_TO_BE_MEMBER:{
                        agent_p agent = createAgent(AgentMember);
                        agent->base.myID    = buf[DATA_MY_ID];
                        agent->base.groupID = buf[DATA_GROUP_ID];
                        agent->member.groupKey = (char *)malloc(SIZE_OF_DATA_GROUP_KEY + 1);
                        memcpy(agent->member.groupKey,buf + DATA_GROUP_KEY,SIZE_OF_DATA_GROUP_KEY + 1);
#if DEBUG
                        DEBUG_LOG("Join Group Success: my id is %d\n",agent->base.myID);
#endif
                        printf("Join Group Success: my id is %d\n",agent->base.myID);
                        MY_AGENT_INFO->agent = agent;
                        return;
                    }
                    case REQUEST_REJECT:{
#if DEBUG
                        DEBUG_LOG("Join Group Reject\n");
#endif
                        close(socketInfo->recv_sockfd);
                        close(socketInfo->send_sockfd);
                        MY_AGENT_INFO = 0;
                        return;
                    }
                }
                break;
            }
        }
    }
    // タイムアウト
    printf("BUILD GROUP\n");
    agent_p agent = createAgent(AgentReader);
    agent->base.myID = 0;
    agent->base.groupID = groupID;
    agent->reader.sizeOfMember = (1U);
    agent->reader.groupKey = (char *)malloc(SIZE_OF_DATA_GROUP_KEY + 1);
    memcpy(agent->reader.groupKey,groupKey,SIZE_OF_DATA_GROUP_KEY + 1);
    MY_AGENT_INFO->agent = agent;
    return;
#undef TIMEOUT
}

void lib_communicate(oop process,oop GM){
    getInt(mpc);int func_num = int_value;
#if DEBUG
    DEBUG_LOG("com lib func_num:%d\n",func_num);
#endif
    switch(func_num){
        case COMMUNICATE_WiFi_SEND_P:{
            communicate_wifi_send(process,GM);
            return;
        }
        case COMMUNICATE_WiFi_BUILD_GROUP_P:{
            communicate_wifi_build_group(process,GM);
            return;
        }
        default:{
            SHICA_FPRINTF(stderr,"this is not happen Primitive_communicate func[%d]\n",func_num);
            exit(1);
        }
    }
    return;
}
#endif //COMMUNICATE_EXECUTE_C