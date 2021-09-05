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
#include <pthread.h>

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

void echo_srv(int con_fd)
{
    struct sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);
    if (getpeername(con_fd, (struct sockaddr*) &peer_addr, &addr_len) < 0) {
        printf("getsockname fail\n");
        exit(0);
    }
    printf("%s:%d connection\n", inet_ntoa(peer_addr.sin_addr),
            ntohs(peer_addr.sin_port));

    char recv_buff[1024];
    while (1) {
        memset(recv_buff, 0, sizeof(recv_buff));
        int nread = readline(con_fd, recv_buff, sizeof(recv_buff));
        if (nread == 0)
            break;
        else if (nread < 0)
            exit(-1);
        printf("(%s:%d): %s\n", inet_ntoa(peer_addr.sin_addr),
                ntohs(peer_addr.sin_port), recv_buff);
        writen(con_fd, recv_buff, nread);
    }
    printf("%s:%d disconnection\n", inet_ntoa(peer_addr.sin_addr),
            ntohs(peer_addr.sin_port));
}

void handle_sigchld(int signo)
{
    pid_t pid;
    // pid = wait(NULL);
    // printf("pid = %d exit\n", pid);

    // 等待所有子进程
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
        printf("pid = %d exit\n", pid);

    return;
}

void *thread_routine(void* arg)
{
    pthread_detach(pthread_self());
    int con = *((int*)arg);
    free(arg);
    echo_srv(con);
    return NULL;
}

int main()
{
    // 处理SIGCHLD信号，防止僵死进程
    //signal(SIGCHLD, SIG_IGN);
    signal(SIGCHLD, handle_sigchld);
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

    while (1) {
        if ((con_fd = accept(listen_fd, (struct sockaddr*) &peer_addr, &len)) < 0) {
            printf("accept fail\n");
            exit(-1);
        }

        pthread_t tid;
        int ret;
        int *p = malloc(sizeof(int));
        *p = con_fd;
        if ((ret =pthread_create(&tid, NULL, thread_routine, (void*)p)) != 0) {
            fprintf(stderr, "pthread_create: %s", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }
    close(listen_fd);

    return 0;
}
