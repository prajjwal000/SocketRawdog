#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
int main() {
    struct addrinfo hints, *res;
    memset( &hints,0 , sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("10.45.38.9", "3345", &hints, &res);
    char hostname[100];
    memset(&hostname, 0, 100);
    gethostname(&hostname, 100);
    printf("%s\n", hostname);

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (bind(sockfd,res->ai_addr, res->ai_addrlen) < 0){
        perror("bind failed");
        exit(1);
    }
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

