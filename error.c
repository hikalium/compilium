#include "compilium.h"

_Noreturn void Error(const char *fmt, ...) {
  fflush(stdout);
  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

_Noreturn void ErrorWithASTNode(void *node, const char *fmt, ...) {
  fflush(stdout);
  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  PrintASTNode(node, 0);
  putchar('\n');
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

void assert(int value) {
  if (!value) Error("Assertion failed!");
}
