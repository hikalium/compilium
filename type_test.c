#include "compilium.h"

void TestSizeOfASTType() {
  ASTType *type;

  type = AllocAndInitBasicType(kTypeChar);
  assert(GetSizeOfType(type) == 1);

  type = AllocAndInitBasicType(kTypeInt);
  assert(GetSizeOfType(type) == 4);

  type = AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeChar));
  assert(GetSizeOfType(type) == 8);

  type = AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeInt));
  assert(GetSizeOfType(type) == 8);

  type = AllocAndInitASTTypeArrayOf(AllocAndInitBasicType(kTypeChar), 3);
  assert(GetSizeOfType(type) == 3);

  type = AllocAndInitASTTypeArrayOf(AllocAndInitBasicType(kTypeInt), 3);
  assert(GetSizeOfType(type) == 12);

  type = AllocAndInitASTTypePointerOf(
      AllocAndInitASTTypeArrayOf(AllocAndInitBasicType(kTypeChar), 3));
  assert(GetSizeOfType(type) == 8);

  puts("PASS Size of ASTType");
}

ASTDecl *ParseDecl(TokenStream *stream);

ASTType *ParseDeclAndGetType(TokenList *tokens, const char *src) {
  SetSizeOfTokenList(tokens, 0);
  Tokenize(tokens, src, NULL);
  TokenStream *stream = AllocAndInitTokenStream(tokens);
  ASTDecl *decl = ParseDecl(stream);
  assert(decl);
  ASTDecltor *first_decltor =
      decl->init_decltors ? ToASTDecltor(GetASTNodeAt(decl->init_decltors, 0))
                          : NULL;
  return AllocAndInitASTType(decl->decl_specs, first_decltor);
}

void TestParsingDeclIntoASTType() {
  TokenList *tokens = AllocTokenList(32);
  ASTType *type;

  type = ParseDeclAndGetType(tokens, "int a;");
  assert(IsEqualASTType(type, AllocAndInitBasicType(kTypeInt)));

  type = ParseDeclAndGetType(tokens, "char a;");
  assert(IsEqualASTType(type, AllocAndInitBasicType(kTypeChar)));

  type = ParseDeclAndGetType(tokens, "int *a;");
  assert(IsEqualASTType(
      type, AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeInt))));

  type = ParseDeclAndGetType(tokens,
                             "struct KeyValue {"
                             "  char *key;"
                             "  int value;"
                             "};");
  assert(GetSizeOfType(type) == 16);

  type = ParseDeclAndGetType(tokens, "int puts(const char *s);");
  assert(IsBasicType(type, kTypeFunction));
  assert(IsEqualToken(GetIdentTokenOfType(type), "puts"));

  puts("PASS Parsing Decl into ASTType");
}

int main(int argc, char *argv[]) {
  InitASTNodeTypeName();

  TestSizeOfASTType();
  TestParsingDeclIntoASTType();
  return 0;
}
