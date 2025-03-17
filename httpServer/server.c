#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 6969

int main() {

  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  int yes = 1;

  // listen on socket
  //--------------------------------------------------------------------------------
  int lsock = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  struct sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);
  bind(lsock, (struct sockaddr *)&my_addr, sizeof my_addr);

  listen(lsock, 10);

  // accept connection from multiple clients ( polling )
  for (;;) {
    int asock = accept(lsock, (struct sockaddr *)&their_addr, &addr_size);
        if (!fork()){
             close(lsock);
            char buf[] = {
                "HTTP/1.1 200 OK\r\n"
                "Server: nginx/1.14.0 (Ubuntu)\r\n"
                "Date: Fri, 14 Apr 2023 12:34:56 GMT\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 1256\r\n"
                "Last-Modified: Mon, 20 Aug 2018 18:51:29 GMT\r\n"
                "Connection: close\r\n"
                "ETag: \"5b7b0e21-4e8\"\r\n"
                "Accept-Ranges: bytes\r\n\n"
                "<html><head><title>hell yeah!</title></head><body>Hell yeah!</body></html>"
            };
            send(asock, buf, sizeof buf, 0);
            close(asock);
            return 0;
        }
        close(asock);
  }
  // parse http requests
  // open file and send it through network
}
