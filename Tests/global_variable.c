int printf(const char *, ...);
int a;

void print_and_increment_a()
{
  printf("a = %d\n", a);
  a++;
}

int main()
{
  for(int i = 0; i < 10; i++){
    print_and_increment_a();
  }
  return 0;
}
