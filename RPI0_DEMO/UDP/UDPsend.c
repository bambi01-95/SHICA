#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BROADCAST_PORT 60000
#define BROADCAST_ADDR "192.168.1.255" // サブネットに合わせて設定
#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in broadcast_addr;
    int yes = 1;

    // 1. ソケットを作成 (UDP)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // 2. SO_BROADCAST オプションを有効にする
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
        perror("setsockopt(SO_BROADCAST)");
        close(sockfd);
        return -1;
    }

    // 3. ブロードキャスト先のアドレス設定
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);

    // 4. 送信する文字列（例として）
    char send_data[BUF_SIZE] = "Hello, broadcast!";
    
    // 5. ブロードキャストパケット送信
    if (sendto(sockfd,
               send_data,
               strlen(send_data),
               0,
               (struct sockaddr *)&broadcast_addr,
               sizeof(broadcast_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        return -1;
    }

    printf("Broadcast message sent: %s\n", send_data);

    // 6. ソケットクローズ
    close(sockfd);
    return 0;
}
