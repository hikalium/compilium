#include <stdio.h>

int main(int argc, char *argv[]) {
  printf(".intel_syntax\n");
  printf(".text\n");
  printf(".global _main\n");
  printf("_main:\n");
  printf("mov rax, %s\n", argv[1]);
  printf("ret\n");
  return 0;
}
