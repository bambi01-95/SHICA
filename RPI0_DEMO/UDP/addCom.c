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


#include "agent.c"

#define BROADCAST_PORT 60000
#define BROADCAST_ADDR "172.28.79.255"
#define BUF_SIZE          16


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

int create_broadcast_socket(int *sockfd, struct sockaddr_in *broadcast_addr) {
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

void send_broadcast_nonblocking(int sockfd, struct sockaddr_in *broadcast_addr, const char *data, size_t data_size) {
    ssize_t ret = sendto(sockfd, data, data_size, 0, (struct sockaddr *)broadcast_addr, sizeof(*broadcast_addr));
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("Send would block, try again later\n");
        } else {
            perror("sendto");
        }
    } else {
        printf("Broadcast sent successfully\n");
    }
}


int create_receive_socket(int *sockfd, struct sockaddr_in *recv_addr) {
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
        perror("bind");
        close(*sockfd);
        return -1;
    }

    return 0;
}

void receive_broadcast(int sockfd, char *buffer, size_t buffer_size) {
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    ssize_t ret = recvfrom(sockfd, buffer, buffer_size, MSG_DONTWAIT, (struct sockaddr *)&sender_addr, &addr_len);
    if (ret > 0) {
        printf("Received from %s: %s\n", inet_ntoa(sender_addr.sin_addr), buffer);
    } else {
        perror("recvfrom");
    }
}

void receive_broadcast_nonblocking(int sockfd, char *buffer, size_t buffer_size) {
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    ssize_t ret = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&sender_addr, &addr_len);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("No data available, try again later\n");
        } else {
            perror("recvfrom");
        }
    } else {
        printf("Received from %s: %s\n", inet_ntoa(sender_addr.sin_addr), buffer);
    }
}



int main(){
// Non-blocking UDP broadcast
#if 1
    int sockfd;
    struct sockaddr_in broadcast_addr, recv_addr;
    char buffer[BUF_SIZE];

    // 送信ソケット作成とノンブロッキング化
    if (create_broadcast_socket(&sockfd, &broadcast_addr) == 0) {
        set_nonblocking(sockfd);
    }

    // // 受信ソケット作成とノンブロッキング化
    // if (create_receive_socket(&sockfd, &recv_addr) == 0) {
    //     set_nonblocking(sockfd);
    // }

    // 非同期送信
    send_broadcast_nonblocking(sockfd, &broadcast_addr, "Hello", 5);

    // 非同期受信
    // receive_broadcast_nonblocking(sockfd, buffer, sizeof(buffer));

    close(sockfd);
    return 0;
#endif
// Blocking UDP broadcast
#if 0
    int sockfd;
    struct sockaddr_in broadcast_addr;
    struct sockaddr_in recv_addr;
    char buf[BUF_SIZE];

    if (create_broadcast_socket(&sockfd, &broadcast_addr) < 0) {
        return -1;
    }

    if (create_receive_socket(&sockfd, &recv_addr) < 0) {
        return -1;
    }

    char send_data[BUF_SIZE] = {
        0x01, // Request
        0x01, // Group ID
        0x00, // My ID
        0x00, // Request Member ID
        0x00, // Size of Member
        'K', 'U', 'A', 'S' // Key
    };

    send_broadcast(sockfd, &broadcast_addr, send_data, sizeof(send_data));

    receive_broadcast(sockfd, buf, sizeof(buf));

    close(sockfd);

    return 0;
#endif
}