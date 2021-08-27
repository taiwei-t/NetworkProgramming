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

struct msgbuf {
    long mtype;
    char mtext[1];
};

#define MSGMAX 65535

int main(int argc, char* argv[])
{
    int flag = 0;
    int type = 0;
    int opt;
    while (1) {
        opt = getopt(argc, argv, "nt:");
        if (opt == '?')
            exit(EXIT_FAILURE);

        if (opt == -1)
            break;

        switch (opt) {
            case 'n':
                flag |= IPC_NOWAIT;
                break;
            case 't':
                type = atoi(optarg);
                break;
        }
    }

    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
        ERR_EXIT("msgget");

    struct msgbuf* ptr;
    ptr = (struct msgbuf*)malloc(sizeof(long) + MSGMAX);
    ptr->mtype = type;
    int nrcv = 0;
    if ((nrcv = msgrcv(msgid, ptr, MSGMAX, type, flag)) == -1)
        ERR_EXIT("msgrcv");

    printf("recv %d bytes, type = %ld\n", nrcv, ptr->mtype);

    return 0;
}

