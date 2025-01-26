
#ifndef  BROADCAST_C
#define BROADCAST_C



#define DEBIF if(0)



// 自身のネットワークインターフェースIPアドレスを取得する関数
int get_network_ip(char *ip_buffer, size_t buffer_size) {
    struct ifaddrs *ifaddr, *ifa;
    int family;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) { // ループバック以外のIPv4アドレス
            int result = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                     ip_buffer, buffer_size, NULL, 0, NI_NUMERICHOST);
            if (result == 0) {
                break; // 最初の有効なネットワークインターフェースを取得
            } else {
                fprintf(stderr, "getnameinfo failed: %s\n", gai_strerror(result));
            }
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}

void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
    }
}

int create_broadcast_socket(int *sockfd, struct sockaddr_in *broadcast_addr, const char *BROADCAST_ADDR, int BROADCAST_PORT) {
    int yes = 1;

    // ソケット作成
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // ブロードキャスト送信を有効化
    if (setsockopt(*sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
        perror("setsockopt(SO_BROADCAST)");
        close(*sockfd);
        return -1;
    }

    // アドレス設定
    memset(broadcast_addr, 0, sizeof(*broadcast_addr));
    broadcast_addr->sin_family = AF_INET;
    broadcast_addr->sin_port = htons(BROADCAST_PORT);
    broadcast_addr->sin_addr.s_addr = inet_addr(BROADCAST_ADDR);

    return 0;
}

void send_broadcast(int sockfd, struct sockaddr_in *broadcast_addr, const char *data, size_t data_size) {
    if (sendto(sockfd, data, data_size, 0, (struct sockaddr *)broadcast_addr, sizeof(*broadcast_addr)) < 0) {
        perror("sendto");
    } else {
        printf("Broadcast sent successfully\n");
    }
}


int send_broadcast_nonblocking(int sockfd, struct sockaddr_in *broadcast_addr, const char *data, size_t data_size) {
    ssize_t ret = sendto(sockfd, data, data_size, 0, (struct sockaddr *)broadcast_addr, sizeof(*broadcast_addr));
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {// EAGAIN: リソースが利用可能になるまで待つ必要がある場合
            printf("Send would block, try again later\n");
            return -1;
        } else {                                    // EWOULDBLOCK: リソースが利用可能になるまで待つ必要がある場合
            perror("sendto");
            return -1;
        }
    } else {
        DEBIF printf("Broadcast sent successfully\n");
        return 0;
    }
}


int create_receive_socket(int *sockfd, struct sockaddr_in *recv_addr, int BROADCAST_PORT) {
    // ソケット作成
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // アドレス設定
    memset(recv_addr, 0, sizeof(*recv_addr));
    recv_addr->sin_family = AF_INET;
    recv_addr->sin_port = htons(BROADCAST_PORT);
    recv_addr->sin_addr.s_addr = INADDR_ANY;

    // ポートにバインド
    if (bind(*sockfd, (struct sockaddr *)recv_addr, sizeof(*recv_addr)) < 0) {
        printf("bind error\n");
        return -1;
        perror("bind");
        close(*sockfd);
        return -1;
    }

    return 0;
}

int receive_broadcast(int sockfd, char *buffer, size_t buffer_size) {
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    ssize_t ret = recvfrom(sockfd, buffer, buffer_size, MSG_DONTWAIT, (struct sockaddr *)&sender_addr, &addr_len);
    if (ret > 0) {
        DEBIF printf("Received from %s: %s\n", inet_ntoa(sender_addr.sin_addr), buffer);
        return 0;
    } else {
        DEBIF perror("recvfrom");
        return -1;
    }
}

int receive_broadcast_nonblocking(int sockfd, char *buffer, size_t buffer_size) {
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    ssize_t ret = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&sender_addr, &addr_len);
    if(ret>0){
        DEBIF printf("Received from %s: %s\n", inet_ntoa(sender_addr.sin_addr), buffer);
        return 0;
    }else{
        DEBIF perror("recvfrom");
        return -1;
    }
#if 0
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("No data available, try again later\n");
        } else {
            perror("recvfrom");
        }
    } else {
        printf("Received from %s: %s\n", inet_ntoa(sender_addr.sin_addr), buffer);
    }
#endif
}

#endif // BROADCAST_C