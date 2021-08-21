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
#include <sys/resource.h>

int main()
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        exit(EXIT_FAILURE);
    printf("%ld\n", rl.rlim_max);
    
    rl.rlim_cur = 10000;
    rl.rlim_max = 10000;
    if (setrlimit(RLIMIT_NOFILE, &rl) < 0)
        exit(EXIT_FAILURE);
    
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        exit(EXIT_FAILURE);
    printf("%ld\n", rl.rlim_max);
    return 0;
}

