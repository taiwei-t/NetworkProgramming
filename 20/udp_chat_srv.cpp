#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "pub.h"

#define ERR_EXIT(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0);

USER_LIST client_list;

void do_login(MESSAGE msg, int sock, struct sockaddr_in* cli_addr);
void do_logout(MESSAGE msg, int sock, struct sockaddr_in* cli_addr);
void do_sendlist(int sock, struct sockaddr_in* cli_addr);

void chat_srv(int sock)
{
    struct sockaddr_in cli_addr;
    socklen_t addrlen = sizeof(cli_addr);
    MESSAGE msg;
    while (1) {
        memset(&msg, 0, sizeof(msg));
        if (recvfrom(sock, &msg, sizeof(msg), 0,
                    (struct sockaddr*)&cli_addr, &addrlen) == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        int cmd = ntohl(msg.cmd);
        switch (cmd) {
            case C2S_LOGIN:
                do_login(msg, sock, &cli_addr);
                break;
            case C2S_LOGOUT:
                do_logout(msg, sock, &cli_addr);
                break;
            case C2S_ONLINE_USER:
                do_sendlist(sock, &cli_addr);
                break;
            default:
                break;
        }
    }
}

int main()
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)))
        ERR_EXIT("bind");

    chat_srv(sock);

    return 0;
}

void do_login(MESSAGE msg, int sock, struct sockaddr_in* cli_addr)
{
    USER_INFO user;
    strcpy(user.username, msg.body);
    user.ip = cli_addr->sin_addr.s_addr;
    user.port = cli_addr->sin_port;

    USER_LIST::const_iterator it;
    for (it = client_list.cbegin(); it != client_list.cend(); it++)
        if (strcmp(it->username, msg.body) == 0)
            break;
    if (it == client_list.cend()) {
        printf("has user login: %s <-> %s:%d\n", msg.body,
                inet_ntoa(cli_addr->sin_addr), ntohs(cli_addr->sin_port));
        client_list.push_back(user);

        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));

        reply_msg.cmd = htonl(S2C_LOGIN_OK);
        sendto(sock, &reply_msg, sizeof(reply_msg), 0,
                (struct sockaddr *)cli_addr, sizeof(struct sockaddr_in));

        int count = htonl((int)client_list.size());
        sendto(sock, &count, sizeof(count), 0,
                (struct sockaddr*)cli_addr, sizeof(struct sockaddr_in));
        printf("send user list information to: %s <-> %s:%d\n", msg.body,
                inet_ntoa(cli_addr->sin_addr), ntohs(cli_addr->sin_port));
        for (it = client_list.cbegin(); it != client_list.cend(); it++)
            sendto(sock, &*it, sizeof(USER_INFO), 0,
                    (struct sockaddr*)cli_addr, sizeof(struct sockaddr_in));
        
        for (it = client_list.cbegin(); it != client_list.cend(); it++) {
            if (strcmp(it->username, msg.body) == 0)
                continue;

            struct sockaddr_in peer_addr;
            memset(&peer_addr, 0, sizeof(peer_addr));
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = it->port;
            peer_addr.sin_addr.s_addr = it->ip;

            reply_msg.cmd = htonl(S2C_SOMEONE_LOGIN);
            memcpy(reply_msg.body, &user, sizeof(user));

            if (sendto(sock, &reply_msg, sizeof(reply_msg), 0 ,
                        (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_in)) == -1)
                ERR_EXIT("sendto");
        }
    } else {
        printf("user %s has already logined\n", msg.body);

        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_ALREADY_LOGINED);
        sendto(sock, &reply_msg, sizeof(reply_msg), 0,
                (struct sockaddr*)cli_addr, sizeof(struct sockaddr_in));
    }
}

void do_logout(MESSAGE msg, int sock, struct sockaddr_in* cli_addr)
{
    printf("has a user logout: %s <-> %s:%d\n", msg.body,
            inet_ntoa(cli_addr->sin_addr), ntohs(cli_addr->sin_port));
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); it++)
        if (strcmp(it->username, msg.body) == 0)
            break;
    if (it != client_list.end())
        client_list.erase(it);

    for (it = client_list.begin(); it != client_list.end(); it++) {
        if (strcmp(it->username, msg.body) == 0)
            continue;

        struct sockaddr_in peer_addr;
        memset(&peer_addr, 0, sizeof(peer_addr));
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = it->port;
        peer_addr.sin_addr.s_addr = it->ip;

        msg.cmd = htonl(S2C_SOMEONE_LOGOUT);

        if (sendto(sock, &msg, sizeof(msg), 0 ,
                    (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_in)) == -1)
            ERR_EXIT("sendto");
    }
}

void do_sendlist(int sock, struct sockaddr_in* cli_addr)
{
    MESSAGE msg;
    msg.cmd = htonl(S2C_ONLINE_USER);
    sendto(sock, &msg, sizeof(msg), 0,
            (struct sockaddr*)cli_addr, sizeof(struct sockaddr_in));
    int count = htonl((int)client_list.size());
    sendto(sock, &count, sizeof(count), 0,
            (struct sockaddr*)cli_addr, sizeof(struct sockaddr_in));
    printf("send user list information to: %s:%d\n",
            inet_ntoa(cli_addr->sin_addr), ntohs(cli_addr->sin_port));
    USER_LIST::const_iterator it;
    for (it = client_list.cbegin(); it != client_list.cend(); it++)
        sendto(sock, &*it, sizeof(USER_INFO), 0,
                (struct sockaddr*)cli_addr, sizeof(struct sockaddr_in));
}

