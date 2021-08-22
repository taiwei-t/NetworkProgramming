#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int main()
{
    for (int i = 0; ; i++) {
    int sock_fd;
    if ((sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket fail\n");
        exit(0);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
   

    if (connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        printf("connect fail\n");
        exit(0);
    }

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sock_fd, (struct sockaddr*) &local_addr, &addr_len) < 0) {
        printf("getsockname fail\n");
        exit(0);
    }
    printf("%d ip = %s, port = %d\n",i, inet_ntoa(local_addr.sin_addr),
            ntohs(local_addr.sin_port));
   
}
    return 0;
}
