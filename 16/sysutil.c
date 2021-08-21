#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "sysutil.h"

/**
 *
 */
int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds > 0) {
        fd_set read_fdset;
        struct timeval timeout;

        FD_ZERO(&read_fdset);
        FD_SET(fd, &read_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do {
            ret = select(fd + 1, &read_fdset, NULL, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);

        if (ret == 0) {
            ret = -1;
            errno = ETIMEDOUT;
        } else if (ret == 1)
            ret = 0;
    }

    return ret;
}

int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds > 0) {
        fd_set write_fdset;
        struct timeval timeout;

        FD_ZERO(&write_fdset);
        FD_SET(fd, &write_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do {
            ret = select(fd + 1, NULL, &write_fdset, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);

        if (ret == 0) {
            ret = -1;
            errno = ETIMEDOUT;
        } else if (ret == 1)
            ret = 0;
    }

    return ret;
}

int accept_timeout(int fd, struct sockaddr_in* addr, unsigned int wait_seconds)
{
    int ret = 0;

    if (wait_seconds > 0) {
        fd_set accept_fdset;
        struct timeval timeout;

        FD_ZERO(&accept_fdset);
        FD_SET(fd, &accept_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do {
            ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);

        if (ret == 0) {
            errno = ETIMEDOUT;
            return -1;
        } else if (ret == -1)
            return -1;
    }

    if (addr != NULL) {
        socklen_t addrlen = sizeof(struct sockaddr_in);
        ret = accept(fd, (struct sockaddr*)addr, &addrlen);
    } else
        ret = accept(fd, NULL, NULL);

    if (ret == -1) {
        printf("accept fail\n");
        exit(EXIT_FAILURE);
    }

    return ret;
}

void activate_nonblock(int fd)
{
    int ret;
    int flag = fcntl(fd, F_GETFL);
    if (flag == -1) {
        printf("fcntl fail\n");
        exit(EXIT_FAILURE);
    }
    ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    if (ret == -1) {
        printf("fcntl fail\n");
        exit(EXIT_FAILURE);
    }
}

void deactivate_nonblock(int fd)
{
    int ret;
    int flag = fcntl(fd, F_GETFL);
    if (flag == -1) {
        printf("fcntl fail\n");
        exit(EXIT_FAILURE);
    }
    ret = fcntl(fd, F_SETFL, flag & ~O_NONBLOCK);
    if (ret == -1) {
        printf("fcntl fail\n");
        exit(EXIT_FAILURE);
    }
}

int connect_timeout(int fd, struct sockaddr_in* addr, unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (wait_seconds > 0)
        activate_nonblock(fd);

    ret = connect(fd, (struct sockaddr*) addr, addrlen);
    
    if (ret < 0 && errno == EINPROGRESS) {
        fd_set connect_fdset;
        struct timeval timeout;

        FD_ZERO(&connect_fdset);
        FD_SET(fd, &connect_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do {
            ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);

        if (ret == 0) {
            ret = -1;
            errno = ETIMEDOUT;
        } else if (ret == 1) {
            int err;
            socklen_t socklen = sizeof(err);
            int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
            if (sockoptret == -1)
                return -1;
            if (err == 0)
                ret = 0;
            else {
                errno = err;
                ret = -1;
            }
        }
    }

    if (wait_seconds > 0)
        deactivate_nonblock(fd);

    return ret;
}

