#include "parser.h"
#include <assert.h>
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

  return req;
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
    }
    strncpy(token.type, "ILLEGAL", TYPE_LENGTH);
    token.value_size = 8;
    token.value = (char *)malloc(token.value_size * sizeof(char));
    strncpy(token.value, "ILLEGAL", token.value_size);
    break;
  default:
    if (is_letter((*lexer).character)) {
      strncpy(token.type, "LEGAL", TYPE_LENGTH);
      read_word(&token, lexer);
    } else {
      strncpy(token.type, "ILLEGAL", TYPE_LENGTH);
      token.value_size = 8;
      token.value = (char *)malloc(token.value_size * sizeof(char));
      strncpy(token.value, "ILLEGAL", token.value_size);
    }
  }
  read_char(lexer);
  return token;
}

int is_letter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') ||
         ('0' <= ch && ch <= '9') || ch == '_' || ch == '/' || ch == '.';
}

void read_word(Token *token, Lexer *lexer) {
  (*token).value_size = 10;
  (*token).value = (char *)malloc((*token).value_size * sizeof(char));
  int tok_value_pos = 0;
  while ((*lexer).character != ' ' && (*lexer).character != '\t' &&
         (*lexer).character != '\n' && (*lexer).character != '\r') {
    if (tok_value_pos >= (*token).value_size - 2) {
      (*token).value_size *= 2;
      (*token).value = (char *)malloc((*token).value_size * sizeof(char));
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
