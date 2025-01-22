#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>


#define BROADCAST_PORT 60000
#define BROADCAST_ADDR "255.255.255.255"
#define BUF_SIZE 1024
#define MESSAGE "Hello, Broadcast!"
#include "broadcast.c"

int main() {
    int sockfd;
    struct sockaddr_in broadcast_addr, recv_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(recv_addr);
    int yes = 1;

    // ソケットの作成
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // ブロードキャストオプションを有効にする
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // アドレスの設定（送信用）
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);

    // アドレスの設定（受信用）
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(BROADCAST_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    // ソケットをバインド
    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // ソケットをノンブロッキングに設定
    set_nonblocking(sockfd);

    printf("Alternating between sending and receiving every second using time()...\n");
    int flag = 0;
    while (1) {
        time_t start_time = time(NULL);
        while (time(NULL) - start_time < 1) {
            if (sendto(sockfd, MESSAGE, strlen(MESSAGE), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                perror("sendto");
            } 
        }

        start_time = time(NULL);

        // 受信処理
        while (time(NULL) - start_time < 1) {
            ssize_t ret = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0, (struct sockaddr *)&recv_addr, &addr_len);
            if (ret > 0) {
                buffer[ret] = '\0';
                printf("Received broadcast message: %s\n", buffer);
            } 
        }
    }

    close(sockfd);
    return 0;
}
