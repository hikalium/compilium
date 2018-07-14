int printf(const char *fmt, ...);
int exit(int code);

int f(int n){
  if(n == 0) exit(32);
  printf("%d\n", n);
  return f(n - 1);
}

int main()
{
  f(10);
}
