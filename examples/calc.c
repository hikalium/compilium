#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 128

void assert(int v) {
  if (v) return;
  printf("Assertion failed!");
}

#define INITIAL_INPUT_SIZE 8192
static const char *ReadLine(void) {
  int buf_size = INITIAL_INPUT_SIZE;
  char *input = NULL;
  int input_size = 0;
  int c;
  input = malloc(buf_size);
  while ((c = getchar()) != EOF && c != '\n') {
    input[input_size++] = c;
    if (input_size == buf_size) {
      buf_size <<= 1;
      assert((input = realloc(input, buf_size)));
    }
  }
  assert(input_size < buf_size);
  input[input_size] = 0;
  return input;
}

struct Token {
  const char *begin;
  int length;
  const char *src_str;
  int line;
  struct Node *next_token;
};

struct Token *AllocToken(const char *src_str, int line, const char *begin,
                         int length) {
  struct Token *t;
  t = malloc(sizeof(*t));
  t->begin = begin;
  t->length = length;
  t->src_str = src_str;
  t->line = line;
  t->next_token = NULL;
  return t;
}

static struct Token *CreateNextToken(const char *p, const char *src,
                                     int *line) {
  assert(line);
  if (!*p) return NULL;
  if (*p == ' ') {
    return AllocToken(src, *line, p, 1);
  }
  if (*p == '\n') {
    (*line)++;
    return AllocToken(src, *line, p, 1);
  }
  if (p[0] == '\\' && p[1] == '\n') {
    (*line)++;
    return AllocToken(src, *line, p, 2);
  }
  if ('1' <= *p && *p <= '9') {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '9') {
      length++;
    }
    return AllocToken(src, *line, p, length);
  } else if ('0' == *p) {
    int length = 0;
    if (p[1] == 'x') {
      // Hexadecimal
      length += 2;
      while (('0' <= p[length] && p[length] <= '9') ||
             ('A' <= p[length] && p[length] <= 'F') ||
             ('a' <= p[length] && p[length] <= 'f')) {
        length++;
      }
    } else {
      // Octal
      while ('0' <= p[length] && p[length] <= '7') {
        length++;
      }
    }
    return AllocToken(src, *line, p, length);
  } else if (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') ||
             *p == '_') {
    int length = 0;
    while (('A' <= p[length] && p[length] <= 'Z') ||
           ('a' <= p[length] && p[length] <= 'z') || p[length] == '_' ||
           ('0' <= p[length] && p[length] <= '9')) {
      length++;
    }
    struct Node *t = AllocToken(src, *line, p, length);
    return t;
  }
  printf("Unexpected char %c\n", *p);
  exit(1);
}

struct Token *Tokenize(const char *input) {
  // returns head of tokens.
  struct Token *token_head = NULL;
  struct Token **last_next_token = &token_head;
  const char *p = input;
  struct Token *t;
  int line = 1;
  while ((t = CreateNextToken(p, input, &line))) {
    *last_next_token = t;
    last_next_token = &t->next_token;
    p = t->begin + t->length;
  }
  return token_head;
}

int main() {
  fputs("Hello stderr!\n", stderr);
  const char *input = ReadLine();
  struct Token *tokens = Tokenize(input);
  for (struct Token *t = tokens; t; t = t->next_token) {
    putchar('<');
    printf("%.*s", t->length, t->begin);
    putchar('>');
  }
  putchar('\n');
  return 0;
}
