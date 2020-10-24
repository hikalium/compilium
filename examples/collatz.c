#include <stdio.h>

static int collatz(int n) {
  // printf("%d->", n);
  if (n == 1) return 1;
  if (n & 1) {
    n = 3 * n + 1;
  } else {
    n /= 2;
  }
  return collatz(n) + 1;
}

int main() {
  for (int k = 0; k < 10; k++) {
    for (int i = 1; i < 100000; i++) {
      printf("%d: ", i);
      int cycle = collatz(i);
      printf("OK! cycle = %d\n", cycle);
    }
  }
  return 0;
}
