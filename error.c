#include <stdarg.h>
#include <stdio.h>

// #include <stdlib.h>
void exit(int);
#define EXIT_FAILURE 1

void Error(const char *fmt, ...) {
  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

