
#ifndef COMMUNICATE_EXECUTE_C
#define COMMUNICATE_EXECUTE_C

#include "communicate-execute.h"
#include "../lib/exgc.c"
#include "../lib/extstr.c"
void init_eve_wifi_receive(oop subcore,oop GM) {
    // 引数チェック

    oop *elements = subcore->SubCore.var->FixArray.elements; 
    char *ipAddr  = getChild(elements[0],_String,value);
    int portNum  = _Integer_value(elements[1]);
    int groupID  = _Integer_value(elements[2]);
    char *groupKey = getChild(elements[3],_String,value);

    // 引数チェック
    if (ipAddr == NULL || groupKey == NULL) {
        SHICA_FPRINTF(stderr, "Invalid argument\n");
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
    }
#if DEBUG
    DEBUG_LOG("ipAddr:%s portNum:%d groupID:%d groupKey:%s\n",ipAddr,portNum,groupID,groupKey);
#endif

#if MSGC  
    // struct ExternMemory *em = newExternMemory(8);
    // subcore->SubCore.em = em;
    // struct SocketInfo *socketInfo = (struct SocketInfo *)gc_extern_alloc(em,sizeof(struct SocketInfo));
    
    struct SocketInfo *socketInfo = (struct SocketInfo *)gc_alloc(sizeof(struct SocketInfo));
    socketInfo->type = registerExternType(STRUCT_SOCKET_INFO_TYPE);
#else
    struct SocketInfo *socketInfo = (struct SocketInfo *)malloc(sizeof(struct SocketInfo));
#endif
    socketInfo->addr_len = sizeof(socketInfo->sender_addr);

    // 自身のネットワークIPアドレスを取得
    if (get_network_ip(socketInfo->own_ip, sizeof(socketInfo->own_ip)) != 0) {
        SHICA_FPRINTF(stderr, "Failed to get own IP address\n");
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
        subcore->SubCore.em = 0;
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
        subcore->SubCore.em = 0;
        return ;
    }

    // 送信ソケット作成とブロードキャストアドレス設定
    if (create_broadcast_socket(&socketInfo->send_sockfd, &socketInfo->broadcast_addr,ipAddr,portNum) != 0) {
        close(socketInfo->recv_sockfd);
        SHICA_PRINTF("%s line %d\n",__FILE__,__LINE__);
        exit(1);
        subcore->SubCore.em = 0;
        return ;
    }

#if MSGC
    // struct AgentInfo *MY_AGENT_INFO = (struct AgentInfo *)gc_extern_alloc(em,sizeof(struct AgentInfo));
    struct AgentInfo *MY_AGENT_INFO = (struct AgentInfo *)gc_alloc(sizeof(struct AgentInfo));
    MY_AGENT_INFO->type = registerExternType(STRUCT_AGENT_INFO_TYPE);
#else
    struct AgentInfo *MY_AGENT_INFO = (struct AgentInfo *)malloc(sizeof(struct AgentInfo));
#endif
    MY_AGENT_INFO->socket = socketInfo;
  
#define TIMEOUT 1
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    buf[DATA_REQUEST_TYPE] = REQUEST_JOIN;
    buf[DATA_GROUP_ID] = groupID;
    buf[DATA_REQUEST_SENDER_ID] = 0;
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
        memcpy(agent->reader.groupKey, groupKey,SIZE_OF_DATA_GROUP_KEY + 1);
        MY_AGENT_INFO->agent = agent;//remove me after adapt em
        subcore->SubCore.any = (void *)MY_AGENT_INFO;
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
#if DEBUG
            DEBUG_LOG("\nReceived from %s: %s\n", sender_ip, buf);
#else
            SHICA_PRINTF("\nReceived from %s: %s\n", sender_ip, buf);
#endif
            if(buf[DATA_GROUP_ID] == groupID && memcmp(buf + DATA_GROUP_KEY, groupKey, SIZE_OF_DATA_GROUP_KEY) == 0){
                switch(buf[DATA_REQUEST_TYPE]){
                    case REQUEST_TO_BE_MEMBER:{
                        agent_p agent = createAgent(AgentMember);
                        agent->base.myID    = buf[DATA_REQUEST_SENDER_ID];
                        agent->base.groupID = buf[DATA_GROUP_ID];
                        memcpy(agent->member.groupKey,buf + DATA_GROUP_KEY,SIZE_OF_DATA_GROUP_KEY + 1);
#if DEBUG
                        DEBUG_LOG("Join Group Success: my id is %d\n",agent->base.myID);
#endif
                        printf("Join Group Success: my id is %d\n",agent->base.myID);
                        MY_AGENT_INFO->agent = agent;//remove me after adapt em
                        subcore->SubCore.any = (void *)MY_AGENT_INFO;
                        return;
                    }
                    case REQUEST_REJECT:{
#if DEBUG
                        DEBUG_LOG("Join Group Reject\n");
#endif
                        close(socketInfo->recv_sockfd);
                        close(socketInfo->send_sockfd);
                        MY_AGENT_INFO = 0;//remove me after em
                        subcore->SubCore.em = 0;
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
    agent->base.myID = 1;
    agent->base.groupID = groupID;
    agent->reader.sizeOfMember = (1U);
// #if MSGC
//     agent->reader.groupKey = (char *)gc_extern_alloc(em,SIZE_OF_DATA_GROUP_KEY + 1);
// #else
//     agent->reader.groupKey = (char *)malloc(SIZE_OF_DATA_GROUP_KEY + 1);
// #endif
    setAgentGroupKey(agent,groupKey);
    MY_AGENT_INFO->agent = agent;//remove me after adapt em
    subcore->SubCore.any = (void *)MY_AGENT_INFO;
    return;
#undef TIMEOUT
}


oop eve_wifi_receive(oop core,oop GM){
    //corret 
#if SBC
        struct AgentInfo *MY_AGENT_INFO = (struct AgentInfo *)core->SubCore.any;
        struct SocketInfo *socketInfo = MY_AGENT_INFO->socket;//remove me after adapt em
        agent_p agent = MY_AGENT_INFO->agent;
        // メッセージ受信
        char buffer[BUF_SIZE];
        ssize_t ret = recvfrom(socketInfo->recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&socketInfo->sender_addr, &socketInfo->addr_len);
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
                        #if DEBUG
                        DEBUG_LOG("REQUEST_JOIN");
                        printAgentData(agent);
                        #endif

                        if(agent->base.agent_type == AgentReader){
                            int list = agent->reader.sizeOfMember;
                            int newId = 1;
                            while(list){
                                if((list & 0x01) == 0){
                                    break;
                                }
                                newId++;
                                list >>= 1;
                            }
                            if(newId > MAX_MEMBER_SIZE){
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
                                agent->reader.sizeOfMember |= (1U <<(newId-1));
    #if DEBUG
                                DEBUG_LOG("current Member is %d\n",agent->reader.sizeOfMember);
    #endif
                                buffer[DATA_REQUEST_TYPE] = REQUEST_TO_BE_MEMBER;
                                buffer[DATA_REQUEST_SENDER_ID]        = newId;
                                buffer[DATA_REQUEST_MEMEBER_ID] = (1 << (newId-1));
                                buffer[DATA_SIZE_OF_MEMBER] = agent->reader.sizeOfMember;
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
                        #if DEBUG
                        DEBUG_LOG("REQUEST_LEAVE\n");
                        #endif
                        if(agent->base.agent_type == AgentReader){
                            agent->reader.sizeOfMember &= ~(1 << buffer[DATA_REQUEST_SENDER_ID]); 
                            //send SUCCESS
                            buffer[DATA_REQUEST_TYPE] = REQUEST_SUCCESS;
                            buffer[DATA_REQUEST_SENDER_ID] = agent->base.myID;
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
                        #if DEBUG
                        DEBUG_LOG("REQUEST_TO_BE_READER\n");
                        #endif
                        if(agent->base.agent_type == AgentMember){
                            if((buffer[DATA_REQUEST_MEMEBER_ID]>> (agent->base.myID-1) & 1) == 1){//checking for me or not
                                agent_p newAgent = createAgent(AgentReader);
                                newAgent->reader.sizeOfMember = buffer[DATA_SIZE_OF_MEMBER] & ~(1 << (agent->base.myID-1));//remove my id
                                newAgent->base.myID = 0;
                                newAgent->base.groupID = buffer[DATA_GROUP_ID];
                                printf("current Member is %d\n",newAgent->reader.sizeOfMember);
                                setAgentGroupKey(newAgent,agent->reader.groupKey);

                                buffer[DATA_REQUEST_TYPE] = REQUEST_SUCCESS;
                                buffer[DATA_REQUEST_SENDER_ID]        = agent->base.myID;
                                int sent = send_broadcast_nonblocking(socketInfo->send_sockfd, &socketInfo->broadcast_addr, buffer, BUF_SIZE);
                                if (sent < 0) {
                                    perror("sendto");
                                } else {
                                    printf("Replied to %s: SUCCESS\n", sender_ip);
                                }
                                MY_AGENT_INFO->agent = newAgent;
                                core->SubCore.any = (void *)MY_AGENT_INFO;
                            }
                        }
                        return core;
                    }
                    case REQUEST_TRIGER:{
                        #if DEBUG
                        DEBUG_LOG("REQUEST_TRIGER\n");
                        #endif
                        if((buffer[DATA_REQUEST_MEMEBER_ID]>> (agent->base.myID-1) & 1) == 1){
                            /* trigger data */
                            int isOnce = 0;
                            evalEventArgsThread->Thread.stack->Array.size = 1;//1:basepoint
                            unsigned char value = buffer[DATA_REQUEST_MEMEBER_ID];
                            for(int thread_i = 0;thread_i<core->Core.size;thread_i++){
                                int isFalse = 0;
                                oop thread = core->Core.threads[thread_i];
                                //<引数の評価>/<Evaluation of arguments>
                                if(thread->Thread.condRelPos != 0){
                                    if(isOnce == 0){
                                        Array_push(evalEventArgsThread->Thread.stack,_newInteger(buffer[DATA_DATA]));//arg 3
                                        if(value== ALL_MEMBER_ID){
                                            Array_push(evalEventArgsThread->Thread.stack,_newInteger(0));//ALL MEMBER:0
                                        }else if(value == ((1U) << (agent->base.myID -1))){
                                            Array_push(evalEventArgsThread->Thread.stack,_newInteger(1));//MYSELF:1
                                        }else{
                                            Array_push(evalEventArgsThread->Thread.stack,_newInteger(2));//OTHER:2
                                        }
                                        Array_push(evalEventArgsThread->Thread.stack,_newInteger((int)buffer[DATA_REQUEST_SENDER_ID]));//arg1
                                        isOnce = 1;
                                    }else{
                                        evalEventArgsThread->Thread.stack->Array.size = 4;
                                    }
                                    evalEventArgsThread->Thread.pc = thread->Thread.base + thread->Thread.condRelPos;
                                    for(;;){
                                        FLAG flag = sub_execute(evalEventArgsThread,GM);
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
                                    oop data = newArray(3);
                                    Array_push(data,_newInteger(buffer[DATA_DATA]));
                                    if(value == ALL_MEMBER_ID){
                                        Array_push(data,_newInteger(0));
                                    }else if(value == ((1U) << (agent->base.myID -1))){
                                        Array_push(data,_newInteger(1));
                                    }else{
                                        Array_push(data,_newInteger(2));
                                    }
                                    Array_push(data,_newInteger((int)buffer[DATA_REQUEST_SENDER_ID]));
                                    gc_popRoots(1);
                                    enqueue(thread->Thread.queue,data);
                                }
                            #if DEBUG
                                else{
                                    DEBUG_LOG("not trigger\n");//remove
                                }
                            #endif
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
#else //SBC
    SHICA_PRINTF("eve_wifi_receive\n");
#endif
    return core;
}

// Function to receive a message from a specific address and port
oop  wifiReceived(oop core) {
    char* from_addr = core->Core.vd->VarIS.v_s1;
    int port        = core->Core.vd->VarIS.v_i1;

    int sockfd;
    struct sockaddr_in recv_addr, sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    static char buffer[1024];

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return NULL;
    }

    // Set non-blocking mode
    set_nonblocking(sockfd);

    // Bind to the specified port
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(port);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return NULL;
    }

    // Receive the message
    ssize_t ret = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_len);
    if (ret < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom");
        }
        close(sockfd);
        return NULL;
    }

    buffer[ret] = '\0'; // Null-terminate the received message
    strcpy(from_addr, inet_ntoa(sender_addr.sin_addr)); // Save the sender's address

    close(sockfd);
    // return buffer;
    return core;
}

// Function to receive a broadcast message
oop wifiBrReceived(oop core) {
    return wifiReceived(core); // Reuse the wifiReceived function
}


oop Event_communicate(int eve_num,oop stack){
    gc_pushRoot((void*)&stack);
    GC_PUSH(oop,core,0);
    switch(eve_num){
        case COMMUNICATE_WiFi_RECEIVE_E:{
            core = newCore(VarIS);
            core->Core.vd->VarIS.v_s1 = 0;
            core->Core.vd->VarIS.v_i1 = 0;
            core->Core.func = &eve_wifi_receive;
            break;
        }
        case COMMUNICATE_WiFi_BROADCAST_RECEIVE_E:{
            core = newCore(VarIS);
            core->Core.vd->VarIS.v_s1 = 0;
            core->Core.vd->VarIS.v_i1 = 0;
            core->Core.func = &eve_wifi_receive;
            break;
        }
        case COMMUNICATE_WiFi_GROUP_RECEIVE_E:{
            oop subcore = newSubCore();
            subcore->SubCore.var = newFixArray(4);
            subcore->SubCore.var->FixArray.elements[0] = Array_pop(stack);//String  address
            subcore->SubCore.var->FixArray.elements[1] = Array_pop(stack);//Integer port
            subcore->SubCore.var->FixArray.elements[2] = Array_pop(stack);//Integer groupID
            subcore->SubCore.var->FixArray.elements[3] = Array_pop(stack);//String  groupKey
            init_eve_wifi_receive(subcore,stack);
            subcore->SubCore.func = &eve_wifi_receive;
            core = subcore;
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



void communicate_wifi_group_send(oop process,oop GM){
    oop subcore = Array_pop(mstack);
    void* any = subcore->SubCore.any;
    if(any == 0){
        fatal("should be initialize wifiGroupReceive()\n");
        return;
    }
    getInt(mpc);int size_args = int_value;
    int sendToId = api();
    int value = api();
#if DEBUG
    DEBUG_LOG("sendToId:%d value:%d\n",sendToId,value);
#endif
    struct AgentInfo *MY_AGENT_INFO = (struct AgentInfo *)any;
    if(MY_AGENT_INFO == 0){
        SHICA_PRINTF("It doesn't belong to some group now\n");
        return;
    }
    struct SocketInfo *socketInfo = MY_AGENT_INFO->socket;
    agent_p agent = MY_AGENT_INFO->agent;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    buf[DATA_REQUEST_TYPE] = REQUEST_TRIGER;
    buf[DATA_GROUP_ID] = agent->base.groupID;
    buf[DATA_REQUEST_SENDER_ID] = agent->base.myID;
    printf("myid %d\n",agent->base.myID);
    if(sendToId < 0){
        buf[DATA_REQUEST_MEMEBER_ID] = ALL_MEMBER_ID;//all member: 11111111
    }else{
        buf[DATA_REQUEST_MEMEBER_ID] = (char)sendToId;//0:reader or other: member
    }   
    buf[DATA_SIZE_OF_MEMBER] = 1;
    memcpy(buf + DATA_GROUP_KEY, getAgentGroupKey(agent), 4);
    buf[DATA_DATA] = (char)value;

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
   SHICA_FPRINTF(stderr, "now unsupported function\n");
   exit(1);
    return;
}

// Function to send a message to a specific address and port
void communicate_wifi_send(oop process,oop GM) {
    getInt(mpc);int size_args = int_value;
    char* addr = aps();
    int   port = api();
    char* msg = aps();

    int sockfd;
    struct sockaddr_in dest_addr;

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return;
    }

    // Set non-blocking mode
    set_nonblocking(sockfd);

    // Set destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, addr, &dest_addr.sin_addr);

    // Send the message
    ssize_t ret = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0) {
        perror("sendto");
    } else {
        printf("Message sent successfully\n");
    }

    // Close the socket
    close(sockfd);
    return;
}

// Function to broadcast a message to a specific port
void communicate_wifi_broadcast(oop process,oop GM) {
    getInt(mpc);int size_args = int_value;
    char* addr = aps();
    int   port = api();
    char* msg = aps();

    int sockfd;
    struct sockaddr_in broadcast_addr;
    int yes = 1;

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return;
    }

    // Enable broadcast mode
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return;
    }

    // Set non-blocking mode
    set_nonblocking(sockfd);

    // Set broadcast address
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = inet_addr(addr);

    // Send the broadcast message
    ssize_t ret = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
    if (ret < 0) {
        perror("sendto");
    } else {
        printf("Broadcast message sent successfully\n");
    }

    // Close the socket
    close(sockfd);
    return ;
}



void lib_communicate(oop process,oop GM){
    getInt(mpc);int func_num = int_value;
#if DEBUG
    DEBUG_LOG("com lib func_num:%d\n",func_num);
#endif
    switch(func_num){
        case COMMUNICATE_WiFi_SEND_P:{
#if DEBUG
            DEBUG_LOG("COMMUNICATE_WiFi_SEND_P: if it is work REMOVEME\n");
#endif
            communicate_wifi_send(process,GM);
            return;
        }
        case COMMUNICATE_WiFi_BROADCAST_P:{
#if DEBUG
            DEBUG_LOG("COMMUNICATE_WiFi_SEND_P: if it is work REMOVEME\n");
#endif
            communicate_wifi_broadcast(process,GM);
            return;
        }
        case COMMUNICATE_WiFi_GROUP_BROADCAST_P:{
            communicate_wifi_group_send(process,GM);
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