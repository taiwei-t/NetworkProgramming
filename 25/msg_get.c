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
    //msgid = msgget(1234, 0666 | IPC_CREAT | IPC_EXCL); // 已存在则报错
    //msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL); // 指定IPC_PRIVATE
    //msgid = msgget(1234, 0); // 打开一个消息队列
    if (msgid == -1)
        ERR_EXIT("msgget");

    printf("msgget success\n");
    printf("msgid = %d\n", msgid);

    return 0;
}

