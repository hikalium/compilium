int puts(char *);
int printf(char *, int);
void exit(int);

void ExpectEq(int actual, int expected, int line) {
  printf("Line %3d: ", line);
  if (actual != expected) {
    puts("FAIL");
    exit(1);
  }
  puts("PASS");
}

int main(int argc, char **argv) {
  ExpectEq(1, 1, __LINE__);
  ExpectEq(017, 15, __LINE__);
  ExpectEq((0), 0, __LINE__);
  ExpectEq((1), 1, __LINE__);
  ExpectEq(1 ? 2 : 3, 2, __LINE__);
  puts("PASS all stmt tests");
  return 0;
}
