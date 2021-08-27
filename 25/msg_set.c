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

int main()
{
    int msgid;
    msgid = msgget(1234, 0400 | IPC_CREAT); // 如果没有就创建
    if (msgid == -1)
        ERR_EXIT("msgget");

    printf("msgget success\n");
    printf("msgid = %d\n", msgid);

    struct msqid_ds buf;
    msgctl(msgid, IPC_STAT, &buf);
    printf("old mode = %o\n", buf.msg_perm.mode);
    sscanf("600", "%ho", &buf.msg_perm.mode);

    msgctl(msgid, IPC_SET, &buf);

    msgctl(msgid, IPC_STAT, &buf);
    printf("new mode = %o\n", buf.msg_perm.mode);

    return 0;
}

