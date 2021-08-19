#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int main() {
    char host[100] = {0};
    if (gethostname(host, sizeof(host)) < 0) {
        printf("gethostname fail\n");
        exit(EXIT_FAILURE);
    }

    struct hostent* hp;
    if((hp = gethostbyname(host)) == NULL) {
        printf("gethostbyname fail\n");
        exit(EXIT_FAILURE);
    }
    
    int i = 0;
    printf("ip list:\n");
    while (hp->h_addr_list[i] != NULL) {
        printf("\t%s\n", inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]));
        i++;
    }
    
    printf("local ip: %s\n", inet_ntoa(*(struct in_addr*)hp->h_addr));

    return 0;
}
