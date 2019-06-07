int puts(char *s);
void exit(int exit_code);

int ExpectEq(int actual, int expected) {
  puts(actual == expected ? "PASS" : "FAIL");
}

int main(int argc, char **argv) {
  puts("statement_tests:");
  ExpectEq(1, 1);
  return 0;
}
