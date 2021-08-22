#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define ERR_EXIT(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0);

void echo_srv(int sock)
{
    struct sockaddr_in peer_addr;
    socklen_t addrlen = sizeof(peer_addr);
    char recv_buf[1024];
    while (1) {
        memset(&recv_buf, 0, sizeof(recv_buf));
        int n = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
                (struct sockaddr*)&peer_addr, &addrlen);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        else if (n > 0) {
            printf("(%s:%d): %s", inet_ntoa(peer_addr.sin_addr),
                    ntohs(peer_addr.sin_port), recv_buf);
            sendto(sock, recv_buf, sizeof(recv_buf), 0,
                    (struct sockaddr*)&peer_addr, addrlen);
        }
    }
}

int main()
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9999);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)))
        ERR_EXIT("bind");

    echo_srv(sock);

    return 0;
}
