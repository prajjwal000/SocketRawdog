#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Request req_parse(char *buf, int buf_size, struct sockaddr_storage their_addr) {
  Request req = {0};
  req.their_addr = their_addr;
  Lexer lexer = new_lexer(buf, buf_size);
  Token token = next_token(&lexer);
  assert((!strncmp("GET", token.value, token.value_size)) ||
         (!strncmp("POST", token.value, token.value_size)));
  strncpy(req.Method, token.value, 10);
  token = next_token(&lexer);
  strncpy(req.Uri, token.value, 500);
  token = next_token(&lexer);
  strncpy(req.Version, token.value, 20);
  header_parse(&req, &lexer);

  return req;
}

void header_parse(Request *req, Lexer *lexer) {
  int header_map_size = 10;
  Header *header_map = (Header *)malloc(header_map_size * sizeof(Header));
  int current_headers = 0;

  Token tok = {0};
  tok = next_token(lexer);
  assert(!strncmp("REGISTERED_NURSE", tok.type, 20));
  for (;;) {
    tok = next_token(lexer);
    if (!strncmp("REGISTERED_NURSE", tok.type, 20)) {
      break;
    }
    char *temp_key = tok.value;
    int temp_key_size = tok.value_size;
    // printf("Token value: %s ", temp_key);
    assert(!strncmp("LEGAL", tok.type, 6));
    // int key_length = strlen(temp_key);
    // assert(temp_key[key_length - 1] == ':');
    tok = next_token(lexer);
    char *temp_value = tok.value;
    int temp_value_size = tok.value_size;
    // printf("Token value: %s ", temp_value);
    assert(!strncmp("LEGAL", tok.type, 6));
    tok = next_token(lexer);
    // printf("Token value: %s ", tok.value);
    assert(!strncmp("REGISTERED_NURSE", tok.type, 20));
    add_to_header_map(header_map, header_map_size, &current_headers, temp_key,
                      temp_key_size, temp_value, temp_value_size);
    req->header_count = current_headers;
    req->headers = header_map;
  }
}

void add_to_header_map(Header *header_map, int header_map_size,
                       int *current_headers, char *key, int key_size,
                       char *value, int value_size) {
  if (header_map_size == (*current_headers)) {
    header_map_size *= 2;
    header_map =
        (Header *)realloc(header_map, header_map_size * sizeof(Header));
  }
  Header head = {0};
  head.field = key;
  head.field_size = key_size;
  head.value = value;
  head.value_size = value_size;
  header_map[(*current_headers)] = head;
  (*current_headers)++;
}

Lexer new_lexer(char *buf, int buf_size) {
  Lexer lexer = {0};
  lexer.buf = buf;
  lexer.buf_size = buf_size;
  read_char(&lexer);
  return lexer;
}

void read_char(Lexer *lexer) {
  if ((*lexer).read_position == (*lexer).buf_size) {
    (*lexer).character = '\0';
  } else {
    (*lexer).character = (*lexer).buf[(*lexer).read_position];
  }

  (*lexer).position = (*lexer).read_position;
  ((*lexer).read_position)++;
}

Token next_token(Lexer *lexer) {
  Token token = {0};
  int jump = 0;

  while ((*lexer).character == ' ' || (*lexer).character == '\t') {
    read_char(lexer);
  }
  switch ((*lexer).character) {
  case '\r':
    read_char(lexer);
    if ((*lexer).character == '\n') {
      strncpy(token.type, "REGISTERED_NURSE\0", TYPE_LENGTH);
      token.value_size = 5;
      token.value = (char *)malloc(token.value_size * sizeof(char));
      strncpy(token.value, "\\r\\n", token.value_size);
    } else {
      strncpy(token.type, "ILLEGAL", TYPE_LENGTH);
      token.value_size = 8;
      token.value = (char *)malloc(token.value_size * sizeof(char));
      strncpy(token.value, "ILLEGAL", token.value_size);
    }
    break;
  case '\0':
    strncpy(token.type, "EOF", TYPE_LENGTH);
    token.value_size = 5;
    token.value = (char *)malloc(token.value_size * sizeof(char));
    strncpy(token.value, "EOF", token.value_size);
    break;
    // case ':':
    //   strncpy(token.type, ":", TYPE_LENGTH);
    //   token.value_size = 2;
    //   token.value = (char *)malloc(token.value_size * sizeof(char));
    //   strncpy(token.value, ":", token.value_size);
    //   break;

  default:
    if (is_letter((*lexer).character)) {
      strncpy(token.type, "LEGAL", TYPE_LENGTH);
      read_word(&token, lexer);
      jump = 1;
    } else {
      strncpy(token.type, "ILLEGAL", TYPE_LENGTH);
      token.value_size = 8;
      token.value = (char *)malloc(token.value_size * sizeof(char));
      strncpy(token.value, "ILLEGAL", token.value_size);
    }
  }
  if (jump == 0) {
    read_char(lexer);
  }
  return token;
}

int is_letter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') ||
         ('0' <= ch && ch <= '9') || ch == '_' || ch == '/' || ch == '.' ||
         ch == ':' || ch == '*' || ch == '-';
}

void read_word(Token *token, Lexer *lexer) {
  (*token).value_size = 10;
  (*token).value = (char *)malloc((*token).value_size * sizeof(char));
  int tok_value_pos = 0;
  while (is_letter((*lexer).character)) {
    if (tok_value_pos >= (*token).value_size - 2) {
      (*token).value_size *= 2;
      (*token).value =
          (char *)realloc((*token).value, (*token).value_size * sizeof(char));
    }
    (*token).value[tok_value_pos] = (*lexer).character;
    tok_value_pos++;
    read_char(lexer);
    //       if (tok_value_pos < 20 ) {
    // printf("Before: Lexer pos: %d Lexer readpos: %d Lexer char: %c\n",
    //        (*lexer).position, (*lexer).read_position, (*lexer).character);
    //       } else {
    //           exit(1);
    //       }
  }
  (*token).value[tok_value_pos] = '\0';
}
