int puts(char*);
int printf(const char*, ...);
void exit(int);

int global_val;

void ExpectEq(int actual, int expected, int line) {
  printf("Line %3d: ", line);
  if (actual != expected) {
    puts("FAIL");
    exit(1);
  }
  puts("PASS");
}

void gf1() {
  ExpectEq(global_val, 0, __LINE__);
  global_val++;  
  ExpectEq(global_val, 1, __LINE__);
}

void gf2() {
  ExpectEq(global_val, 1, __LINE__);
  global_val++;  
  ExpectEq(global_val, 2, __LINE__);
}

void TestGlobal() {
  global_val = 0;  
  ExpectEq(global_val, 0, __LINE__);
  gf1();
  gf2();
  ExpectEq(global_val, 2, __LINE__);
}

int main() {
  TestGlobal();
  return 0;
}
