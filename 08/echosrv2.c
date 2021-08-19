#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void echo(int con_fd)
{
    char recv_buff[1024];
    while (1) {
        memset(recv_buff, 0, sizeof(recv_buff));
        int nread = read(con_fd, recv_buff, sizeof(recv_buff));
        if (nread == 0)
            break;
        else if (nread < 0)
            exit(-1);
        printf("%s\n", recv_buff);
        write(con_fd, recv_buff, nread);
    }
}

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
    socklen_t len = sizeof(peer_addr);
    int con_fd;

    pid_t pid;
    while (1) {
        if ((con_fd = accept(listen_fd, (struct sockaddr*) &peer_addr, &len)) < 0) {
            printf("accept fail\n");
            exit(-1);
        }

        pid = fork();
        if (pid == -1) {
            printf("fork fail\n");
            exit(-1);
        }
        if (pid == 0) {
            close(listen_fd);
            printf("%s:%d connection\n", inet_ntoa(peer_addr.sin_addr),
                    ntohs(peer_addr.sin_port));

            echo(con_fd);

            printf("%s:%d disconnection\n", inet_ntoa(peer_addr.sin_addr),
                    ntohs(peer_addr.sin_port));
            close(con_fd);
            exit(0);
        }

        close(con_fd);
    }
    close(listen_fd);

    return 0;
}
