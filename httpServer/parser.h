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
  int field_size;
  char *value;
  int value_size;
} Header;

typedef struct {
  char Method[10];
  char Uri[500];
  char Version[20];
  struct sockaddr_storage their_addr;
  Header *headers;
  int header_count;
} Request;

Lexer new_lexer(char *buf, int buf_size);
Token next_token(Lexer *lexer);
void read_char(Lexer *lexer);
int is_letter(char ch);
void read_word(Token *token, Lexer *lexer);
Request req_parse(char *buf, int buf_size, struct sockaddr_storage their_addr);
void header_parse(Request *req, Lexer *lexer);
void add_to_header_map(Header *header_map, int header_map_size,
                       int *current_headers, char *key, int key_size,
                       char *value, int value_size);
#endif
