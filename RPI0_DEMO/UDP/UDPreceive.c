#include <stdio.h> //printf(), perror()
#include <sys/socket.h> //socket()
#include <sys/ioctl.h> //ioctl()
#include <netinet/in.h> //htons(), inet_addr()
#include <string.h> //memset()
#include <unistd.h> //close()
#include <stdlib.h> //atof()
const int PortNumber = 60000;
const int BufLength = 2048;
int main(int argc, char** argv)
{
    struct sockaddr_in addr , from_addr;
    int sock_df;
    socklen_t from_addr_size;    
    //ソケット作成
    sock_df = socket(AF_INET, SOCK_DGRAM, 0);
    //ソケット作成失敗
    if(sock_df < 0)
    {
        perror("Couldn't make a socket");
        return -1;
    }
    //通信の設定
    addr.sin_family = AF_INET; //IPv4を指定
    addr.sin_port = htons(PortNumber); //ポート番号。ここでは60000を指定
    addr.sin_addr.s_addr = INADDR_ANY; //クライアントを指定せず，全て受け付ける
    //addrとソケットの紐付け
    int bind_status;
    bind_status = bind(sock_df, (struct sockaddr *)&addr, sizeof(addr) );
    if( bind_status < 0)
    {
        perror("bind");
        return -1;
    }
    printf("bind success\n");
    //受信バッファ
    char buf[BufLength];
    memset(buf, 0, sizeof(buf)); //初期化
    while(1)
    {
        int recv_status;
        
        //受信
        recv_status = recvfrom(sock_df, buf, BufLength + 1, 0, (struct sockaddr *)&from_addr, &from_addr_size);
        if ( recv_status < 0)continue;
        buf[BufLength - 1]= '\0'; //もしbufに\0が含まれていなかったときでも正常にprintできるように
        printf("receive data:%s\n", buf);
        //バッファbufの先頭8バイトが送られてきた数値である。
        //そこで、bufの先頭のアドレスをdoubleのアドレスとして読むことで、数値として読み出せる。
        double *valp = (double *)&buf;
        printf("value:%f\n", *valp);
        
    }
    //ソケットをクローズ
    close(sock_df);
    return 0;
}