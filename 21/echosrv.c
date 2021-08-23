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


void echo_srv(int con_fd)
{
    char recv_buf[1024];
    while (1) {
        memset(recv_buf, 0, sizeof(recv_buf));
        int n = read(con_fd, recv_buf, sizeof(recv_buf));
        if (n == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("read");
        } else if (n == 0)
            break;

        printf("%s", recv_buf);
        write(con_fd, recv_buf, n);
    }
    close(con_fd);
}

int main()
{
    int listen_fd;
    if ((listen_fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, "/tmp/test_socket", sizeof(server_addr.sun_path));

    unlink("/tmp/test_socket");
    if ((bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0)
        ERR_EXIT("bind");

    if (listen(listen_fd, SOMAXCONN))
        ERR_EXIT("listen");

    int con_fd;
    while (1) {
        con_fd = accept(listen_fd, NULL, NULL);
        if (con_fd == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("accept");
        }
        pid_t pid = fork();

        if (pid == -1)
            ERR_EXIT("fork");

        if (pid == 0) {
            close(listen_fd);
            echo_srv(con_fd);
            exit(EXIT_SUCCESS);
        }
        close(con_fd);
    }

    return 0;
}
