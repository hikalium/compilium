int printf(const char *fmt, ...);

int fib(int n){
  if(n == 0) return 0;
  if(n == 1) return 1;
  int v;
  v = fib(n - 1) + fib(n - 2);
  printf("fib(%d) = %d\n", n, v);
  return v;
}

int main(){
  fib(10);
  return 0;
}
