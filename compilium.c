#include "compilium.h"

#define MAX_TOKENS 2048

int main(int argc, char *argv[]) {
  if (argc < 3) {
    Error("Usage: %s <src_c_file> <dst_S_file>", argv[0]);
  }

  char *input = ReadFile(argv[1]);
  TokenList *tokens = AllocateTokenList(MAX_TOKENS);
  Tokenize(tokens, input);
  free(input);

  ASTNode *ast = Parse(tokens);

  PrintASTNode(ast, 1); putchar('\n');

  FILE *dst_fp = fopen(argv[2], "wb");
  if(!dst_fp){
    Error("Failed to open %s", argv[2]);
  }
  Generate(dst_fp, ast);
  fclose(dst_fp);

  return 0;
}
