#include "broadcast.c"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


#include <netdb.h> // 追加: getnameinfo と NI_NUMERICHOST のため
#include <net/if.h> // IFF_LOOPBACKのため
#include "broadcast.c"

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

int main() {
    int sockfd;
    struct sockaddr_in broadcast_addr, recv_addr, sender_addr;
    char buffer[BUF_SIZE];
    char own_ip[INET_ADDRSTRLEN];
    socklen_t addr_len = sizeof(sender_addr);

    // 自身のネットワークIPアドレスを取得
    if (get_network_ip(own_ip, sizeof(own_ip)) != 0) {
        fprintf(stderr, "Failed to get own IP address\n");
        return -1;
    }
    printf("Own IP: %s\n", own_ip);

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
    while (1) {
        ssize_t ret = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &addr_len);
        if (ret > 0) {
            buffer[ret] = '\0'; // Null終端
            char *sender_ip = inet_ntoa(sender_addr.sin_addr);

            // 自分自身の送信データを無視
            if (strcmp(sender_ip, own_ip) == 0) {
                continue;
            }

            printf("Received from %s: %s\n", sender_ip, buffer);
        }

        usleep(100000); // 100ms待機
    }

    close(sockfd);
    return 0;
}

