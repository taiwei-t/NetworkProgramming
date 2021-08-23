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

char username[10];
USER_LIST client_list;

void do_someone_login(MESSAGE msg);
void do_someone_logout(MESSAGE msg);
void do_getlist(int sock);
void do_chat(MESSAGE msg);
void parse_cmd(char* cmdline, int sock, struct sockaddr_in* server_addr);
bool send_msg_to(int sock, char* peer_name, char* msg);

void print_tips()
{
    printf("\n");
    printf("+----- Commands -----+\n");
    printf("+ send username msg  +\n");
    printf("+ list               +\n");
    printf("+ exit               +\n");
    printf("+--------------------+\n");
    printf("\ninput command: ");
    fflush(stdout);
}

void chat_cli(int sock)
{
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    memset(&server_addr, 0, addrlen);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct sockaddr_in peer_addr;

    MESSAGE msg;
    while (1) {
        memset(username, 0, sizeof(username));
        printf("please input your name: ");
        scanf("%s", username);
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_LOGIN);
        strcpy(msg.body, username);

        sendto(sock, &msg, sizeof(msg), 0,
                (struct sockaddr*)&server_addr, addrlen);
        memset(&msg, 0, sizeof(msg));
        if (recvfrom(sock, &msg, sizeof(msg), 0,
                (struct sockaddr*)&server_addr, &addrlen) == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        int cmd = ntohl(msg.cmd);
        if (cmd == S2C_ALREADY_LOGINED)
            printf("user %s already logined server,\
                    please use auother username\n", username);
        else if (cmd == S2C_LOGIN_OK) {
            printf("user %s has logined server\n", username);
            break;
        }
    }

    int count;
    recvfrom(sock, &count, sizeof(count), 0, NULL, NULL);

    int n = ntohl(count);
    printf("has %d user(s) logined server\n", n);

    for (int i = 0; i < n; i++) {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(user), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;
        printf("%d %s <-> %s:%d\n", i, user.username,
                inet_ntoa(tmp), ntohs(user.port));
    }

    print_tips();

    fd_set rset;
    FD_ZERO(&rset);
    while (1) {
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sock, &rset);
        int nready = select(sock+1, &rset, NULL, NULL, NULL);
        if (nready == -1)
            ERR_EXIT("select");
        if (nready == 0)
            continue;
        if (FD_ISSET(sock, &rset)) {
            memset(&msg, 0, sizeof(msg));
            recvfrom(sock, &msg, sizeof(msg), 0,
                    (struct sockaddr*)&peer_addr, &addrlen);
            int cmd = ntohl(msg.cmd);
            switch (cmd) {
                case S2C_SOMEONE_LOGIN:
                    do_someone_login(msg);
                    break;
                case S2C_SOMEONE_LOGOUT:
                    do_someone_logout(msg);
                    break;
                case S2C_ONLINE_USER:
                    do_getlist(sock);
                    break;
                case C2C_CHAT:
                    do_chat(msg);
                    break;
                default:
                    break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            char cmdline[100] = {0};
            if (fgets(cmdline, sizeof(cmdline), stdin) == NULL)
                break;
            if (cmdline[0] == '\n')
                continue;
            cmdline[strlen(cmdline) - 1] = '\0';
            parse_cmd(cmdline, sock, &server_addr);
        }
    }
}

int main()
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    
    chat_cli(sock);

    return 0;
}


void do_someone_login(MESSAGE msg)
{
    USER_INFO *user = (USER_INFO*)msg.body;
    in_addr tmp;
    tmp.s_addr = user->ip;
    printf("%s <-> %s:%d has logined server\n", user->username,
            inet_ntoa(tmp), ntohs(user->port));
    client_list.push_back(*user);
}

void do_someone_logout(MESSAGE msg)
{
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); it++)
        if (strcmp(it->username, msg.body) == 0)
            break;
    if (it != client_list.end())
        client_list.erase(it);
    printf("user %s has logout server\n", msg.body);
}

void do_getlist(int sock)
{
    int count;
    recvfrom(sock, &count, sizeof(count), 0, NULL, NULL);

    int n = ntohl(count);
    printf("has %d user(s) logined server\n", n);

    client_list.clear();
    for (int i = 0; i < n; i++) {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(user), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;
        printf("%d %s <-> %s:%d\n", i, user.username,
                inet_ntoa(tmp), ntohs(user.port));
    }
    print_tips();
}

void do_chat(MESSAGE msg)
{
    CHAT_MSG *cm = (CHAT_MSG*)msg.body;
    printf("recv a msg [%s] from [%s]\n", cm->msg, cm->username);
}

void parse_cmd(char* cmdline, int sock, struct sockaddr_in* server_addr)
{
    char cmd[10] = {0};
    char* p;
    p = strchr(cmdline, ' '); // 返回指向第一个目标字符地址,
                              // 未找到返回NULL
    if (p != NULL)
        *p = '\0';

    strcpy(cmd, cmdline); //strcpy()拷贝到第一个'\0'截至

    if (strcmp(cmd, "exit") == 0) {
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_LOGOUT);
        strcpy(msg.body, username);

        if (sendto(sock, &msg, sizeof(msg), 0,
                    (struct sockaddr*)server_addr, sizeof(struct sockaddr_in)) == -1)
            ERR_EXIT("sendto");

        printf("user %s has logout server\n", username);
        exit(EXIT_SUCCESS);
    } else if (strcmp(cmd, "send") == 0) {
        char peer_name[16] = {0};
        char msg[MSG_LEN] = {0};

        while (*(++p) == ' ') ;  // 忽略多余的空格
        char* p2;
        p2 = strchr(p, ' ');
        if (p2 == NULL) {
            printf("bad command\n");
            print_tips();
            return;
        }
        *p2 = '\0';
        strcpy(peer_name, p);

        while (*(++p2) == ' ') ;
        strcpy(msg, p2);
        send_msg_to(sock, peer_name, msg);
        print_tips();
    } else if (strcmp(cmd, "list") == 0) {
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_ONLINE_USER);

        if (sendto(sock, &msg, sizeof(msg), 0 ,
                    (struct sockaddr*)server_addr, sizeof(struct sockaddr_in)) == -1)
            ERR_EXIT("sendto");
    } else {
        printf("bad command\n");
        print_tips();
    }
}

bool send_msg_to(int sock, char* name, char* msg)
{
    if (strcmp(name, username) == 0) {
        printf("can't send message to self\n");
        return false;
    }
    USER_LIST::const_iterator it;
    for (it = client_list.cbegin(); it != client_list.cend(); it++)
        if (strcmp(it->username, name) == 0)
            break;
    if (it == client_list.cend()) {
        printf("user %s has not logined server\n", name);
        return false;
    }

    MESSAGE m;
    memset(&m, 0, sizeof(m));
    m.cmd = htonl(C2C_CHAT);

    CHAT_MSG cm;
    strcpy(cm.username, username);
    strcpy(cm.msg, msg);
    memcpy(m.body, &cm, sizeof(cm));

    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = it->port;
    peer_addr.sin_addr.s_addr = it->ip;
         
    in_addr tmp;
    tmp.s_addr =it->ip;
    printf("send [%s] to  %s <-> %s:%d\n", msg, name,
            inet_ntoa(tmp), ntohs(it->port));

    if (sendto(sock, &m, sizeof(m), 0 ,
                (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_in)) == -1)
        ERR_EXIT("sendto");
    return true;
}

