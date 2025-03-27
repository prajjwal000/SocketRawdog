#ifndef TOKEN_H
#define TOKEN_H

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

Lexer new_lexer(char* buf, int buf_size);
Token next_token(Lexer *lexer);
void read_char(Lexer *lexer);
int is_letter(char ch);
void read_word(Token *token, Lexer *lexer);
#endif
