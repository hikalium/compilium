#include "compilium.h"

FILE *stdin;
FILE *stdout;
FILE *stderr;

void InitStd() {
  stdin = fdopen(0, "r");
  stdout = fdopen(1, "w");
  stderr = fdopen(2, "w");
}
