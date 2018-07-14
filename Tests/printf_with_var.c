int printf(const char *s, ...);

int main()
{
  int foo;
  foo = 3;
  int bar;
  bar = 7;
  printf("foo - bar = %d, foo = %d, bar = %d\n", foo - bar, foo, bar);
  return 0;
}
