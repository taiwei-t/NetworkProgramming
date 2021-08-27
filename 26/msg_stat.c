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
    printf("mode = %o\n", buf.msg_perm.mode);
    printf("bytes = %ld\n", buf.__msg_cbytes);
    printf("number = %ld\n", buf.msg_qnum);
    printf("msgmnb = %ld\n", buf.msg_qbytes);

    return 0;
}

