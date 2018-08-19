int vprintf(const char *, __builtin_va_list);
int printf(const char *, ...);
int putchar(int);

void vptest1(const char *fmt, ...) {
  printf("vptest1: ");
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  vprintf(fmt, ap);
  putchar('\n');
}

void vptest2(int a, const char *fmt, ...) {
  printf("vptest2: a = %d, ", a);
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  vprintf(fmt, ap);
  putchar('\n');
}

int main(){
  int a = 29;
  vptest1("This is a test %s %s %s", "two", "three", "four");
  vptest1("This is a test %s %d 0x%X", "str", a, a);
  vptest2(17, "This is a test %s %d 0x%X", "str", a, a);
  return 0;
}
