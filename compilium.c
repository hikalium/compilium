#include "compilium.h"

#define MAX_TOKENS 2048
int main(int argc, char *argv[]) {
  if (argc < 3) {
    Error("Usage: %s <src_c_file> <dst_S_file>", argv[0]);
  }

  const char *filename = argv[1];
  char *input = ReadFile(filename);
  TokenList *tokens = AllocateTokenList(MAX_TOKENS);
  Tokenize(tokens, input, argv[1]);
  free(input);

  puts("\nTokens:");
  PrintTokenList(tokens);
  putchar('\n');

  InitASTTypeName();
  ASTNode *ast = Parse(tokens);

  puts("\nAST:");
  PrintASTNode(ast, 0);
  putchar('\n');

  puts("\nCode generation:");
  FILE *dst_fp = fopen(argv[2], "wb");
  if (!dst_fp) {
    Error("Failed to open %s", argv[2]);
  }
  Generate(dst_fp, ast);
  fclose(dst_fp);

  return 0;
}
