#include "compilium.h"

bool IsToken(struct Node *n) { return n && n->type == kNodeToken; }

bool IsTokenWithType(struct Node *n, enum TokenType token_type) {
  return IsToken(n) && n->token_type == token_type;
}

struct Node *AllocToken(const char *src_str, int line, const char *begin,
                        int length, enum TokenType type) {
  struct Node *t = AllocNode(kNodeToken);
  t->begin = begin;
  t->length = length;
  t->token_type = type;
  t->src_str = src_str;
  t->line = line;
  return t;
}

const char *CreateTokenStr(struct Node *t) {
  assert(IsToken(t));
  return strndup(t->begin, t->length);
}

int IsEqualTokenWithCStr(struct Node *t, const char *s) {
  return strlen(s) == (unsigned)t->length &&
         strncmp(t->begin, s, t->length) == 0;
}

void PrintTokenSequence(struct Node *t) {
  if (!t) return;
  assert(IsToken(t));
  while (t) {
    PrintASTNode(t);
    t = t->next_token;
  }
}

void OutputTokenSequenceAsCSource(struct Node *t) {
  if (!t) return;
  assert(IsToken(t));
  for (; t; t = t->next_token) {
    if (t->token_type == kTokenZeroWidthNoBreakSpace) {
      continue;
    }
    fprintf(stdout, "%.*s", t->length, t->begin);
  }
}

void PrintToken(struct Node *t) {
  fprintf(stderr, "(Token %.*s type=%d)", t->length, t->begin, t->token_type);
}

void PrintTokenBrief(struct Node *t) {
  assert(t);
  if (t->token_type == kTokenStringLiteral ||
      t->token_type == kTokenCharLiteral) {
    fprintf(stderr, "%.*s", t->length, t->begin);
    return;
  }
  fprintf(stderr, "<%.*s>", t->length, t->begin);
}

void PrintTokenStrToFile(struct Node *t, FILE *fp) {
  fprintf(fp, "%.*s", t->length, t->begin);
}

static bool ShouldRemoveToken(struct Node *t) {
  return t->token_type == kTokenDelimiter ||
         t->token_type == kTokenZeroWidthNoBreakSpace;
}

struct Node *RemoveDelimiterTokens(struct Node *head) {
  struct Node **t = &head;
  for (;;) {
    while (*t && ShouldRemoveToken(*t)) {
      *t = (*t)->next_token;
    }
    if (!*t) break;
    t = &(*t)->next_token;
  }
  return head;
}
