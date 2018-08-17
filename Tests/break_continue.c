int putchar(int);
int printf(const char *, ...);

int main() {
  int i;

  for (i = 0; i < 7; i++) {
    printf("%d", i);
    putchar('A');
    if (i == 3) {
      putchar('X');
      continue;
    }
    putchar('B');
    if (i == 5) {
      putchar('Y');
      break;
    }
    putchar('C');
  }
  printf("Z%d\n", i);

  i = 0;
  while (i++ < 7) {
    printf("%d", i);
    putchar('A');
    if (i == 3) {
      putchar('X');
      continue;
    }
    putchar('B');
    if (i == 5) {
      putchar('Y');
      break;
    }
    putchar('C');
  }
  printf("Z%d\n", i);

  return 0;
}
