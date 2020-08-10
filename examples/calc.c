#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 128

void assert(int v) {
  if (v) return;
  printf("Assertion failed!");
}

void PrintPtr(int p) {
  for (int i = 15; i >= 0; i--) {
    int v = (p >> (i * 4)) & 15;
    if (v < 10)
      putchar('0' + v);
    else
      putchar('A' + (v - 10));
  }
  putchar('\n');
}

#define INITIAL_INPUT_SIZE 8192
static const char *ReadLine(void) {
  PrintPtr(1024);
  PrintPtr(1024 * 1024);
  PrintPtr(1024 * 1024 * 1024);
  PrintPtr(1024 * 1024 * 1024 * 1024);
  int buf_size = INITIAL_INPUT_SIZE;
  char *input = NULL;
  int input_size = 0;
  int c;
  input = malloc(buf_size);
  PrintPtr(input);
  while ((c = getchar()) != EOF && c != '\n') {
    PrintPtr(&input[input_size]);
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

int main() {
  fputs("Hello stderr!\n", stderr);
  const char *input = ReadLine();
  PrintPtr(input);
  for (int i = 0; input[i]; i++) {
    putchar(input[i]);
  }
  putchar('\n');
  return 0;
}
