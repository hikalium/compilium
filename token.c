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

struct Node *DuplicateToken(struct Node *base_token) {
  assert(IsToken(base_token));
  struct Node *t = AllocNode(kNodeToken);
  memcpy(t, base_token, sizeof(*t));
  t->next_token = NULL;
  return t;
}

struct Node *DuplicateTokenSequence(struct Node *base_head) {
  struct Node *dup_head = NULL;
  struct Node **dup_head_holder = &dup_head;
  while (base_head) {
    *dup_head_holder = DuplicateToken(base_head);
    dup_head_holder = &(*dup_head_holder)->next_token;
    base_head = base_head->next_token;
  }
  return dup_head;
}

char *CreateTokenStr(struct Node *t) {
  assert(IsToken(t));
  return strndup(t->begin, t->length);
}

int IsEqualTokenWithCStr(struct Node *t, const char *s) {
  return IsToken(t) && strlen(s) == (unsigned)t->length &&
         strncmp(t->begin, s, t->length) == 0;
}

void PrintTokenSequence(struct Node *t) {
  if (!t) return;
  assert(IsToken(t));
  for (; t; t = t->next_token) {
    if (t->token_type == kTokenZeroWidthNoBreakSpace) {
      continue;
    }
    fprintf(stderr, "%.*s", t->length, t->begin);
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

// Token stream

static struct Node **next_token_holder;

void InitTokenStream(struct Node **head_token_holder) {
  assert(head_token_holder);
  next_token_holder = head_token_holder;
}

static void AdvanceTokenStream(void) {
  if (!*next_token_holder) return;
  next_token_holder = &(*next_token_holder)->next_token;
}

struct Node *PeekToken(void) {
  assert(next_token_holder);
  return *next_token_holder;
}

struct Node *ReadToken(enum TokenType type) {
  struct Node *t = *next_token_holder;
  if (!t || !IsTokenWithType(t, type)) return NULL;
  return t;
}

struct Node *ConsumeToken(enum TokenType type) {
  struct Node *t = *next_token_holder;
  if (!t || !IsTokenWithType(t, type)) return NULL;
  AdvanceTokenStream();
  return t;
}

struct Node *ConsumeTokenStr(const char *s) {
  struct Node *t = *next_token_holder;
  if (!t || !IsEqualTokenWithCStr(t, s)) return NULL;
  AdvanceTokenStream();
  return t;
}

struct Node *ExpectTokenStr(const char *s) {
  struct Node *t = *next_token_holder;
  if (!t) Error("Expect token %s but got EOF", s);
  if (!ConsumeTokenStr(s)) ErrorWithToken(t, "Expected token %s here", s);
  return t;
}

struct Node *ConsumePunctuator(const char *s) {
  struct Node *t = *next_token_holder;
  if (!t || !IsTokenWithType(t, kTokenPunctuator) ||
      !IsEqualTokenWithCStr(t, s))
    return NULL;
  AdvanceTokenStream();
  return t;
}

struct Node *ExpectPunctuator(const char *s) {
  struct Node *t = *next_token_holder;
  if (!t) Error("Expect token %s but got EOF", s);
  if (!ConsumePunctuator(s)) ErrorWithToken(t, "Expected token %s here", s);
  return t;
}

struct Node *NextToken(void) {
  struct Node *t = *next_token_holder;
  AdvanceTokenStream();
  return t;
}

void RemoveCurrentToken(void) {
  if (!*next_token_holder) return;
  *next_token_holder = (*next_token_holder)->next_token;
}

void RemoveTokensTo(struct Node *end) {
  while (*next_token_holder && *next_token_holder != end) {
    RemoveCurrentToken();
  }
}

void InsertTokens(struct Node *seq_first) {
  // Insert token sequece (seq) at current cursor pos.
  if (!IsToken(seq_first)) return;
  struct Node *seq_last = seq_first;
  while (seq_last->next_token) seq_last = seq_last->next_token;
  seq_last->next_token = PeekToken();
  *next_token_holder = seq_first;
}

static struct Node *CreateStringLiteralOfTokens(struct Node *head) {
  assert(IsToken(head));
  int len = 0;
  for (struct Node *t = head; t; t = t->next_token) {
    len += t->length;
  }
  char *s = malloc(len + 1 + 2);
  assert(s);
  char *p = s;
  *p = '"';
  p++;
  for (struct Node *t = head; t; t = t->next_token) {
    for (int i = 0; i < t->length; i++) {
      *p = t->begin[i];
      p++;
    }
  }
  *p = '"';
  p++;
  *p = 0;
  return AllocToken(s, 0, s, len + 2, kTokenStringLiteral);
}

void InsertTokensWithIdentReplace(struct Node *seq, struct Node *rep_list) {
  // Insert token sequece (seq) at current cursor pos.
  // if seq contains token in rep_list, replace it with tokens rep_list[token];
  // elements of seq will be inserted directly.
  if (!IsToken(seq)) return;
  struct Node **next_holder = next_token_holder;
  while (seq) {
    struct Node *e;
    if (IsEqualTokenWithCStr(seq, "#") && seq->next_token &&
        (e = GetNodeByTokenKey(rep_list, seq->next_token))) {
      struct Node *st = CreateStringLiteralOfTokens(e->value);
      seq = seq->next_token->next_token;
      //
      st->next_token = *next_holder;
      *next_holder = st;
      next_holder = &st->next_token;
      continue;
    }
    if (!(e = GetNodeByTokenKey(rep_list, seq))) {
      // no replace
      struct Node *n = seq;
      seq = seq->next_token;
      //
      n->next_token = *next_holder;
      *next_holder = n;
      next_holder = &(*next_holder)->next_token;
      continue;
    }
    struct Node *n = DuplicateTokenSequence(e->value);
    struct Node *n_last = n;
    while (n_last->next_token) n_last = n_last->next_token;
    seq = seq->next_token;
    //
    n_last->next_token = *next_holder;
    *next_holder = n;
    next_holder = &n_last->next_token;
  }
}

struct Node **RemoveDelimiterTokens(struct Node **head_holder) {
  InitTokenStream(head_holder);
  for (;;) {
    struct Node *t = PeekToken();
    if (!t) break;
    if (ShouldRemoveToken(t)) {
      RemoveCurrentToken();
      continue;
    }
    AdvanceTokenStream();
  }
  return head_holder;
}
