#include "compilium.h"

void ExpectASTIdent(ASTNode *node, const char *expected_str) {
  ASTIdent *ident = ToASTIdent(node);
  if (!ident) Error("FAIL: expected ASTIdent, got NULL");
  if (!IsEqualToken(ident->token, expected_str))
    Error("FAIL: expected token is %s, got %s", expected_str,
          ident->token->str);
}

void ExpectExactASTNode(ASTNode *actual, ASTNode *expected) {
  if (actual != expected)
    Error("FAIL: expected ASTNode %p, got %p", expected, actual);
}

void TestASTDict() {
  ASTDict *dict = AllocASTDict(5);
  Token *token_one = AllocToken("one", kIdentifier);
  Token *token_two = AllocToken("2", kIdentifier);
  Token *token_three = AllocToken("III", kIdentifier);
  AppendASTNodeToDict(dict, "one", ToASTNode(AllocAndInitASTIdent(token_one)));
  AppendASTNodeToDict(dict, "two", ToASTNode(AllocAndInitASTIdent(token_two)));
  AppendASTNodeToDict(dict, "three",
                      ToASTNode(AllocAndInitASTIdent(token_three)));

  ExpectASTIdent(FindASTNodeInDict(dict, "one"), "one");
  ExpectASTIdent(FindASTNodeInDict(dict, "two"), "2");
  ExpectASTIdent(FindASTNodeInDict(dict, "three"), "III");
  ExpectExactASTNode(FindASTNodeInDict(dict, "four"), NULL);

  puts("PASS TestASTDict");
}

int main(int argc, char *argv[]) {
  TestASTDict();
  return 0;
}
