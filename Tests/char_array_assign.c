int main()
{
  char s[11];
  int i;
  printf("size = %d\n", sizeof(s));
  for(i = 0; i < 10; i++){
    s[i] = 'A' + i;
  }
  s[10] = 0;
  printf("%s\n", s);
  return 0;
}
