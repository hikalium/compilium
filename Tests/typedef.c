typedef struct KEY_VALUE KeyValue;
struct KEY_VALUE {
  const char *key;
  int value;
};

int main()
{
  KeyValue kv1;
  kv1.key = "seven";
  kv1.value = 7;
  printf("kv[%s] = %d\n", kv1.key, kv1.value);
  return 0;
}
