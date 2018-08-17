#define NULL 0

int putchar(int);

int func()
{
  putchar('A');
  return 1;
}

int main()
{
  int a;
  int *p = NULL;

  putchar('P');
  if(p && func()){
    putchar('Q');
  }
  putchar('\n');

  p = &a;
  putchar('R');
  if(p && func()){
    putchar('S');
  }
  putchar('\n');

  return 0;
}
