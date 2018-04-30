#include "compilium.h"

int main(int argc, char *argv[]) {
  if (argc < 2) return 1;

  char *input = ReadFile(argv[1]);
  Tokenize(input);
  free(input);

  ASTNodeList *ast = Parse();

  printf("ASTRoot: ");
  PrintASTNodeList(ast, 0);
  putchar('\n');

  return 0;
}
