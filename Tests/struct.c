struct KeyValue {
  const char *key;
  int value;
};

int main(int argc, char **argv)
{
  struct KeyValue kv;
  printf("size = %d\n", sizeof(kv));
  return 0;
}
