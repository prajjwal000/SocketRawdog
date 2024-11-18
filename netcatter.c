#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
    struct addrinfo hints, *res;
    memset( &hints,0 , sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "3334", &hints, &res);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    bind(sockfd,res->ai_addr, res->ai_addrlen);
    listen(sockfd, 10);

    struct sockaddr_storage them;
    socklen_t addr_size;
    int newfd;

    addr_size = sizeof them;
    newfd = accept(sockfd, &them, &addr_size);

    void* buf[100];
    recv(newfd, buf, 100, 0);

    printf("%s", (char *) buf);
}

