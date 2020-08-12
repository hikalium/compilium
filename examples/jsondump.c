// https://www.json.org/json-en.html
#include <stdio.h>
#include <stdlib.h>

int TryValueBool(int c) {
  if (c == 't') {
    if ((c = getchar() != 'r') || (c = getchar()) != 'u' ||
        (c = getchar()) != 'e') {
      printf("Unexpected char %c\n", c);
      exit(1);
    }
    printf("> true\n");
    return 1;
  }
  if (c == 'f') {
    if ((c = getchar() != 'a') || (c = getchar()) != 'l' ||
        (c = getchar()) != 's' || (c = getchar()) != 'e') {
      printf("Unexpected char %c\n", c);
      exit(1);
    }
    printf("> false\n");
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
    printf("> null\n");
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
  printf("[\n");
  for (;;) {
    if (c == ']') break;
    if (c == ',') {
      c = ReadWhiteSpaces(getchar());
      continue;
    }
    c = ReadElement(c);
  }
  printf("]\n");
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
  if ((c = getchar()) != '\n') {
    printf("Unexpected char %c (%d)\n", c, c);
  }
  return 0;
}
