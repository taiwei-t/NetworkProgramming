#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
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

    char send_buff[1024] = {0};
    char recv_buff[1024] = {0};
    while (fgets(send_buff, sizeof(send_buff), stdin) != NULL) {
        write(sock_fd, send_buff, sizeof(send_buff));
        read(sock_fd, recv_buff, sizeof(recv_buff));
        printf("%s\n", recv_buff);
        memset(send_buff, 0, sizeof(send_buff));
        memset(recv_buff, 0, sizeof(recv_buff));
    }

    close(sock_fd);
    return 0;
}
