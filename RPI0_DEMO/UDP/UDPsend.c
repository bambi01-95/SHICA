#include <stdio.h> //printf(), perror()
#include <sys/socket.h> //sendto(), socket()
#include <unistd.h> //close()
#include <arpa/inet.h> //htons(), inet_addr()
#include <time.h> //time(), localtime()
const int PortNumber = 60000;
const char *IPaddress = "169.254.46.83";
int main(int argc, char** argv)
{
    struct sockaddr_in addr;
    int sock_df;
    sock_df = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_df < 0)
    {
        perror("Couldn't make a socket");
        return -1;
    }
    // 通信の設定
    addr.sin_family = AF_INET; //IPv4を指定
    addr.sin_port = htons(PortNumber); //ポート番号。ここでは60000を指定
    addr.sin_addr.s_addr = inet_addr(IPaddress); //サーバー側のアドレス
    ssize_t send_status;
    double sendval = 3.141592654;
    while(1)
    {
        // 送信
        send_status = sendto(sock_df, &sendval , sizeof(double) , 0,
                    (struct sockaddr *)&addr, sizeof(addr) );
        // 送信失敗
        if(send_status < 0)
        {
            perror("send error");
            return -1;
        }
        printf("send:%f\n",sendval);
        // 1秒wait
        time_t t;
        time(&t);
        struct tm start, *now;
        now = localtime(&t);
        start = *now; // nowの値をコピー
        while(now->tm_sec == start.tm_sec)
        {
            time(&t);
            localtime(&t);
        }    
    }
    close(sock_df);
    return 0;
}