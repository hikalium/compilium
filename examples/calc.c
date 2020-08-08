#include <stdio.h>

#define MAX_INPUT_SIZE 128

int main() {
  char s[MAX_INPUT_SIZE + 1];
  s[0] = 0;
  for (int i = 0; i < MAX_INPUT_SIZE; i++) {
    int c = getchar();
    // if(c == EOF || c == '\n') break;
    s[i] = c;
    s[i + 1] = 0;
  }
  for (int i = 0; s[i]; i++) {
    putchar(s[i]);
  }
  putchar('\n');
  return 0;
}
