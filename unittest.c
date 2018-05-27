#include "compilium.h"

void ExpectASTType(const char *description, const ASTNode *ast, ASTType type) {
  if (ast && ast->type == type) {
    printf("PASS: %s\n", description);
  } else if (ast) {
    printf("FAIL: %s expected AST type %d but got %d\n", description, type,
           ast->type);
  } else {
    printf("FAIL: %s expected AST type %d but got NullAST\n", description,
           type);
  }
}

void ExpectNullAST(const char *description, const ASTNode *ast) {
  if (!ast) {
    printf("PASS: %s\n", description);
  } else {
    printf("FAIL: %s expected null AST but got not null\n", description);
  }
}

void TestParsingExpression() {
  TokenList *tokens = AllocateTokenList(64);

  Tokenize(tokens, "int main(){return 0;}");
  ASTNode *expr_stmt = Parse(tokens);
  ExpectASTType("Simple return source is parsed successfully", expr_stmt,
                kRoot);
}

int main(int argc, char *argv[]) {
  TestParsingExpression();
  return 0;
}
