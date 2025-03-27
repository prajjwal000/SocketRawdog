#include <sys/socket.h>
#ifndef PARSER_H
#define PARSER_H

#define TYPE_LENGTH 20
typedef struct {
  char type[TYPE_LENGTH];
  char *value;
  int value_size;
} Token;

typedef struct {
  char *buf;
  int buf_size;
  int position;
  int read_position;
  char character;
} Lexer;

typedef struct {
  char *field;
  char *value;
} Header;

typedef struct {
  char Method[10];
  char Uri[500];
  char Version[20];
  struct sockaddr_storage their_addr;
  Header *headers;
} Request;

Lexer new_lexer(char *buf, int buf_size);
Token next_token(Lexer *lexer);
void read_char(Lexer *lexer);
int is_letter(char ch);
void read_word(Token *token, Lexer *lexer);
Request req_parse(char *buf, int buf_size, struct sockaddr_storage their_addr);
#endif
