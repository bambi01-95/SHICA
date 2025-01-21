#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BROADCAST_PORT 60000
#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in recv_addr, sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    char buf[BUF_SIZE];

    // 1. ソケットを作成 (UDP)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // 2. 受信用アドレス設定
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(BROADCAST_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY; // すべてのアドレスで待ち受け

    // 3. bind でポートへバインド
    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    printf("Listening on port %d for broadcast...\n", BROADCAST_PORT);

    while (1) {
        // 4. 受信 (ブロッキング)
        memset(buf, 0, BUF_SIZE);
        int ret = recvfrom(sockfd, buf, BUF_SIZE - 1, 0,
                           (struct sockaddr *)&sender_addr,
                           &sender_addr_len);
        if (ret < 0) {
            perror("recvfrom");
            break;
        }
        
        // 5. 受信データの表示
        buf[ret] = '\0'; // 文字列終端
        printf("Received: %s\n", buf);
        printf("From IP: %s\n", inet_ntoa(sender_addr.sin_addr));
    }

    close(sockfd);
    return 0;
}
