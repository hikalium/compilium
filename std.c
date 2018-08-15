#include "compilium.h"

FILE *stdin;
FILE *stdout;
FILE *stderr;

void InitStd() {
  stdin = __stdinp;
  stdout = __stdoutp;
  stderr = __stderrp;
}
