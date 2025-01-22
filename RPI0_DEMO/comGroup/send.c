#include "broadcast.c"

int main(int argc, char *argv[])
{
    int sockfd, sockfd2;
    struct sockaddr_in broadcast_addr, recv_addr;
    char buffer[16];
    printf("setting nonblocking send\n");
    // 送信ソケット作成とノンブロッキング化
    if (create_broadcast_socket(&sockfd, &broadcast_addr) == 0) {
        set_nonblocking(sockfd);
    }
    printf("setting nonblocking receive\n");
    // 受信ソケット作成とノンブロッキング化
    if (create_receive_socket(&sockfd2, &recv_addr) == 0) {
        set_nonblocking(sockfd2);
    }
    printf("send request\n");
    // 非同期送信
    send_broadcast_nonblocking(sockfd, &broadcast_addr, "Hello\n", 5);
    printf("receive request\n");
    while(1){
        int ret = receive_broadcast_nonblocking(sockfd2, buffer, sizeof(buffer));
        if (ret == 0) {
            printf("Received: %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}