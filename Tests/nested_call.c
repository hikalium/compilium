int puts(const char *s);

int f(){
  puts("f");
}

int g(){
  puts("g");
  f();
}

int h(){
  puts("h");
  g();
}

int main()
{
  puts("main");
  h();
  return 0;
}
