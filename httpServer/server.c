#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT "6969"

typedef struct {
  char *field;
  char *value;
} Header;

typedef struct {
  char Method[10]; // Noone with good intentions send method longer than 10
  char Uri[500];   // Only support 499 characters URI, else send internal server
                   // error
  Header *headers;
} Request;

void *get_in_addr(struct sockaddr *sa) {

  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int get_listener_socket(void) {

  int listener, err;
  int yes = 1;
  socklen_t myaddr_size;
  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((err = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "httpserver: %s\n", gai_strerror(err));
    exit(1);
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      perror("ERROR: socket:");
      continue;
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
      perror("ERROR: setsockopt: ");
      continue;
    }

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      perror("ERROR: bind: ");
      close(listener);
      continue;
    }
    break;
  }

  freeaddrinfo(ai);

  if (p == NULL) {
    printf("ERROR: No addr selected for listener\n");
    return -1;
  }

  if (listen(listener, 10) == -1) {
    perror("ERROR: listen");
    return -1;
  }

  return listener;
}

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count,
                 int *fd_size) {

  if (*fd_count == *fd_size) {
    *fd_size *= 2;
    *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
  }

  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN;
  (*fd_count)++;
}
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
  pfds[i] = pfds[*fd_count - 1];
  (*fd_count)--;
}

int sendall(int s, char* buf, int* len){
    int total = 0;
    int bytesleft = n;
    int n;
    while (total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n==1) {break;}
        total += n;
        bytesleft -= n;
    }

    *len = total;

    return n==-1?-1:0;
}

int main() {

  int listener;
  int newfd;
  struct sockaddr_storage their_addr;
  socklen_t addrlen;

  char buf[1000];
  char theirIP[INET6_ADDRSTRLEN];

  int fd_count = 0;
  int fd_size = 5;
  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

  listener = get_listener_socket();
  if (listener == -1) {
    fprintf(stderr, "Error getting listener socket\n");
    exit(1);
  }

  // Add listener to pfds
  pfds[0].fd = listener;
  pfds[0].events = POLL_IN;
  fd_count = 1;

  for (;;) {
    int poll_count = poll(pfds, fd_count, -1);
    if (poll_count == -1) {
      perror("ERROR: poll");
      exit(1);
    }

    for (int i = 0; i < fd_count; i++) {
      if (!(pfds[i].revents & POLLIN)) {
        continue;
      }

      if (pfds[i].fd == listener) { // LISTENER SOCKET
        addrlen = sizeof(their_addr);
        newfd = accept(listener, (struct sockaddr *)&their_addr, &addrlen);
        if (newfd == -1) {
          perror("ERROR: accept");
          continue;
        }
        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
        printf("INFO: connection from %s on socket %d\n",
               inet_ntop(their_addr.ss_family,
                         get_in_addr((struct sockaddr *)&their_addr), theirIP,
                         INET6_ADDRSTRLEN),
               newfd);
        continue; // To other fds
      }
      // Receiver
      int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);
      int reciever_fd = pfds[i].fd;

      if (nbytes == 0) {
        printf("Connection closed on socket %d\n", reciever_fd);
        close(reciever_fd);
        del_from_pfds(pfds, i, &fd_count);
        continue;
      }
      if (nbytes < 0) {
        perror("ERROR: recv");
        close(reciever_fd);
        del_from_pfds(pfds, i, &fd_count);
        continue;
      }
      buf[nbytes - 1] = '\0';
      printf("Got %s from socket: %d\n", buf, reciever_fd);
    }
  }
  return 0;
}
