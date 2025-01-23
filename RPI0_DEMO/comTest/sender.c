#include "broadcast.c"

int main() {
    int sockfd;
    struct sockaddr_in broadcast_addr, recv_addr;
    char buffer[BUF_SIZE];

    // 送信ソケット作成とノンブロッキング化
    if (create_broadcast_socket(&sockfd, &broadcast_addr) == 0) {
        set_nonblocking(sockfd);
    } else {
        return -1;
    }

    // 受信アドレス設定
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(BROADCAST_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    // ポートにバインド
    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    // メッセージ送信
    send_broadcast_nonblocking(sockfd, &broadcast_addr, "Hello", 5);

    // メッセージ受信
    if (receive_broadcast_nonblocking(sockfd, buffer, sizeof(buffer)) == 0) {
        printf("Received: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}

