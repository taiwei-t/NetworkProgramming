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

void echo_cli(int sock)
{
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    memset(&server_addr, 0, addrlen);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server_addr, addrlen);

    char send_buf[1024];
    char recv_buf[1024];
    while (fgets(send_buf, sizeof(send_buf), stdin) != NULL) {
        sendto(sock, send_buf, strlen(send_buf), 0,
                (struct sockaddr*)&server_addr, addrlen);
        int ret = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
                (struct sockaddr*)&server_addr, &addrlen);

        if (ret == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        printf("%s\n", recv_buf);

        memset(recv_buf, 0, sizeof(recv_buf));
        memset(send_buf, 0, sizeof(send_buf));
    }
}

int main()
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    
    echo_cli(sock);

    return 0;
}
