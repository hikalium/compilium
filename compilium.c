#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compilium.h"

#define MAX_TOKENS 2048
int main(int argc, char *argv[]) {
  KernelType kernel_type = kKernelDarwin;
  int is_parse_only = 0;
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
    } else if (strcmp(argv[i], "--parse_only") == 0) {
      is_parse_only = 1;
    } else if (strcmp(argv[i], "-o") == 0) {
      dst_file = argv[i];
    } else {
      src_file = argv[i];
    }
  }
  if (!dst_file) {
    dst_file = "out.S";
  }
  if (!src_file) {
    Error("Usage: %s -o <dst_file> <src_file>", argv[0]);
  }

  InitASTNodeTypeName();
  InitILOpTypeName();

  char *input = ReadFile(src_file);
  TokenList *tokens = AllocTokenList(MAX_TOKENS);
  Tokenize(tokens, input, src_file);
  free(input);

  puts("\nTokens:");
  PrintTokenList(tokens);
  putchar('\n');

  ASTNode *ast = Parse(tokens);
  Analyze(ast);

  puts("\nAST:");
  PrintASTNode(ast, 0);
  putchar('\n');

  if (is_parse_only) return 0;

  ASTList *il = GenerateIL(ast);

  puts("\nIL:");
  PrintASTNode(ToASTNode(il), 0);
  putchar('\n');

  puts("\nCode generation:");
  FILE *dst_fp = fopen(argv[2], "wb");
  if (!dst_fp) {
    Error("Failed to open %s", dst_file);
  }
  GenerateCode(dst_fp, il, kernel_type);
  fclose(dst_fp);

  return 0;
}
