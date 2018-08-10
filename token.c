#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compilium.h"

static void CopyTokenStr(Token *token, const char *s, size_t len) {
  if (len >= MAX_TOKEN_LEN) {
    Error("Too long token");
  }
  strncpy(token->str, s, len);
  token->str[len] = 0;
}

Token *AllocToken(const char *s, TokenType type) {
  if (!s) {
    Error("Trying to allocate a token with a null string");
  }
  Token *token = malloc(sizeof(Token));
  CopyTokenStr(token, s, strlen(s));
  token->type = type;
  return token;
}

Token *AllocTokenWithSubstring(const char *begin, const char *end,
                               TokenType type, const char *filename, int line) {
  Token *token = malloc(sizeof(Token));
  CopyTokenStr(token, begin, end - begin);
  token->type = type;
  token->filename = filename;
  token->line = line;
  return token;
}

int IsEqualToken(const Token *token, const char *s) {
  if (!token) return 0;
  return strcmp(token->str, s) == 0;
}

int IsTypeToken(const Token *token) {
  return IsEqualToken(token, "int") || IsEqualToken(token, "char");
}

void DebugPrintToken(const Token *token) {
  if (!token) {
    printf("(Token: NULL)\n");
    return;
  }
  printf("(Token: '%s' type %d at %s:%d)\n", token->str, token->type,
         token->filename, token->line);
}

void PrintToken(const Token *token) { printf("%s", token->str); }

// TokenList

struct TOKEN_LIST {
  int capacity;
  int size;
  const Token *tokens[];
};

TokenList *AllocTokenList(int capacity) {
  TokenList *list =
      malloc(sizeof(TokenList) + sizeof(const Token *) * capacity);
  list->capacity = capacity;
  list->size = 0;
  return list;
}

void AppendTokenToList(TokenList *list, const Token *token) {
  if (list->size >= list->capacity) {
    Error("No more space in TokenList");
  }
  list->tokens[list->size++] = token;
}

const Token *GetTokenAt(const TokenList *list, int index) {
  if (!list || index < 0 || list->size <= index) return NULL;
  return list->tokens[index];
}

int GetSizeOfTokenList(const TokenList *list) { return list->size; }

void SetSizeOfTokenList(TokenList *list, int size) { list->size = size; }

void PrintTokenList(const TokenList *list) {
  for (int i = 0; i < list->size; i++) {
    if (i) putchar(' ');
    PrintToken(list->tokens[i]);
  }
}

// TokenStream

struct TOKEN_STREAM {
  const TokenList *list;
  int pos;
};

TokenStream *AllocAndInitTokenStream(const TokenList *list) {
  TokenStream *stream = malloc(sizeof(TokenStream));
  stream->list = list;
  stream->pos = 0;
  return stream;
}

const Token *PopToken(TokenStream *stream) {
  if (stream->pos >= stream->list->size) return NULL;
  return GetTokenAt(stream->list, stream->pos++);
}

void UnpopToken(TokenStream *stream) {
  stream->pos--;
  if (stream->pos < 0) stream->pos = 0;
}

int GetStreamPos(TokenStream *stream) { return stream->pos; }

int SeekStream(TokenStream *stream, int pos) {
  if (0 <= pos && pos <= stream->list->size) {
    stream->pos = pos;
  }
  return stream->pos;
}

const Token *PeekToken(const TokenStream *stream) {
  return GetTokenAt(stream->list, stream->pos);
}

int IsNextToken(TokenStream *stream, const char *str) {
  return IsEqualToken(GetTokenAt(stream->list, stream->pos), str);
}

int IsNextTokenInList(TokenStream *stream, const char *list[]) {
  int i;
  for (i = 0; list[i]; i++) {
    if (IsNextToken(stream, list[i])) return 1;
  }
  return 0;
}

const Token *ConsumeToken(TokenStream *stream, const char *str) {
  if (!IsNextToken(stream, str)) return NULL;
  return PopToken(stream);
}

const Token *ExpectToken(TokenStream *stream, const char *str) {
  if (!IsNextToken(stream, str)) {
    const Token *t = PeekToken(stream);
    Error("%s:%d Expected %s but got %s", t->filename, t->line, str, t->str);
  }
  return PopToken(stream);
}

void DebugPrintTokenStream(const char *s, const TokenStream *stream) {
  printf("%s ", s);
  DebugPrintToken(PeekToken(stream));
}
