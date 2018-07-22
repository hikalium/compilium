int printf(const char *s, ...);

int main()
{
  int i;
  int k;
  for(i = 2; i < 100; i++){
    for(k = 2; k < i / 2; k++){
      if(i % k == 0) break;
    }
    if(k >= i / 2){
      printf("%d\n", i);
    }
  }
  return 0;
}
