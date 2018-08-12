struct KeyValue1 {
  const char *key;
  int value1;
  int value2;
};

struct KeyValue2 {
  int value1;
  const char *key;
  int value2;
};

struct KeyValue3 {
  int value1;
  int value2;
  const char *key;
};

int main(int argc, char **argv)
{
  struct KeyValue1 kv1;
  struct KeyValue2 kv2;
  struct KeyValue3 kv3;
  printf("sizeof(kv1) = %d\n", sizeof(kv1));
  printf("sizeof(kv2) = %d\n", sizeof(kv2));
  printf("sizeof(kv3) = %d\n", sizeof(kv3));
  return 0;
}
