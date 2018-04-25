#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 64
#define MAX_TOKENS 2048
#define MAX_INPUT_SIZE 4096

typedef struct {
  char str[MAX_TOKEN_LEN];
} Token;

char file_buf[MAX_INPUT_SIZE];
int file_buf_size;

Token tokens[MAX_TOKENS];
int tokens_count;

void AddToken(const char *begin, const char *end) {
  if (end <= begin || (end - begin) >= MAX_TOKEN_LEN) {
    puts("Too long token");
    exit(EXIT_FAILURE);
  }
  if (tokens_count >= MAX_TOKENS) {
    puts("Too many token");
    exit(EXIT_FAILURE);
  }
  strncpy(tokens[tokens_count].str, begin, end - begin);
  printf("[%s]", tokens[tokens_count].str);
  tokens_count++;
}

void ReadFile(FILE *fp) {
  int file_buf_size = fread(file_buf, 1, MAX_INPUT_SIZE, fp);
  if (file_buf_size >= MAX_INPUT_SIZE) {
    puts("Too large input");
    exit(EXIT_FAILURE);
  }
  file_buf[file_buf_size] = 0;
  printf("Input size: %d\n", file_buf_size);
}

#define IS_IDENT_NODIGIT(c) \
  ((c) == '_' || ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z'))
#define IS_IDENT_DIGIT(c) (('0' <= (c) && (c) <= '9'))
void Tokenize() {
  const char *p = file_buf;
  const char *begin = NULL;
  const char *single_char_punctuators = "[](){}~?:;,\\\n";
  do {
    if (*p == ' ') {
      p++;
    } else if(*p == '\n') {
      p++;
      putchar('\n');
    } else if (IS_IDENT_NODIGIT(*p)) {
      begin = p++;
      while (IS_IDENT_NODIGIT(*p) || IS_IDENT_DIGIT(*p)) {
        p++;
      }
      AddToken(begin, p);
    } else if (IS_IDENT_DIGIT(*p)) {
      begin = p++;
      while (IS_IDENT_DIGIT(*p)) {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '"' || *p == '\'') {
      begin = p++;
      while (*p && *p != *begin) {
        if(*p == '\\') p++;
        p++;
      }
      if(*(p++) != *begin){
        printf("Expected %c but got char 0x%02X", *begin, *p);
        exit(EXIT_FAILURE);
      }
      AddToken(begin, p);
    } else if (strchr(single_char_punctuators, *p)) {
      // single character punctuator
      begin = p++;
      AddToken(begin, p);
    } else if (*p == '#') {
      begin = p++;
      if (*p == '#') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '|' || *p == '&' || *p == '+' || *p == '/') {
      // | || |=
      // & && &=
      // + ++ +=
      // / // /=
      begin = p++;
      if (*p == *begin || *p == '=') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '-') {
      // - -- -= ->
      begin = p++;
      if (*p == *begin || *p == '-' || *p == '>') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '=' || *p == '!' || *p == '*') {
      // = ==
      // ! !=
      // * *=
      begin = p++;
      if (*p == '=') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '<' || *p == '>') {
      // < << <= <<=
      // > >> >= >>=
      begin = p++;
      if (*p == *begin) {
        p++;
        if(*p == '='){
          p++;
        }
      } else if(*p == '='){
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '.') {
      // .
      // ...
      begin = p++;
      if (p[0] == '.' && p[1] == '.') {
        p += 2;
      }
      AddToken(begin, p);
    } else {
      printf("Unexpected char '%c'\n", *p);
      break;
    }
  } while (*p);
}

int main(int argc, char *argv[]) {
  if (argc < 2) return 1;

  FILE *fp = fopen(argv[1], "rb");
  if (!fp) return 1;
  ReadFile(fp);
  fclose(fp);

  Tokenize();


  return 0;
}
