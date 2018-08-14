#include "compilium.h"

void TestTokenStream() {
  TokenList *list = AllocTokenList(3);
  AppendTokenToList(list, AllocToken("one", kIdentifier));
  AppendTokenToList(list, AllocToken("two", kIdentifier));
  AppendTokenToList(list, AllocToken("three", kIdentifier));

  TokenStream *stream = AllocAndInitTokenStream(list);

  const Token *token;
  token = PeekToken(stream);
  if (token != PeekToken(stream)) Error("PeekToken should not change pos");
  if (GetStreamPos(stream) != 0) Error("GetStreamPos() should return 0");
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
  if (GetStreamPos(stream) != 3) Error("GetStreamPos() should return 3");
  UnpopToken(stream);
  if (!IsNextToken(stream, "three"))
    Error("Next token should be 'three' after Unpop()");
  UnpopToken(stream);
  if (!IsNextToken(stream, "two"))
    Error("Next token should be 'two' after Unpop()");
  UnpopToken(stream);
  if (!IsNextToken(stream, "one"))
    Error("Next token should be 'one' after Unpop()");
  UnpopToken(stream);
  if (!IsNextToken(stream, "one"))
    Error("Unpop() should not move pos before firste element");
  if (GetStreamPos(stream) != 0) Error("GetStreamPos() should return 0");
  if (SeekStream(stream, -1) != 0)
    Error(
        "SeekStream() should keep its pos when trying to seek out of "
        "range(-1)");
  if (SeekStream(stream, 1) != 1) Error("SeekStream() can set pos to 1");
  if (SeekStream(stream, 3) != 3)
    Error("SeekStream() can set pos after last element");
  if (SeekStream(stream, 4) != 3)
    Error("SeekStream() should keep pos when trying to seek out of range(4)");

  puts("PASS TestTokenStream");
}

int main(int argc, char *argv[]) {
  InitStd();
  TestTokenStream();
  return 0;
}
