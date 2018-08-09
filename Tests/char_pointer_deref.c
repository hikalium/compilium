int main()
{
  char *p;
  int i;
  p = "compilium";
  for(i = 0; i < 9; i++){
    printf("*(p + %d) = '%c'\n", i, *(p + i));
  }
  return 0;
}
