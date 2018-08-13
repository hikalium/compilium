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
  int var;
  struct KeyValue1 kv1;
  struct KeyValue2 kv2;
  struct KeyValue3 kv3;
  printf("sizeof(kv1) = %d\n", sizeof(kv1));
  printf("sizeof(kv2) = %d\n", sizeof(kv2));
  printf("sizeof(kv3) = %d\n", sizeof(kv3));

  kv1.value1 = 3;
  kv1.value2 = 5;
  kv1.key = "seven";

  printf("kv1.value1 = %d\n", kv1.value1);
  printf("kv1.value2 = %d\n", kv1.value2);
  printf("kv1.key = %s\n", kv1.key);

  struct KeyValue1 *kv1p;
  kv1p = &kv1;

  printf("kv1p->value1 = %d\n", kv1p->value1);
  printf("kv1p->value2 = %d\n", kv1p->value2);
  printf("kv1p->key = %s\n", kv1p->key);

  kv1p->value1 = kv1p->value1 + kv1p->value2;
  kv1p->value2 = kv1p->value1 + kv1p->value2;
  kv1p->key = kv1.key;

  printf("kv1.value1 = %d\n", kv1.value1);
  printf("kv1.value2 = %d\n", kv1.value2);
  printf("kv1.key = %s\n", kv1.key);

  return 0;
}
