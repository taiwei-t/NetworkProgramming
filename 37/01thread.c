#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void *thread_routine(void* arg)
{
    for (int i =0; i != 20; i++) {
        printf("B");
        fflush(stdout);
        usleep(20);
//        if (i == 3)
//            pthread_exit("thread exit");
    }

    return "end"; 
}

int main()
{
    pthread_t tid;
    int ret;
    if ((ret = pthread_create(&tid, NULL, thread_routine, NULL)) != 0) {
        fprintf(stderr, "pthread_create: %s", strerror(ret));
        exit(EXIT_FAILURE);
    }

    for (int i =0; i != 20; i++) {
        printf("A");
        fflush(stdout);
        usleep(20);
    }

    void* value;
    if ((pthread_join(tid, &value)) != 0) {
        fprintf(stderr, "pthread_join: %s", strerror(ret));
        exit(EXIT_FAILURE);
    }
    printf("\n");
    printf("return message: %s\n", (char*) value);
    return 0;
}
