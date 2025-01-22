#include "broadcast.c"
//receive_broadcast_nonblocking 

typedef enum RequestType{
    REQUEST_UNDEFINED,
    REQUEST_JOIN,
    REQUEST_LEAVE,
    REQUEST_TO_BE_MEMBER,
    REQUEST_TO_BE_READER,
    REQUEST_SUCCESS,
    REQUEST_REJECT,
    REQUEST_MOVE,
}request_t;

int main(int argc, char *argv[])
{
    int sockfd;
    int sockfd2;
    struct sockaddr_in  recv_addr;
    struct sockaddr_in broadcast_addr;
    char buffer[16];


    // 受信ソケット作成とノンブロッキング化
    if (create_receive_socket(&sockfd, &recv_addr) == 0) {
        set_nonblocking(sockfd);
    }

    // 送信ソケット作成とノンブロッキング化
    if (create_broadcast_socket(&sockfd2, &broadcast_addr) == 0) {
        set_nonblocking(sockfd2);
    }

    int groudID = 111;
    char *gKey = "KUAS";
    union Agent *agent = NULL;

    char requestMsg[BUF_SIZE] = {
        REQUEST_JOIN, // Request
        groudID, // Group ID
        1, // My ID
        0, // Request Member ID
        0, // Size of Member
        gKey[0], gKey[1], gKey[2], gKey[3] // Key
    };

    while(1){
        int ret = receive_broadcast_nonblocking(sockfd, buffer, sizeof(buffer));
        if (ret == 0) {
            printf("Received: %s\n", buffer);
            int ret = send_broadcast_nonblocking(sockfd2, &broadcast_addr, "World\n", 6);
            if (ret < 0) {
                perror("send_broadcast_nonblocking");
            }
            else {
                printf("Broadcast sent successfully\n");
            }
        }
    }


    close(sockfd);
    return 0;
}