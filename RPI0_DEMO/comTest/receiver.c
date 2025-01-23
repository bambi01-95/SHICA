#include "broadcast.c"

int main() {
    int recv_sockfd, send_sockfd;
    struct sockaddr_in recv_addr, broadcast_addr, sender_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(sender_addr);

    // 受信ソケット作成とノンブロッキング化
    if (create_receive_socket(&recv_sockfd, &recv_addr) == 0) {
        set_nonblocking(recv_sockfd);
    } else {
        return -1;
    }

    // 送信ソケット作成とノンブロッキング化
    if (create_broadcast_socket(&send_sockfd, &broadcast_addr) == 0) {
        set_nonblocking(send_sockfd);
    } else {
        close(recv_sockfd);
        return -1;
    }

    while (1) {
        // メッセージ受信
        ssize_t ret = recvfrom(recv_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &addr_len);
        if (ret > 0) {
            buffer[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(sender_addr.sin_addr);

            // 自分自身のデータを無視
            if (strcmp(sender_ip, BROADCAST_ADDR) == 0) {
                continue;
            }

            printf("Received from %s: %s\n", sender_ip, buffer);

            // 受信後に「world」を送信
            send_broadcast_nonblocking(send_sockfd, &broadcast_addr, "world", 5);
        }

        usleep(100000); // 100ms待機してループを回す
    }

    close(recv_sockfd);
    close(send_sockfd);
    return 0;
}
