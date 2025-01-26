#include 
// MEMO

// グループ退会リクエストを送信する関数
agent_p LeaveGroupRequest(agent_p agent){
    int sockfd;
    struct sockaddr_in broadcast_addr;
    // 5秒間応答を待つ
    socklen_t broadcast_addr_len = sizeof(broadcast_addr);

    // 初期化処理
    if (init(&sockfd, &broadcast_addr, 1) < 0) {
        return agent;
    }

    switch(agent->base.type){
        case AgentMember:{
            // 送信データの準備
            char send_data[BUF_SIZE] = {
                REQUEST_LEAVE, // Request
                agent->base.groupID,      // Group ID
                agent->base.myID,            // My ID
                agent->reader.sizeOfMember,            // Size of Member
                agent->reader.groupKey[0], agent->reader.groupKey[1], agent->reader.groupKey[2], agent->reader.groupKey[3] // Key
            };
            // ブロードキャストパケット送信
            if (sendto(sockfd, send_data, sizeof(send_data), 0,
                    (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                perror("sendto");
            }
            break;
        }
        case AgentReader:{
            // 送信データの準備
            char send_data[BUF_SIZE] = {
                REQUEST_TO_BE_READER, // Request
                agent->base.groupID,      // Group ID
                agent->base.myID,            // My ID
                agent->reader.sizeOfMember,            // Size of Member
                agent->reader.groupKey[0], agent->reader.groupKey[1], agent->reader.groupKey[2], agent->reader.groupKey[3] // Key
            };
            // ブロードキャストパケット送信
            if (sendto(sockfd, send_data, sizeof(send_data), 0,
                    (struct sockaddr *)&broadcast_addr, broadcast_addr_len) < 0) {
                perror("sendto");
            }
            break;
        }

        default:
            return agent;
    }
    close(sockfd);

    return agent;
}

// ブロードキャストメッセージを受信する関数
agent_p triBroadMSG(agent_p agent){
    int sockfd;
    struct sockaddr_in broadcast_addr;
    socklen_t broadcast_addr_len = sizeof(broadcast_addr);
    char buf[BUF_SIZE];

    // // 初期化処理
    // if (init(&sockfd, &broadcast_addr, 1) < 0) {
    //     return agent;
    // }

    // // ポートへバインド
    // if (bind(sockfd, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
    //     perror("bind");
    //     close(sockfd);
    //     return agent;
    // }

    int ret = recvfrom(sockfd, buf, BUF_SIZE - 1, MSG_DONTWAIT,
                        (struct sockaddr *)&broadcast_addr, &broadcast_addr_len);

    if (ret < 0) {  
        return agent;
    }

    if (buf[2] == agent->base.groupID || memcmp(buf + DATA_GROUP_KEY, agent->member.groupKey, 4) == 0){
        if((buf[3] >> agent->base.myID) & (1U) == 1){
            switch(buf[0]){
                case REQUEST_TO_BE_READER:{
                    printf("I'm new Reader\n");
                    agent_p newAgent = createAgent(AgentReader);
                    newAgent->base.groupID = buf[1];
                    newAgent->base.myID = buf[3];
                    newAgent->reader.sizeOfMember = buf[4] & ~(1U << buf[2]); // 退会者を削除
                    newAgent->reader.groupKey = strdup(buf + DATA_GROUP_KEY);
                    return newAgent;
                }
            }
        }
    }
    close(sockfd);
}