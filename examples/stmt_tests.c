int puts(char *s);
void exit(int exit_code);

void ExpectEq(int actual, int expected) {
  if(actual == expected) return;
  puts("FAIL");
  exit(1);
}

int main(int argc, char **argv) {
  puts("statement_tests:");
  ExpectEq(1, 1);
  puts("PASS");
  return 0;
}
