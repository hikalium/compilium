#include "compilium.h"

void ExpectASTType(const char *description, const ASTNode *ast, ASTType type)
{
  if(ast && ast->type == type){
    printf("PASS: %s\n", description);
  } else if(ast){
    printf("FAIL: %s expected AST type %d but got %d\n", description, type, ast->type);
  } else{
    printf("FAIL: %s expected AST type %d but got NullAST\n", description, type);
  }
}

void ExpectNullAST(const char *description, const ASTNode *ast)
{
  if(!ast){
    printf("PASS: %s\n", description);
  } else{
    printf("FAIL: %s expected null AST but got not null\n", description);
  }
}

ASTNode *TryReadExpressionStatement(int index, int *after_index);

void TestParsingExpression(){
  int after_index;

  SetNumOfTokens(0);
  Tokenize("3;"); putchar('\n');
  ASTNode *expr_stmt = TryReadExpressionStatement(0, &after_index);
  ExpectASTType("A statement contains a number is parsed as ExpressionStatement", expr_stmt, kExprStmt);
}

int main(int argc, char *argv[])
{
  TestParsingExpression();
  return 0;
}
