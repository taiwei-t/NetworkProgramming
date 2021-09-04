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

void echo_srv(int msgid)
{
    struct msgbuf msg;
    int n;
    while (1) {
        memset(&msg, 0, sizeof(msg));
        if ((n = msgrcv(msgid, &msg, MSGMAX, 1, 0)) < 0)
            ERR_EXIT("msgrcv");
        int pid;
        pid = *((int*) msg.mtext);
        printf("%s\n", msg.mtext+4);
        msg.mtype = pid;
        msgsnd(msgid, &msg, n, 0);
    }
}

int main(int argc, char* argv[])
{
    int msgid;
    msgid = msgget(1234, IPC_CREAT | 0666);
    if (msgid == -1)
        ERR_EXIT("msgget");

    echo_srv(msgid);

    return 0;
}

