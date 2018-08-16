int func(int, int);
int func_unsigned(int, unsigned int);
int func_unsigned_vaarg(int, unsigned int, ...);
int func_noparam();
int func_void(void);
void func_returns_void(void);
unsigned int func_returns_unsigned_int(void);

int main()
{
  int a;
  unsigned int b;
  const char *c;
  int array1[3 + 4];
  int array2[3 * 4];

  return 0;
}
