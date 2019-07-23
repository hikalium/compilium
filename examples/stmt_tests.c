int puts(char *s);
int printf(char *);
void exit(int exit_code);

void ExpectEq(int actual, int expected, char *case_name) {
  printf(case_name);
  printf("\t");
  if (actual != expected) {
    puts("FAIL");
    exit(1);
  }
  puts("PASS");
}

int main(int argc, char **argv) {
  puts("statement_tests:");
  ExpectEq(1, 1, "1 == 1");
  ExpectEq(1, 0, "1 == 0");
  puts("PASS");
  return 0;
}
