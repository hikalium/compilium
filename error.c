#include <stdarg.h>
#include <stdio.h>

void exit(int);
#define EXIT_FAILURE 1

void Error(const char *fmt, ...) {
  fflush(stdout);

  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

void Warning(const char *fmt, ...) {
  fflush(stdout);

  fprintf(stderr, "Warning: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
}
