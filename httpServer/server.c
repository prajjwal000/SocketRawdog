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
  int usock = accept(lsock, (struct sockaddr *)&their_addr, &addr_size);
  send(usock, "Hello sailor", 13, 0);
  close(usock);
  close(lsock);
  // parse http requests
  // open file and send it through network
}
