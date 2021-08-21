#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "sysutil.h"
int main()
{
    int listen_fd;
    if ((listen_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket fail\n");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((bind(listen_fd, (struct sockaddr*) &server_addr, sizeof(server_addr))) < 0) {
        printf("bind fail\n");
        exit(-1);
    }

    if (listen(listen_fd, SOMAXCONN)) {
        printf("listen fail\n");
        exit(-1);
    }

    struct sockaddr_in peer_addr;
    int conn;

    if ((conn = accept_timeout(listen_fd, &peer_addr, 5)) < 0) {
        exit(-1);
    }
    printf("ip = %s, port = %d\n", inet_ntoa(peer_addr.sin_addr),
            ntohs(peer_addr.sin_port));
}
