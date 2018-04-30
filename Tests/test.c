int main(int argc, char **argv)
{
  puts("Hello, world!");
  printf("argc: %d\n", argc);
  for(int i = 0; i < argc; i++){
    printf("argv[%d] = %s\n", i, argv[i]);
    if(i % 2){
      puts("odd!");
    } else{
      puts("even!");
    }
  }
  return 0;
}

