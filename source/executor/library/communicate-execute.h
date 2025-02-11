
#ifndef COMMUNICATE_EXECUTE_H
#define COMMUNICATE_EXECUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h> // 追加: getnameinfo と NI_NUMERICHOST のため
#include <net/if.h> // IFF_LOOPBACKのため
#include <ifaddrs.h>

#include "../object.c"
#include "../../common/liblist/communicate.h"
#include "../lib/broadcast.c"
#include "../agent.c"

// request type
#define  REQUEST_UNDEFINED      0x00
#define  REQUEST_JOIN           0x01
#define  REQUEST_LEAVE          0x02
#define  REQUEST_TO_BE_MEMBER   0x03
#define  REQUEST_TO_BE_READER   0x04
#define  REQUEST_SUCCESS        0x05
#define  REQUEST_REJECT         0x06
#define  REQUEST_MOVE           0x07
#define  REQUEST_TRIGER         0x08


// broadcast data index
#define DATA_REQUEST_TYPE        0x00
#define DATA_GROUP_ID            0x01
#define DATA_MY_ID               0x02
#define DATA_REQUEST_MEMEBER_ID  0x03
#define DATA_SIZE_OF_MEMBER      0x04
#define DATA_GROUP_KEY           0x05
#define DATA_DATA                0x09

// group key size
#define SIZE_OF_DATA_GROUP_KEY   0x04

// member size
#define MAX_MEMBER_SIZE          0x08
#define ALL_MEMBER_ID            0xFF

#ifdef BUF_SIZE
#undef BUF_SIZE
#endif
#define BUF_SIZE                16


#define STRUCT_SOCKET_INFO_TYPE 0b000
struct SocketInfo{
    enum Type type;
    int recv_sockfd, send_sockfd;
    struct sockaddr_in recv_addr, broadcast_addr, sender_addr;
    socklen_t addr_len;
    char own_ip[INET_ADDRSTRLEN];
};
#define STRUCT_AGENT_INFO_TYPE 0b110
struct AgentInfo{
    enum Type type;
    agent_p agent;//agent_p is struct Agent*
    struct SocketInfo *socket;
};
#endif