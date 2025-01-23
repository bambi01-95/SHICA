#include "broadcast.c"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>  // getnameinfoとNI_NUMERICHOSTのため
#include <net/if.h> // IFF_LOOPBACKのため

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
    int recv_sockfd, send_sockfd;
    struct sockaddr_in recv_addr, broadcast_addr, sender_addr;
    char buffer[BUF_SIZE];
    char own_ip[INET_ADDRSTRLEN];
    socklen_t addr_len = sizeof(sender_addr);

    // 自身のネットワークIPアドレスを取得
    if (get_network_ip(own_ip, sizeof(own_ip)) != 0) {
        fprintf(stderr, "Failed to get own IP address\n");
        return -1;
    }
    printf("Own IP: %s\n", own_ip);

    // 受信ソケット作成とノンブロッキング化
    if (create_receive_socket(&recv_sockfd, &recv_addr) == 0) {
        set_nonblocking(recv_sockfd);
    } else {
        return -1;
    }

    // 送信ソケット作成とブロードキャストアドレス設定
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

            // 自分自身の送信データを無視
            if (strcmp(sender_ip, own_ip) == 0) {
                continue;
            }

            printf("Received from %s: %s\n", sender_ip, buffer);

            // 受信したアドレスに "world" を送信
            ssize_t sent = sendto(send_sockfd, "world", 5, 0, (struct sockaddr *)&sender_addr, addr_len);
            if (sent < 0) {
                perror("sendto");
            } else {
                printf("Replied to %s: world\n", sender_ip);
            }

            // "join" をブロードキャストで送信
            ssize_t broadcast_sent = send_broadcast_nonblocking(send_sockfd, &broadcast_addr, "join", 4);
            if (broadcast_sent < 0) {
                printf("Failed to broadcast join\n");
            } else {
                printf("Broadcasted: join\n");
            }
        }

        usleep(100000); // 100ms待機してループを回す
    }

    close(recv_sockfd);
    close(send_sockfd);
    return 0;
}
