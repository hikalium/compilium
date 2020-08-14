// https://www.json.org/json-en.html
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_SIZE 128
#define PATH_LEN_STACK_SIZE 16

char path[PATH_SIZE];
int path_used;
int path_len_stack[PATH_LEN_STACK_SIZE];
int path_len_stack_used;

void PushPath(const char *s) {
  if (path_len_stack_used >= PATH_LEN_STACK_SIZE) {
    printf("No more path len stack\n");
    exit(1);
  }
  int slen = strlen(s);
  if (path_used + slen >= PATH_SIZE) {
    printf("Too long path\n");
    exit(1);
  }
  path_len_stack[path_len_stack_used++] = path_used;
  memcpy(&path[path_used], s, slen);
  path_used += slen;
}

void PopPath() {
  if (path_len_stack_used <= 0) {
    printf("Cannot pop path\n");
    exit(1);
  }
  path_used = path_len_stack[--path_len_stack_used];
}

void PrintPath() { printf("%.*s", path_used, path); }

int TryValueBool(int c) {
  if (c == 't') {
    if ((c = getchar() != 'r') || (c = getchar()) != 'u' ||
        (c = getchar()) != 'e') {
      printf("Unexpected char %c\n", c);
      exit(1);
    }
    PrintPath();
    printf(" = true\n");
    return 1;
  }
  if (c == 'f') {
    if ((c = getchar() != 'a') || (c = getchar()) != 'l' ||
        (c = getchar()) != 's' || (c = getchar()) != 'e') {
      printf("Unexpected char %c\n", c);
      exit(1);
    }
    PrintPath();
    printf(" = false\n");
    return 1;
  }
  return 0;
}
int TryValueNull(int c) {
  if (c == 'n') {
    if ((c = getchar() != 'u') || (c = getchar()) != 'l' ||
        (c = getchar()) != 'l') {
      printf("Unexpected char %c\n", c);
      exit(1);
    }
    PrintPath();
    printf(" = null\n");
    return 1;
  }
  return 0;
}

int ReadWhiteSpaces(int c) {
  // returns first non-whitespace character
  while (c == ' ') {
    c = getchar();
  }
  return c;
}

int ReadElement();
int TryValueArray(int c) {
  if (c != '[') return 0;
  c = ReadWhiteSpaces(getchar());
  int index = 0;
  char buf[16];
  for (;;) {
    if (c == ']') break;
    if (c == ',') {
      c = ReadWhiteSpaces(getchar());
      index++;
      continue;
    }
    snprintf(buf, sizeof(buf), "[%d]", index);
    PushPath(buf);
    c = ReadElement(c);
    PopPath();
  }
  return 1;
}

void ParseValue(int c) {
  if (TryValueBool(c) || TryValueNull(c) || TryValueArray(c)) {
    return;
  }
  printf("Unexpected char %c\n", c);
  exit(1);
}

int ReadElement(int c) {
  // returns first non-element character
  ParseValue(ReadWhiteSpaces(c));
  return ReadWhiteSpaces(getchar());
}

int main() {
  int c;
  ParseValue(getchar());
  c = getchar();
  if (c != '\n' && c != EOF) {
    printf("Unexpected char %c (%d)\n", c, c);
  }
  return 0;
}
