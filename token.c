#include "compilium.h"

bool IsToken(struct Node *n) {
  return n && kTokenLowerBound < n->type && n->type < kTokenUpperBound;
}

struct Node *AllocToken(const char *src_str, const char *begin, int length,
                        enum NodeType type) {
  struct Node *t = AllocNode(type);
  t->begin = begin;
  t->length = length;
  t->type = type;
  t->src_str = src_str;
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

void PrintToken(struct Node *t) {
  fprintf(stderr, "(Token %.*s type=%d)", t->length, t->begin, t->type);
}

void PrintTokenBrief(struct Node *t) {
  assert(t);
  if (t->type == kTokenStringLiteral || t->type == kTokenCharLiteral) {
    fprintf(stderr, "%.*s", t->length, t->begin);
    return;
  }
  fprintf(stderr, "<%.*s>", t->length, t->begin);
}

void PrintTokenStrToFile(struct Node *t, FILE *fp) {
  fprintf(fp, "%.*s", t->length, t->begin);
}
