int printf(const char *, ...);

int main()
{
  int array[4];
  int i;
  for(i = 0; i < 4; i++){
    array[i] = i;
  }
  for(i = 0; i < 4; i++){
    printf("array[%d] = %d\n", i, array[i]);
  }
  return 0;
}
