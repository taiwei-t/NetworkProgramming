#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define ERR_EXIT(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0);
    
void echo_cli(int sock_fd)
{
    char send_buf[1024] = {0};
    char recv_buf[1024] = {0};

    while (fgets(send_buf, sizeof(send_buf), stdin) != NULL) {
        write(sock_fd, send_buf, sizeof(send_buf));
        read(sock_fd, recv_buf, sizeof(recv_buf));
        printf("%s\n", recv_buf);
        memset(send_buf, 0, sizeof(send_buf));
        memset(recv_buf, 0, sizeof(recv_buf));
    }
    close(sock_fd);
}

int main()
{
    int sock_fd;
    if ((sock_fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("sock");

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, "/tmp/test_socket");

    if (connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
        ERR_EXIT("connect");

    echo_cli(sock_fd);

    return 0;
}
