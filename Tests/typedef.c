typedef struct KEY_VALUE KeyValue;
struct KEY_VALUE {
  const char *key;
  int value;
};

typedef struct {
  unsigned int gp_offset;
  unsigned int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} va_list[1];

int main()
{
  KeyValue kv1;
  va_list va;
  kv1.key = "seven";
  kv1.value = 7;
  printf("kv[%s] = %d\n", kv1.key, kv1.value);
  va[0].gp_offset = 3;
  va[0].fp_offset = 4;
  printf("va[0].gp_offset = %d\n", va[0].gp_offset);
  printf("va[0].fp_offset = %d\n", va[0].fp_offset);
  return 0;
}
