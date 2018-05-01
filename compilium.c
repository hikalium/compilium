#include "compilium.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    Error("Usage: %s <src_c_file> <dst_S_file>", argv[0]);
  }

  char *input = ReadFile(argv[1]);
  Tokenize(input);
  free(input);

  ASTNode *ast = Parse();

  PrintASTNode(ast, 1); putchar('\n');

  FILE *dst_fp = fopen(argv[2], "wb");
  if(!dst_fp){
    Error("Failed to open %s", argv[2]);
  }
  Generate(dst_fp, ast);
  fclose(dst_fp);

  return 0;
}
