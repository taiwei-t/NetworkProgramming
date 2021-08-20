#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

ssize_t readn(int fd, void* buf, size_t count)
{
    size_t nleft = count;
    ssize_t nread;
    char* ptr = buf;

    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        } else if (nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }

    return count - nleft;
}

ssize_t writen(int fd, const void* buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwritten;
    const char* ptr = buf;

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        } else if (nwritten == 0)
            break;
        nleft -= nwritten;
        ptr += nwritten;
    }

    return count;
}

ssize_t recv_peek(int sockfd, void* buf, size_t len)
{
    while (1) {
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if (ret ==-1 && errno == EINTR)
            continue;
        else
            return ret;
    }
}

ssize_t readline(int sockfd, void* buf, size_t maxlen)
{
    int ret;
    int nread;
    char* ptr = buf;
    int nleft = maxlen;

    while (1) {
        ret = recv_peek(sockfd, ptr, nleft);
        if (ret <= 0)
            return ret;
        nread = ret;
        for (int i = 0; i < nread; i++) {
            if (ptr[i] == '\n') {
                ret = readn(sockfd, ptr, i+1);
                if (ret != i+1)
                    exit(EXIT_FAILURE);
                return ret;
            }
        }
        if (nread > nleft)
            exit(EXIT_FAILURE);
        nleft -= nread;
        ret = readn(sockfd, ptr, nread);
        if (ret != nread)
            exit(EXIT_FAILURE);
        ptr += nread;
    }
    return -1;
}

void echo_cli(int sock_fd)
{
    char send_buff[1024] = {0};
    char recv_buff[1024] = {0};
    while (fgets(send_buff, sizeof(send_buff), stdin) != NULL) {
        //writen(sock_fd, send_buff, strlen(send_buff));
        
        // 试验SIGPIPE信号
        writen(sock_fd, send_buff, 1);
        writen(sock_fd, send_buff + 1, strlen(send_buff) - 1);

        readline(sock_fd, recv_buff, sizeof(recv_buff));
        printf("%s\n", recv_buff);
        memset(send_buff, 0, sizeof(send_buff));
        memset(recv_buff, 0, sizeof(recv_buff));
    }
}

void handle_pipe(int signo)
{
    printf("signal pipe\n");
}

int main()
{
    signal(SIGPIPE, handle_pipe); // 处理SIGPIPE信号

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
    printf("ip = %s, port = %d\n", inet_ntoa(local_addr.sin_addr),
            ntohs(local_addr.sin_port));
   
    echo_cli(sock_fd);

    close(sock_fd);
    return 0;
}
