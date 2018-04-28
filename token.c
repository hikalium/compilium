#include "compilium.h"

Token tokens[MAX_TOKENS];
int tokens_count;

void AddToken(const char *begin, const char *end, TokenType type) {
  if (end <= begin || (end - begin) >= MAX_TOKEN_LEN) {
    Error("Too long token");
  }
  if (tokens_count >= MAX_TOKENS) {
    Error("Too many token");
  }
  strncpy(tokens[tokens_count].str, begin, end - begin);
  tokens[tokens_count].str[end - begin] = 0;
  tokens[tokens_count].type = type;
  printf("[%s](%d)", tokens[tokens_count].str, tokens[tokens_count].type);
  tokens_count++;
}

int IsEqualToken(const Token *token, const char *s) {
  if (!token) return 0;
  return strcmp(token->str, s) == 0;
}

const Token *GetTokenAt(int index) {
  if (index < 0 || tokens_count <= index) return NULL;
  return &tokens[index];
}

int GetNumOfTokens() {
  return tokens_count;
}

void SetNumOfTokens(int num_of_tokens){
  tokens_count = num_of_tokens;
}

TokenList *AllocateTokenList() {
  TokenList *list = malloc(sizeof(TokenList));
  list->used = 0;
  return list;
}

void AppendTokenToList(TokenList *list, const Token *token) {
  if (list->used >= TOKEN_LIST_SIZE) {
    Error("No more space in TokenList");
  }
  list->tokens[list->used++] = token;
}

void PrintTokenList(const TokenList *list) {
  for (int i = 0; i < list->used; i++) {
    printf("%s ", list->tokens[i]->str);
  }
}

