int puts(const char *puts);

int zero(){
  puts("zero");
  return 0;
}

int one(){
  puts("one");
  return 1;
}

int main(){
  puts("Only zero() should be evaluated");
  zero() && one();
  puts("one() and zero() should be evaluated");
  one() && zero();
  puts("zero() and one() should be evaluated");
  zero() || one();
  puts("Only one() should be evaluated");
  one() || zero();
  return 0;
}
