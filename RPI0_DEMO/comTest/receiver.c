#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h> // 追加: getnameinfo と NI_NUMERICHOST のため
#include "broadcast.c"

// 自身のIPアドレスを取得する関数
int get_own_ip(char *ip_buffer, size_t buffer_size) {
    struct ifaddrs *ifaddr, *ifa;
    int family;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) { // IPv4のみを対象
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                        ip_buffer, buffer_size, NULL, 0, NI_NUMERICHOST);
            break; // 最初のIPv4アドレスを取得
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}

int main() {
    int recv_sockfd, send_sockfd;
    struct sockaddr_in recv_addr, broadcast_addr, sender_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(sender_addr);
    char own_ip[INET_ADDRSTRLEN];

    // 自身のIPアドレスを取得
    if (get_own_ip(own_ip, sizeof(own_ip)) != 0) {
        fprintf(stderr, "Failed to get own IP address\n");
        return -1;
    }
    printf("Own IP: %s\n", own_ip);

    // 受信ソケット作成
    if (create_receive_socket(&recv_sockfd, &recv_addr) != 0) {
        return -1;
    }

    // 送信ソケット作成
    if (create_broadcast_socket(&send_sockfd, &broadcast_addr) != 0) {
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
            if (strcmp(sender_ip, own_ip) == 0) {
                continue;
            }

            printf("Received from %s: %s\n", sender_ip, buffer);

            // 受信後に「world」を送信
            send_broadcast_nonblocking(send_sockfd, &broadcast_addr, "world", 5);
        }

        usleep(100000); // 100ms待機
    }

    close(recv_sockfd);
    close(send_sockfd);
    return 0;
}
