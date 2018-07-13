#include "compilium.h"

void TestTokenStream() {
  TokenList *list = AllocateTokenList(3);
  AppendTokenToList(list, AllocateToken("one", kIdentifier));
  AppendTokenToList(list, AllocateToken("two", kIdentifier));
  AppendTokenToList(list, AllocateToken("three", kIdentifier));

  TokenStream *stream = AllocAndInitTokenStream(list);

  const Token *token;
  token = PeekToken(stream);
  if (token != PeekToken(stream)) Error("PeekToken should not change pos");
  if (!IsNextToken(stream, "one")) Error("Next token should be 'one'");
  if (!IsNextToken(stream, "one")) Error("IsNextToken should not change pos");
  if (IsNextToken(stream, "two"))
    Error("Next token should be 'one', not 'two'");
  if (token != PopToken(stream)) Error("PopToken should return next token");
  if (!IsNextToken(stream, "two")) Error("Next token should be 'two'");
  if (!ConsumeToken(stream, "two")) Error("Token 'two' should be consumed");
  if (ConsumeToken(stream, "two")) Error("Token 'two' should not be consumed");
  if (!ConsumeToken(stream, "three")) Error("Token 'three' should be consumed");
  if (ConsumeToken(stream, "four"))
    Error("Token 'four' should not be consumed");
  if (PeekToken(stream)) Error("PeekToken should return NULL");
  if (PopToken(stream)) Error("PopToken should return NULL");

  puts("PASS TestTokenStream");
}

int main(int argc, char *argv[]) {
  TestTokenStream();
  return 0;
}
