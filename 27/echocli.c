#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define ERR_EXIT(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0);

#define MSGMAX 65535
struct msgbuf {
    long mtype;
    char mtext[MSGMAX];
};

void echo_cli(int msgid)
{
    int pid = getpid();
    struct msgbuf msg;
    memset(&msg, 0, sizeof(msg));
    *((int*)msg.mtext) = pid;
    msg.mtype = 1;
    while (fgets(msg.mtext+4, MSGMAX, stdin) != NULL) {
        if (msgsnd(msgid, &msg, 4+strlen(msg.mtext+4), 0) < 0)
            ERR_EXIT("msgsnd");
        if (msgrcv(msgid, &msg, MSGMAX, pid, 0) < 0)
            ERR_EXIT("msgrcv");
        printf("%s\n", msg.mtext+4);
        memset(msg.mtext, 0, MSGMAX-4);
    }
}

int main(int argc, char* argv[])
{
    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
        ERR_EXIT("msgget");

    echo_cli(msgid);

    return 0;
}

