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
    
int main()
{
    int socks[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, socks) < 0)
        ERR_EXIT("socketpair");

    pid_t pid;
    pid = fork();
    if (pid == -1)
        ERR_EXIT("fork");
    if (pid == 0) {
        close(socks[1]);
        int val = 0;
        while (1) {
            read(socks[0], &val, sizeof(val));
            ++val;
            write(socks[0], &val, sizeof(val));
        }
    } else if (pid > 0) {
        close(socks[0]);
        int val = 0;
        while (1) {
            ++val;
            printf("send data: %d\n", val);
            write(socks[1], &val, sizeof(val));
            read(socks[1], &val, sizeof(val));
            printf("data received: %d\n", val);
            sleep(1);
        }
    }

    return 0;
}
