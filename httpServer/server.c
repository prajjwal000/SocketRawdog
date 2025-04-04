#include "parser.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT "6777"
#define CONN 10

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

  if (listen(listener, CONN) == -1) {
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
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count,
                   struct sockaddr_storage their_addr[CONN]) {
  pfds[i] = pfds[*fd_count - 1];
  (*fd_count)--;
  their_addr[i] = their_addr[*fd_count];
}
int sendall(int s, char *buf, int *len) {
  int total = 0;
  int bytesleft = *len;
  int n;
  while (total < *len) {
    n = send(s, buf + total, bytesleft, 0);
    if (n == 1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  *len = total;

  return n == -1 ? -1 : 0;
}
char *read_file_to_buffer(char *filename, int *size) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("file");
    exit(69);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *buffer = (char *)malloc(file_size + 1);
  if (!buffer) {
    perror("Failed to allocate memory");
    fclose(file);
    exit(10);
  }
  size_t bytes_read = fread(buffer, 1, file_size, file);
  if (bytes_read != file_size) {
    perror("Failed to read the complete file");
    free(buffer);
    fclose(file);
    exit(10);
  }

  buffer[file_size] = '\0';
  *size = file_size + 1;
  fclose(file);
  return buffer;
}

int main() {
  int listener;
  int newfd;
  struct sockaddr_storage their_addr[CONN];
  socklen_t addrlen;

  int buf_size = 1000;
  char *buf = malloc(buf_size * sizeof(char));
  char theirIP[INET6_ADDRSTRLEN] = {0};

  int fd_count = 0;
  int fd_size = 5;
  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

  listener = get_listener_socket();
  if (listener == -1) {
    fprintf(stderr, "Error getting listener socket\n");
    exit(1);
  }

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

      if (pfds[i].fd == listener) {
        addrlen = sizeof(their_addr);
        newfd = accept(listener, (struct sockaddr *)&their_addr[fd_count],
                       &addrlen);
        if (newfd == -1) {
          perror("ERROR: accept");
          continue;
        }
        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
        printf(
            "INFO: connection from %s on socket %d\n",
            inet_ntop(their_addr[fd_count - 1].ss_family,
                      get_in_addr((struct sockaddr *)&their_addr[fd_count - 1]),
                      theirIP, INET6_ADDRSTRLEN),
            newfd);
        continue; // To other fds
      }
      // Receiver
      int nbytes = recv(pfds[i].fd, buf, buf_size - 1, 0);
      int reciever_fd = pfds[i].fd;

      if (nbytes == 0) {
        printf("Connection closed on socket %d\n", reciever_fd);
        close(reciever_fd);
        del_from_pfds(pfds, i, &fd_count, their_addr);
        continue;
      }
      if (nbytes < 0) {
        perror("ERROR: recv");
        close(reciever_fd);
        del_from_pfds(pfds, i, &fd_count, their_addr);
        continue;
      }
      Request req = req_parse(buf, buf_size, their_addr[i]);
      char file_path[100] = ".";
      strncat(file_path, req.Uri, 90);
      if (!strncmp(file_path, "./", 100)) {
        strncat(file_path, "index.html", 90);
      }
      int length = 0;
      char *send_body = read_file_to_buffer(file_path, &length);
      char length_string[50];
      sprintf(length_string, "%d\r\n\r\n", length-1);
      char sendbuf[10000];
      const char *http_response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html; charset=UTF-8\r\n"
                                  "Connection: close\r\n"
                                  "Content-Length: ";
      strncat(sendbuf, http_response, 490);
      strncat(sendbuf, length_string, 50);
      strncat(sendbuf, send_body, 10000);
      int len = strlen(sendbuf);
      sendall(pfds[i].fd, sendbuf, &len);

            //INFO
      printf("Req: %s %s %s on %s:%d \n", req.Method, req.Uri, req.Version,
             inet_ntop(req.their_addr.ss_family,
                       get_in_addr((struct sockaddr *)&req.their_addr), theirIP,
                       INET6_ADDRSTRLEN),
             ntohs(((struct sockaddr_in *)&req.their_addr)->sin_port));
            printf("Headers-----------------------------\n");
            for ( int i = 0; i < req.header_count; i++){
                printf("Field: \"%s\", Value: \"%s\"\n", req.headers[i].field, req.headers[i].value);
            }

      printf("sending: %s\n", sendbuf);
    }
  }
  return 0;
}
