#include "compilium.h"

#define MAX_TOKENS 2048
int main(int argc, char *argv[]) {
  KernelType kernel_type = kKernelDarwin;
  const char *src_file = NULL;
  const char *dst_file = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--prefix_type") == 0) {
      i++;
      if (strcmp(argv[i], "Darwin") == 0) {
        kernel_type = kKernelDarwin;
      } else if (strcmp(argv[i], "Linux") == 0) {
        kernel_type = kKernelLinux;
      } else {
        Error("Unknown kernel type %s", argv[i]);
      }
    } else if (strcmp(argv[i], "-o") == 0) {
      dst_file = argv[i];
    } else {
      src_file = argv[i];
    }
  }
  if (!src_file || !dst_file) {
    Error("Usage: %s -o <dst_file> <src_file>", argv[0]);
  }

  InitASTTypeName();
  InitILOpTypeName();

  char *input = ReadFile(src_file);
  TokenList *tokens = AllocateTokenList(MAX_TOKENS);
  Tokenize(tokens, input, src_file);
  free(input);

  puts("\nTokens:");
  PrintTokenList(tokens);
  putchar('\n');

  ASTNode *ast = Parse(tokens);

  puts("\nAST:");
  PrintASTNode(ast, 0);
  putchar('\n');

  puts("\nCode generation:");
  FILE *dst_fp = fopen(argv[2], "wb");
  if (!dst_fp) {
    Error("Failed to open %s", dst_file);
  }
  Generate(dst_fp, ast, kernel_type);
  fclose(dst_fp);

  return 0;
}
