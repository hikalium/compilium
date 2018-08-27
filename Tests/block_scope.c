int printf(const char *, ...);

int main()
{
  int a = 3;
  {
    int a = 5;
    {
      int a = 7;
      printf("a = %d\n", a);
    }
    printf("a = %d\n", a);
  }
  printf("a = %d\n", a);
  return 0;
}
