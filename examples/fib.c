// https://oeis.org/A000045
#include <stdio.h>

int fib(int n) {
  // assume n >= 0
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
}

int main() {
  for (int i = 0; i < 41; i++) {
    printf("fib[%2d] = %d\n", i, fib(i));
  }
  return 0;
}
