int putchar(int);

int main() {
  for (int y = 0; y < 10; y++) {
    for (int x = 0; x <= y; x++) {
      putchar('*');
    }
    putchar('\n');
  }
  return 0;
}
