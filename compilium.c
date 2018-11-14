#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n);

struct CompilerArgs {
  const char *input;
};

const char *symbol_prefix;

#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))

enum TokenTypes {
  kTokenDecimalNumber,
  kTokenOctalNumber,
  kTokenPlus,
  kTokenStar,
  kTokenMinus,
  kTokenSlash,
  kTokenPercent,
  kTokenShiftLeft,
  kTokenLessThanEq,
  kTokenLessThan,
  kTokenShiftRight,
  kTokenGreaterThanEq,
  kTokenGreaterThan,
  kTokenEq,
  kTokenNotEq,
  kTokenBitAnd,
  kTokenBitXor,
  kTokenBitOr,
  kTokenBoolAnd,
  kTokenBoolOr,
  kTokenBitNot,
  kTokenBoolNot,
  kTokenConditional,
  kTokenColon,
  kTokenComma,
  kTokenSemicolon,
  kTokenIdent,
  kTokenKwReturn,
  kTokenKwInt,
  kTokenKwSizeof,
  kTokenLBrace,
  kTokenRBrace,
  kTokenLParen,
  kTokenRParen,
  kTokenAssign,
  kNumOfTokenTypeNames
};

struct Token {
  const char *begin;
  int length;
  enum TokenTypes type;
  const char *src_str;
};

#define NUM_OF_TOKENS 32
struct Token *tokens[NUM_OF_TOKENS];
int tokens_used;

_Noreturn void Error(const char *fmt, ...) {
  fflush(stdout);
  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

_Noreturn void __assert(const char *expr_str, const char *file, int line) {
  Error("Assertion failed: %s at %s:%d\n", expr_str, file, line);
}

const char *token_type_names[kNumOfTokenTypeNames];

void TestList(void);
void TestType(void);
void ParseCompilerArgs(struct CompilerArgs *args, int argc, char **argv) {
  args->input = NULL;
  symbol_prefix = "_";
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--target-os") == 0) {
      i++;
      if (strcmp(argv[i], "Darwin") == 0) {
        symbol_prefix = "_";
      } else if (strcmp(argv[i], "Linux") == 0) {
        symbol_prefix = "";
      } else {
        Error("Unknown os type %s", argv[i]);
      }
    } else if (strcmp(argv[i], "--run-unittest=List") == 0) {
      TestList();
    } else if (strcmp(argv[i], "--run-unittest=Type") == 0) {
      TestType();
    } else {
      args->input = argv[i];
    }
  }
  if (!args->input)
    Error("Usage: %s [--os_type=Linux|Darwin] src_string", argv[0]);
}

void InitTokenTypeNames() {
  token_type_names[kTokenDecimalNumber] = "DecimalNumber";
  token_type_names[kTokenOctalNumber] = "OctalNumber";
  token_type_names[kTokenPlus] = "Plus";
  token_type_names[kTokenStar] = "Star";
  token_type_names[kTokenMinus] = "Minus";
  token_type_names[kTokenSlash] = "Slash";
  token_type_names[kTokenPercent] = "Percent";
  token_type_names[kTokenShiftLeft] = "ShiftLeft";
  token_type_names[kTokenLessThanEq] = "LessThanEq";
  token_type_names[kTokenLessThan] = "LessThan";
  token_type_names[kTokenShiftRight] = "ShiftRight";
  token_type_names[kTokenGreaterThanEq] = "GreaterThanEq";
  token_type_names[kTokenGreaterThan] = "GreaterThan";
  token_type_names[kTokenEq] = "Eq";
  token_type_names[kTokenNotEq] = "NotEq";
  token_type_names[kTokenBitAnd] = "BitAnd";
  token_type_names[kTokenBitXor] = "BitXor";
  token_type_names[kTokenBitOr] = "BitOr";
  token_type_names[kTokenBoolAnd] = "BoolAnd";
  token_type_names[kTokenBoolOr] = "BoolOr";
  token_type_names[kTokenBitNot] = "BitNot";
  token_type_names[kTokenBoolNot] = "BoolNot";
  token_type_names[kTokenConditional] = "Conditional";
  token_type_names[kTokenColon] = "Colon";
  token_type_names[kTokenComma] = "Comma";
  token_type_names[kTokenSemicolon] = "Semicolon";
  token_type_names[kTokenIdent] = "Ident";
  token_type_names[kTokenKwReturn] = "`return`";
  token_type_names[kTokenKwInt] = "`int`";
  token_type_names[kTokenKwSizeof] = "`sizeof`";
  token_type_names[kTokenLBrace] = "LBrace";
  token_type_names[kTokenRBrace] = "RBrace";
  token_type_names[kTokenLParen] = "LParen";
  token_type_names[kTokenRParen] = "RParen";
  token_type_names[kTokenAssign] = "Assign";
}

const char *GetTokenTypeName(enum TokenTypes type) {
  assert(0 <= type && type < kNumOfTokenTypeNames);
  assert(token_type_names[type]);
  return token_type_names[type];
}

struct Token *AllocToken(const char *src_str, const char *begin, int length,
                         enum TokenTypes type) {
  struct Token *t = calloc(1, sizeof(struct Token));
  t->begin = begin;
  t->length = length;
  t->type = type;
  t->src_str = src_str;
  return t;
}

void AddToken(struct Token *t) {
  assert(tokens_used < NUM_OF_TOKENS);
  tokens[tokens_used++] = t;
}

const char *CreateTokenStr(struct Token *t) {
  return strndup(t->begin, t->length);
}

int IsEqualTokenWithCStr(struct Token *t, const char *s) {
  return strlen(s) == t->length && strncmp(t->begin, s, t->length) == 0;
}

struct Token *CreateNextToken(const char **p, const char *src) {
  while (*(*p) <= ' ') {
    if (!*(*p)) return NULL;
    (*p)++;
  }
  if ('1' <= *(*p) && *(*p) <= '9') {
    int length = 0;
    while ('0' <= (*p)[length] && (*p)[length] <= '9') {
      length++;
    }
    struct Token *t = AllocToken(src, (*p), length, kTokenDecimalNumber);
    (*p) += length;
    return t;
  } else if ('0' == *(*p)) {
    int length = 0;
    while ('0' <= (*p)[length] && (*p)[length] <= '7') {
      length++;
    }
    struct Token *t = AllocToken(src, (*p), length, kTokenOctalNumber);
    (*p) += length;
    return t;
  } else if (('A' <= *(*p) && *(*p) <= 'Z') || ('a' <= *(*p) && *(*p) <= 'z') ||
             *(*p) == '_') {
    int length = 0;
    while (('A' <= (*p)[length] && (*p)[length] <= 'Z') ||
           ('a' <= (*p)[length] && (*p)[length] <= 'z') ||
           (*p)[length] == '_' ||
           ('0' <= (*p)[length] && (*p)[length] <= '9')) {
      length++;
    }
    struct Token *t = AllocToken(src, (*p), length, kTokenIdent);
    if (IsEqualTokenWithCStr(t, "return")) t->type = kTokenKwReturn;
    if (IsEqualTokenWithCStr(t, "int")) t->type = kTokenKwInt;
    if (IsEqualTokenWithCStr(t, "sizeof")) t->type = kTokenKwSizeof;
    (*p) += length;
    return t;
  } else if ('&' == *(*p)) {
    if ((*p)[1] == '&') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenBoolAnd);
      (*p) += 2;
      return t;
    }
    return AllocToken(src, (*p)++, 1, kTokenBitAnd);
  } else if ('|' == *(*p)) {
    if ((*p)[1] == '|') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenBoolOr);
      (*p) += 2;
      return t;
    }
    return AllocToken(src, (*p)++, 1, kTokenBitOr);
  } else if ('<' == *(*p)) {
    if ((*p)[1] == '<') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenShiftLeft);
      (*p) += 2;
      return t;
    } else if ((*p)[1] == '=') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenLessThanEq);
      (*p) += 2;
      return t;
    }
    return AllocToken(src, (*p)++, 1, kTokenLessThan);
  } else if ('>' == *(*p)) {
    if ((*p)[1] == '>') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenShiftRight);
      (*p) += 2;
      return t;
    } else if ((*p)[1] == '=') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenGreaterThanEq);
      (*p) += 2;
      return t;
    }
    return AllocToken(src, (*p)++, 1, kTokenGreaterThan);
  } else if ('=' == *(*p)) {
    if ((*p)[1] == '=') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenEq);
      (*p) += 2;
      return t;
    }
    return AllocToken(src, (*p)++, 1, kTokenAssign);
  } else if ('!' == *(*p)) {
    if ((*p)[1] == '=') {
      struct Token *t = AllocToken(src, (*p), 2, kTokenNotEq);
      (*p) += 2;
      return t;
    }
    return AllocToken(src, (*p)++, 1, kTokenBoolNot);
  } else if ('^' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenBitXor);
  } else if ('+' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenPlus);
  } else if ('-' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenMinus);
  } else if ('*' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenStar);
  } else if ('/' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenSlash);
  } else if ('%' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenPercent);
  } else if ('~' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenBitNot);
  } else if ('?' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenConditional);
  } else if (':' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenColon);
  } else if (',' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenComma);
  } else if (';' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenSemicolon);
  } else if ('{' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenLBrace);
  } else if ('}' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenRBrace);
  } else if ('(' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenLParen);
  } else if (')' == *(*p)) {
    return AllocToken(src, (*p)++, 1, kTokenRParen);
  }
  Error("Unexpected char %c", *(*p));
}

struct Token *CreateToken(const char *input) {
  return CreateNextToken(&input, input);
}

void Tokenize(const char *input) {
  const char *p = input;
  struct Token *t;
  while ((t = CreateNextToken(&p, input))) {
    AddToken(t);
  }
}

void PrintToken(struct Token *t) {
  fprintf(stderr, "(Token %.*s type=%s)", t->length, t->begin,
          GetTokenTypeName(t->type));
}

void PrintTokenBrief(struct Token *t) {
  assert(t);
  fprintf(stderr, "(%s", GetTokenTypeName(t->type));
  if (t->type == kTokenDecimalNumber || t->type == kTokenOctalNumber ||
      t->type == kTokenIdent) {
    fprintf(stderr, ":%.*s", t->length, t->begin);
  }
  fputc(')', stderr);
}

void PrintTokenStr(struct Token *t) {
  fprintf(stderr, "%.*s", t->length, t->begin);
}

void PrintTokens() {
  for (int i = 0; i < tokens_used; i++) {
    struct Token *t = tokens[i];
    PrintToken(t);
    fputc('\n', stderr);
  }
}

_Noreturn void ErrorWithToken(struct Token *t, const char *fmt, ...) {
  assert(t);
  const char *line_begin = t->begin;
  while (t->src_str < line_begin) {
    if (line_begin[-1] == '\n') break;
    line_begin--;
  }

  for (const char *p = line_begin; *p && *p != '\n'; p++) {
    fputc(*p <= ' ' ? ' ' : *p, stderr);
  }
  fputc('\n', stderr);
  const char *p;
  for (p = line_begin; p < t->begin; p++) {
    fputc(' ', stderr);
  }
  for (int i = 0; i < t->length; i++) {
    fputc('^', stderr);
    p++;
  }
  for (; *p && *p != '\n'; p++) {
    fputc(' ', stderr);
  }
  fputc('\n', stderr);

  fflush(stdout);
  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

int token_stream_index;
struct Token *ConsumeToken(enum TokenTypes type) {
  if (token_stream_index < tokens_used &&
      tokens[token_stream_index]->type == type) {
    return tokens[token_stream_index++];
  }
  return NULL;
}

struct Token *ExpectToken(enum TokenTypes type) {
  if (token_stream_index < tokens_used &&
      tokens[token_stream_index]->type == type) {
    return tokens[token_stream_index++];
  }
  ErrorWithToken(tokens[token_stream_index], "Expected %s here",
                 GetTokenTypeName(type));
}

struct Token *NextToken() {
  if (token_stream_index >= tokens_used) return NULL;
  return tokens[token_stream_index++];
}

enum ASTType {
  kASTTypeNone,
  kASTTypePrimaryExpr,
  kASTTypeUnaryPrefixOp,
  kASTTypeBinOp,
  kASTTypeCondExpr,
  kASTTypeList,
  kASTTypeExprStmt,
  kASTTypeJumpStmt,
  kASTTypeIdent,
  kASTTypeDecl,
  kASTTypeKeyValue,
  kASTTypeLocalVar,
  kASTTypeBaseType,
  kASTTypeLValueOf,
  kASTTypePointerOf,
};

struct ASTNode {
  enum ASTType type;
  int reg;
  struct ASTNode *expr_type;
  struct Token *op;
  struct ASTNode *left;
  struct ASTNode *right;
  struct ASTNode *cond;
  // for list
  int capacity;
  int size;
  struct ASTNode **nodes;
  // for key value
  const char *key;
  struct ASTNode *value;
  // for local var
  int byte_offset;
};

struct ASTNode *AllocASTNode(enum ASTType type) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->type = type;
  return node;
}

struct ASTNode *AllocAndInitASTNodeBinOp(struct Token *t, struct ASTNode *left,
                                         struct ASTNode *right) {
  if (!right) ErrorWithToken(t, "Expected expression after binary operator");
  struct ASTNode *op = AllocASTNode(kASTTypeBinOp);
  op->op = t;
  op->left = left;
  op->right = right;
  return op;
}

struct ASTNode *AllocAndInitASTNodeUnaryPrefixOp(struct Token *t,
                                                 struct ASTNode *right) {
  if (!right) ErrorWithToken(t, "Expected expression after prefix operator");
  struct ASTNode *op = AllocASTNode(kASTTypeUnaryPrefixOp);
  op->op = t;
  op->right = right;
  return op;
}

struct ASTNode *AllocAndInitASTNodeExprStmt(struct Token *t,
                                            struct ASTNode *left) {
  struct ASTNode *op = AllocASTNode(kASTTypeExprStmt);
  op->op = t;
  op->left = left;
  return op;
}

struct ASTNode *AllocAndInitASTNodeIdent(struct Token *ident) {
  struct ASTNode *n = AllocASTNode(kASTTypeIdent);
  n->op = ident;
  return n;
}

struct ASTNode *AllocAndInitASTNodeKeyValue(const char *key,
                                            struct ASTNode *value) {
  struct ASTNode *n = AllocASTNode(kASTTypeKeyValue);
  n->key = key;
  n->value = value;
  return n;
}

struct ASTNode *AllocAndInitASTNodeLocalVar(int byte_offset,
                                            struct ASTNode *var_type) {
  struct ASTNode *n = AllocASTNode(kASTTypeLocalVar);
  n->byte_offset = byte_offset;
  n->expr_type = var_type;
  return n;
}

struct ASTNode *AllocAndInitBaseType(struct Token *t) {
  struct ASTNode *n = AllocASTNode(kASTTypeBaseType);
  n->op = t;
  return n;
}

struct ASTNode *AllocAndInitLValueOf(struct ASTNode *type) {
  struct ASTNode *n = AllocASTNode(kASTTypeLValueOf);
  n->right = type;
  return n;
}

struct ASTNode *AllocAndInitPointerOf(struct ASTNode *type) {
  struct ASTNode *n = AllocASTNode(kASTTypePointerOf);
  n->right = type;
  return n;
}

int IsSameType(struct ASTNode *a, struct ASTNode *b) {
  assert(a && b);
  if (a->type != b->type) return 0;
  if (a->type == kASTTypeBaseType) {
    assert(a->op && b->op);
    return a->op->type == b->op->type;
  } else if (a->type == kASTTypeLValueOf || a->type == kASTTypePointerOf) {
    return IsSameType(a->right, b->right);
  }
  Error("IsSameType: Comparing non-type nodes");
}

struct ASTNode *GetRValueType(struct ASTNode *t) {
  if (!t) return NULL;
  if (t->type != kASTTypeLValueOf) return t;
  return t->right;
}

int IsAssignable(struct ASTNode *dst, struct ASTNode *src) {
  assert(dst && src);
  if (dst->type != kASTTypeLValueOf) return 0;
  return IsSameType(GetRValueType(dst), src);
}

int GetSizeOfType(struct ASTNode *t) {
  t = GetRValueType(t);
  assert(t);
  if (t->type == kASTTypeBaseType) {
    assert(t->op);
    if (t->op->type == kTokenKwInt) return 8;
  } else if (t->type == kASTTypePointerOf) {
    return 8;
  }
  assert(false);
}

void TestType() {
  fprintf(stderr, "Testing Type...");

  struct ASTNode *int_type = AllocAndInitBaseType(CreateToken("int"));
  struct ASTNode *another_int_type = AllocAndInitBaseType(CreateToken("int"));
  struct ASTNode *lvalue_int_type = AllocAndInitLValueOf(int_type);
  struct ASTNode *pointer_of_int_type = AllocAndInitPointerOf(int_type);
  struct ASTNode *another_pointer_of_int_type =
      AllocAndInitPointerOf(another_int_type);

  assert(IsSameType(int_type, int_type));
  assert(IsSameType(int_type, another_int_type));
  assert(!IsSameType(int_type, lvalue_int_type));
  assert(IsSameType(lvalue_int_type, lvalue_int_type));
  assert(!IsSameType(int_type, pointer_of_int_type));
  assert(IsSameType(pointer_of_int_type, another_pointer_of_int_type));

  assert(GetSizeOfType(int_type) == 8);
  assert(GetSizeOfType(pointer_of_int_type) == 8);

  fprintf(stderr, "PASS\n");
  exit(EXIT_SUCCESS);
}

struct ASTNode *AllocList() {
  return AllocASTNode(kASTTypeList);
}

void ExpandListSizeIfNeeded(struct ASTNode *list) {
  if (list->size < list->capacity) return;
  list->capacity = (list->capacity + 1) * 2;
  list->nodes = realloc(list->nodes, sizeof(struct ASTNode *) * list->capacity);
  assert(list->nodes);
  assert(list->size < list->capacity);
}

void PushToList(struct ASTNode *list, struct ASTNode *node) {
  ExpandListSizeIfNeeded(list);
  list->nodes[list->size++] = node;
}

void PushKeyValueToList(struct ASTNode *list, const char *key,
                        struct ASTNode *value) {
  ExpandListSizeIfNeeded(list);
  list->nodes[list->size++] = AllocAndInitASTNodeKeyValue(key, value);
}

void AddLocalVar(struct ASTNode *list, const char *key,
                 struct ASTNode *var_type) {
  struct ASTNode *local_var = AllocAndInitASTNodeLocalVar(8, var_type);
  PushKeyValueToList(list, key, local_var);
}

int GetSizeOfList(struct ASTNode *list) {
  assert(list && list->type == kASTTypeList);
  return list->size;
}

struct ASTNode *GetNodeAt(struct ASTNode *list, int index) {
  assert(list && list->type == kASTTypeList);
  assert(0 <= index && index < list->size);
  return list->nodes[index];
}

struct ASTNode *GetNodeByTokenKey(struct ASTNode *list, struct Token *key) {
  assert(list && list->type == kASTTypeList);
  for (int i = 0; i < list->size; i++) {
    struct ASTNode *n = list->nodes[i];
    if (n->type != kASTTypeKeyValue) continue;
    if (IsEqualTokenWithCStr(key, n->key)) return n->value;
  }
  return NULL;
}

struct ASTNode *GetNodeByKey(struct ASTNode *list, const char *key) {
  assert(list && list->type == kASTTypeList);
  for (int i = 0; i < list->size; i++) {
    struct ASTNode *n = list->nodes[i];
    if (n->type != kASTTypeKeyValue) continue;
    if (strcmp(n->key, key) == 0) return n->value;
  }
  return NULL;
}

void TestList() {
  fprintf(stderr, "Testing List...");

  struct ASTNode *list = AllocList();
  struct ASTNode *item1 = AllocASTNode(kASTTypeNone);
  struct ASTNode *item2 = AllocASTNode(kASTTypeNone);
  assert(list);

  PushToList(list, item1);
  assert(GetSizeOfList(list) == 1);
  PushToList(list, item2);
  assert(GetSizeOfList(list) == 2);

  assert(GetNodeAt(list, 0) == item1);
  assert(GetNodeAt(list, 1) == item2);

  int base_capacity = list->capacity;
  while (GetSizeOfList(list) <= base_capacity) {
    PushToList(list, item1);
  }
  assert(list->capacity > base_capacity);
  assert(GetNodeAt(list, 0) == item1);
  assert(GetNodeAt(list, 1) == item2);
  assert(GetNodeAt(list, GetSizeOfList(list) - 1) == item1);

  PushKeyValueToList(list, "item1", item1);
  PushKeyValueToList(list, "item2", item2);
  assert(GetNodeByKey(list, "item1") == item1);
  assert(GetNodeByKey(list, "item2") == item2);
  assert(GetNodeByKey(list, "not_existed") == NULL);

  fprintf(stderr, "PASS\n");
  exit(EXIT_SUCCESS);
}

void PrintPadding(int depth) {
  for (int i = 0; i < depth; i++) {
    fputc(' ', stderr);
  }
}

void PrintASTNodeSub(struct ASTNode *n, int depth) {
  if (n->type == kASTTypeList) {
    fprintf(stderr, "[\n");
    for (int i = 0; i < GetSizeOfList(n); i++) {
      if (i) fprintf(stderr, ",\n");
      PrintPadding(depth + 1);
      PrintASTNodeSub(GetNodeAt(n, i), depth + 1);
    }
    fprintf(stderr, "\n]");
    return;
  } else if (n->type == kASTTypeBaseType) {
    PrintTokenStr(n->op);
    return;
  } else if (n->type == kASTTypeLValueOf) {
    fprintf(stderr, "lvalue<");
    PrintASTNodeSub(n->right, depth + 1);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kASTTypePointerOf) {
    fprintf(stderr, "*");
    PrintASTNodeSub(n->right, depth + 1);
    return;
  }
  fprintf(stderr, "(");
  PrintTokenBrief(n->op);
  if (n->expr_type) {
    fprintf(stderr, ":");
    PrintASTNodeSub(n->expr_type, depth + 1);
  }
  if (n->reg) fprintf(stderr, " reg: %d", n->reg);
  if (n->left) {
    fprintf(stderr, " left: ");
    PrintASTNodeSub(n->left, depth + 1);
  }
  if (n->right) {
    fprintf(stderr, " right: ");
    PrintASTNodeSub(n->right, depth + 1);
  }
  fprintf(stderr, ")");
}

void PrintASTNode(struct ASTNode *n) { PrintASTNodeSub(n, 0); }

struct ASTNode *ParseCastExpr();

struct ASTNode *ParseExpr(void);
struct ASTNode *ParsePrimaryExpr() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenDecimalNumber)) ||
      (t = ConsumeToken(kTokenOctalNumber)) ||
      (t = ConsumeToken(kTokenIdent))) {
    struct ASTNode *op = AllocASTNode(kASTTypePrimaryExpr);
    op->op = t;
    return op;
  }
  if ((t = ConsumeToken(kTokenLParen))) {
    struct ASTNode *op = AllocASTNode(kASTTypePrimaryExpr);
    op->op = t;
    op->right = ParseExpr();
    if (!op->right) ErrorWithToken(t, "Expected expr after this token");
    ExpectToken(kTokenRParen);
    return op;
  }
  return NULL;
}

struct ASTNode *ParseUnaryExpr() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenPlus)) || (t = ConsumeToken(kTokenMinus)) ||
      (t = ConsumeToken(kTokenBitNot)) || (t = ConsumeToken(kTokenBoolNot))) {
    return AllocAndInitASTNodeUnaryPrefixOp(t, ParseCastExpr());
  } else if ((t = ConsumeToken(kTokenKwSizeof))) {
    return AllocAndInitASTNodeUnaryPrefixOp(t, ParseUnaryExpr());
  }
  return ParsePrimaryExpr();
}

struct ASTNode *ParseCastExpr() {
  return ParseUnaryExpr();
}

struct ASTNode *ParseMulExpr() {
  struct ASTNode *op = ParseCastExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenStar)) || (t = ConsumeToken(kTokenSlash)) ||
         (t = ConsumeToken(kTokenPercent))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseCastExpr());
  }
  return op;
}

struct ASTNode *ParseAddExpr() {
  struct ASTNode *op = ParseMulExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenPlus)) || (t = ConsumeToken(kTokenMinus))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseMulExpr());
  }
  return op;
}

struct ASTNode *ParseShiftExpr() {
  struct ASTNode *op = ParseAddExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenShiftLeft)) ||
         (t = ConsumeToken(kTokenShiftRight))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseAddExpr());
  }
  return op;
}

struct ASTNode *ParseRelExpr() {
  struct ASTNode *op = ParseShiftExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenLessThan)) ||
         (t = ConsumeToken(kTokenGreaterThan)) ||
         (t = ConsumeToken(kTokenLessThanEq)) ||
         (t = ConsumeToken(kTokenGreaterThanEq))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseShiftExpr());
  }
  return op;
}

struct ASTNode *ParseEqExpr() {
  struct ASTNode *op = ParseRelExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenEq)) || (t = ConsumeToken(kTokenNotEq))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseRelExpr());
  }
  return op;
}

struct ASTNode *ParseAndExpr() {
  struct ASTNode *op = ParseEqExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenBitAnd))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseEqExpr());
  }
  return op;
}

struct ASTNode *ParseXorExpr() {
  struct ASTNode *op = ParseAndExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenBitXor))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseAndExpr());
  }
  return op;
}

struct ASTNode *ParseOrExpr() {
  struct ASTNode *op = ParseXorExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenBitOr))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseXorExpr());
  }
  return op;
}

struct ASTNode *ParseBoolAndExpr() {
  struct ASTNode *op = ParseOrExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenBoolAnd))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseOrExpr());
  }
  return op;
}

struct ASTNode *ParseBoolOrExpr() {
  struct ASTNode *op = ParseBoolAndExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenBoolOr))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseBoolAndExpr());
  }
  return op;
}

struct ASTNode *ParseConditionalExpr() {
  struct ASTNode *expr = ParseBoolOrExpr();
  if (!expr) return NULL;
  struct Token *t;
  if ((t = ConsumeToken(kTokenConditional))) {
    struct ASTNode *op = AllocASTNode(kASTTypeCondExpr);
    op->op = t;
    op->cond = expr;
    op->left = ParseConditionalExpr();
    if (!op->left)
      ErrorWithToken(t, "Expected true-expr for this conditional expr");
    ExpectToken(kTokenColon);
    op->right = ParseConditionalExpr();
    if (!op->right)
      ErrorWithToken(t, "Expected false-expr for this conditional expr");
    return op;
  }
  return expr;
}

struct ASTNode *ParseAssignExpr() {
  struct ASTNode *left = ParseConditionalExpr();
  if (!left) return NULL;
  // TODO: Add kASTTypeUnaryPostfixOp after its implementation
  if (left->type != kASTTypeUnaryPrefixOp && left->type != kASTTypePrimaryExpr)
    return left;
  int last_token_stream_index = token_stream_index;
  struct Token *t;
  if ((t = ConsumeToken(kTokenAssign))) {
    struct ASTNode *right = ParseAssignExpr();
    if (!right) {
      token_stream_index = last_token_stream_index;
      return left;
    }
    return AllocAndInitASTNodeBinOp(t, left, right);
  }
  return left;
}

struct ASTNode *ParseExpr() {
  struct ASTNode *op = ParseAssignExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenComma))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseAssignExpr());
  }
  return op;
}

struct ASTNode *ParseExprStmt() {
  struct ASTNode *expr = ParseExpr();
  struct Token *t;
  if ((t = ConsumeToken(kTokenSemicolon))) {
    return AllocAndInitASTNodeExprStmt(t, expr);
  } else if (expr) {
    ExpectToken(kTokenSemicolon);
  }
  return NULL;
}

struct ASTNode *ParseJumpStmt() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenKwReturn))) {
    struct ASTNode *expr = ParseExpr();
    ExpectToken(kTokenSemicolon);
    struct ASTNode *stmt = AllocASTNode(kASTTypeJumpStmt);
    stmt->op = t;
    stmt->right = expr;
    return stmt;
  }
  return NULL;
}

struct ASTNode *ParseStmt() {
  struct ASTNode *stmt;
  if ((stmt = ParseExprStmt()) || (stmt = ParseJumpStmt())) return stmt;
  return NULL;
}

struct ASTNode *ParseDecl() {
  struct Token *t;
  if (!(t = ConsumeToken(kTokenKwInt))) return NULL;
  struct ASTNode *n = AllocASTNode(kASTTypeDecl);
  n->op = t;
  struct ASTNode *type = AllocAndInitBaseType(t);
  while ((t = ConsumeToken(kTokenStar))) {
    type = AllocAndInitPointerOf(type);
  }
  n->right = AllocAndInitASTNodeIdent(ExpectToken(kTokenIdent));
  n->left = type;
  ExpectToken(kTokenSemicolon);
  return n;
}

struct ASTNode *ParseCompStmt() {
  struct Token *t;
  if (!(t = ConsumeToken(kTokenLBrace))) return NULL;
  struct ASTNode *list = AllocList();
  list->op = t;
  struct ASTNode *stmt;
  while ((stmt = ParseDecl()) || (stmt = ParseStmt())) {
    PushToList(list, stmt);
  }
  ExpectToken(kTokenRBrace);
  return list;
}

struct ASTNode *Parse() {
  struct ASTNode *ast = ParseCompStmt();
  struct Token *t;
  if (!(t = NextToken())) return ast;
  ErrorWithToken(t, "Unexpected token");
}

#define NUM_OF_SCRATCH_REGS 4
const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1] = {NULL, "rdi", "rsi", "r8",
                                                     "r9"};
const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1] = {NULL, "dil", "sil", "r8b",
                                                    "r9b"};

int reg_used_table[NUM_OF_SCRATCH_REGS];

int AllocReg() {
  for (int i = 1; i <= NUM_OF_SCRATCH_REGS; i++) {
    if (!reg_used_table[i]) {
      reg_used_table[i] = 1;
      return i;
    }
  }
  Error("No more regs");
}
void FreeReg(int reg) {
  assert(1 <= reg && reg <= NUM_OF_SCRATCH_REGS);
  reg_used_table[reg] = 0;
}

int label_number;
int GetLabelNumber() { return ++label_number; }

void EmitConvertToBool(int dst, int src) {
  // This code also sets zero flag as boolean value
  printf("cmp %s, 0\n", reg_names_64[src]);
  printf("setnz %s\n", reg_names_8[src]);
  printf("movzx %s, %s\n", reg_names_64[dst], reg_names_8[src]);
}

void EmitCompareIntegers(int dst, int left, int right, const char *cc) {
  printf("cmp %s, %s\n", reg_names_64[left], reg_names_64[right]);
  printf("set%s %s\n", cc, reg_names_8[dst]);
  printf("movzx %s, %s\n", reg_names_64[dst], reg_names_8[dst]);
}

void GenerateRValue(struct ASTNode *node);

struct ASTNode *var_context;
void Analyze(struct ASTNode *node) {
  assert(node && node->op);
  if (node->type == kASTTypePrimaryExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber) {
      node->reg = AllocReg();
      node->expr_type = AllocAndInitBaseType(CreateToken("int"));
      return;
    } else if (node->op->type == kTokenLParen) {
      Analyze(node->right);
      node->reg = node->right->reg;
      node->expr_type = node->right->expr_type;
      return;
    } else if (node->op->type == kTokenIdent) {
      struct ASTNode *var_info = GetNodeByTokenKey(var_context, node->op);
      if (!var_info || var_info->type != kASTTypeLocalVar)
        ErrorWithToken(node->op, "Unknown identifier");
      node->byte_offset = var_info->byte_offset;
      node->reg = AllocReg();
      node->expr_type = AllocAndInitLValueOf(var_info->expr_type);
      return;
    }
  } else if (node->type == kASTTypeExprStmt) {
    if (!node->left) return;
    Analyze(node->left);
    if (node->left->reg) FreeReg(node->left->reg);
    return;
  } else if (node->type == kASTTypeList) {
    var_context = AllocList();
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Analyze(GetNodeAt(node, i));
    }
    return;
  } else if (node->type == kASTTypeDecl) {
    AddLocalVar(var_context, CreateTokenStr(node->right->op), node->left);
    return;
  } else if (node->type == kASTTypeJumpStmt) {
    if (node->op->type == kTokenKwReturn) {
      Analyze(node->right);
      FreeReg(node->right->reg);
      return;
    }
  } else if (node->type == kASTTypeUnaryPrefixOp) {
    Analyze(node->right);
    if (node->op->type == kTokenKwSizeof) {
      node->reg = AllocReg();
      node->expr_type = AllocAndInitBaseType(CreateToken("int"));
      return;
    }
    node->reg = node->right->reg;
    node->expr_type = GetRValueType(node->right->expr_type);
    return;
  } else if (node->type == kASTTypeCondExpr) {
    Analyze(node->cond);
    Analyze(node->left);
    Analyze(node->right);
    FreeReg(node->left->reg);
    FreeReg(node->right->reg);
    assert(IsSameType(GetRValueType(node->left->expr_type),
                      GetRValueType(node->right->expr_type)));
    node->reg = node->cond->reg;
    node->expr_type = GetRValueType(node->right->expr_type);
    return;
  } else if (node->type == kASTTypeBinOp) {
    Analyze(node->left);
    Analyze(node->right);
    if (node->op->type == kTokenAssign || node->op->type == kTokenComma) {
      FreeReg(node->left->reg);
      node->reg = node->right->reg;
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    }
    FreeReg(node->right->reg);
    node->reg = node->left->reg;
    node->expr_type = GetRValueType(node->left->expr_type);
    return;
  }
  ErrorWithToken(node->op, "Analyze: Not implemented");
}

void Generate(struct ASTNode *node) {
  assert(node && node->op);
  if (node->type == kASTTypePrimaryExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber) {
      printf("mov %s, %ld\n", reg_names_64[node->reg],
             strtol(node->op->begin, NULL, 0));
      return;
    } else if (node->op->type == kTokenLParen) {
      Generate(node->right);
      return;
    } else if (node->op->type == kTokenIdent) {
      printf("lea %s, [rbp - %d]\n", reg_names_64[node->reg],
             node->byte_offset);
      return;
    }
  } else if (node->type == kASTTypeExprStmt) {
    if (node->left) Generate(node->left);
    return;
  } else if (node->type == kASTTypeList) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Generate(GetNodeAt(node, i));
    }
    return;
  } else if (node->type == kASTTypeDecl) {
    return;
  } else if (node->type == kASTTypeJumpStmt) {
    if (node->op->type == kTokenKwReturn) {
      GenerateRValue(node->right);
      printf("mov rax, %s\n", reg_names_64[node->right->reg]);
      printf("mov rsp, rbp\n");
      printf("pop rbp\n");
      printf("ret\n");
      return;
    }
    ErrorWithToken(node->op, "Generate: Not implemented jump stmt");
  } else if (node->type == kASTTypeUnaryPrefixOp) {
    if (node->op->type == kTokenKwSizeof) {
      printf("mov %s, %d\n", reg_names_64[node->reg],
             GetSizeOfType(node->right->expr_type));
      return;
    }
    GenerateRValue(node->right);
    if (node->op->type == kTokenPlus) {
      return;
    }
    if (node->op->type == kTokenMinus) {
      printf("neg %s\n", reg_names_64[node->reg]);
      return;
    }
    if (node->op->type == kTokenBitNot) {
      printf("not %s\n", reg_names_64[node->reg]);
      return;
    }
    if (node->op->type == kTokenBoolNot) {
      EmitConvertToBool(node->reg, node->reg);
      printf("setz %s\n", reg_names_8[node->reg]);
      return;
    }
    ErrorWithToken(node->op, "Generate: Not implemented unary prefix op");
  } else if (node->type == kASTTypeCondExpr) {
    GenerateRValue(node->cond);
    int false_label = GetLabelNumber();
    int end_label = GetLabelNumber();
    EmitConvertToBool(node->cond->reg, node->cond->reg);
    printf("jz L%d\n", false_label);
    GenerateRValue(node->left);
    printf("mov %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->left->reg]);
    printf("jmp L%d\n", end_label);
    printf("L%d:\n", false_label);
    GenerateRValue(node->right);
    printf("mov %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("L%d:\n", end_label);
    return;
  } else if (node->type == kASTTypeBinOp) {
    if (node->op->type == kTokenBoolAnd) {
      GenerateRValue(node->left);
      int skip_label = GetLabelNumber();
      EmitConvertToBool(node->reg, node->left->reg);
      printf("jz L%d\n", skip_label);
      GenerateRValue(node->right);
      EmitConvertToBool(node->reg, node->right->reg);
      printf("L%d:\n", skip_label);
      return;
    } else if (node->op->type == kTokenBoolOr) {
      GenerateRValue(node->left);
      int skip_label = GetLabelNumber();
      EmitConvertToBool(node->reg, node->left->reg);
      printf("jnz L%d\n", skip_label);
      GenerateRValue(node->right);
      EmitConvertToBool(node->reg, node->right->reg);
      printf("L%d:\n", skip_label);
      return;
    } else if (node->op->type == kTokenComma) {
      Generate(node->left);
      GenerateRValue(node->right);
      return;
    } else if (node->op->type == kTokenAssign) {
      Generate(node->left);
      GenerateRValue(node->right);
      printf("mov [%s], %s\n", reg_names_64[node->left->reg],
             reg_names_64[node->right->reg]);
      return;
    }
    GenerateRValue(node->left);
    GenerateRValue(node->right);
    if (node->op->type == kTokenPlus) {
      printf("add %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->right->reg]);
      return;
    } else if (node->op->type == kTokenMinus) {
      printf("sub %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->right->reg]);
      return;
    } else if (node->op->type == kTokenStar) {
      // rdx:rax <- rax * r/m
      printf("xor rdx, rdx\n");
      printf("mov rax, %s\n", reg_names_64[node->reg]);
      printf("imul %s\n", reg_names_64[node->right->reg]);
      printf("mov %s, rax\n", reg_names_64[node->reg]);
      return;
    } else if (node->op->type == kTokenSlash) {
      // rax <- rdx:rax / r/m
      printf("xor rdx, rdx\n");
      printf("mov rax, %s\n", reg_names_64[node->reg]);
      printf("idiv %s\n", reg_names_64[node->right->reg]);
      printf("mov %s, rax\n", reg_names_64[node->reg]);
      return;
    } else if (node->op->type == kTokenPercent) {
      // rdx <- rdx:rax / r/m
      printf("xor rdx, rdx\n");
      printf("mov rax, %s\n", reg_names_64[node->reg]);
      printf("idiv %s\n", reg_names_64[node->right->reg]);
      printf("mov %s, rdx\n", reg_names_64[node->reg]);
      return;
    } else if (node->op->type == kTokenShiftLeft) {
      // r/m <<= CL
      printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
      printf("sal %s, cl\n", reg_names_64[node->reg]);
      return;
    } else if (node->op->type == kTokenShiftRight) {
      // r/m >>= CL
      printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
      printf("sar %s, cl\n", reg_names_64[node->reg]);
      return;
    } else if (node->op->type == kTokenLessThan) {
      EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "l");
      return;
    } else if (node->op->type == kTokenGreaterThan) {
      EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "g");
      return;
    } else if (node->op->type == kTokenLessThanEq) {
      EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "le");
      return;
    } else if (node->op->type == kTokenGreaterThanEq) {
      EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "ge");
      return;
    } else if (node->op->type == kTokenEq) {
      EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "e");
      return;
    } else if (node->op->type == kTokenNotEq) {
      EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "ne");
      return;
    } else if (node->op->type == kTokenBitAnd) {
      printf("and %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->right->reg]);
      return;
    } else if (node->op->type == kTokenBitXor) {
      printf("xor %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->right->reg]);
      return;
    } else if (node->op->type == kTokenBitOr) {
      printf("or %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->right->reg]);
      return;
    }
  }
  ErrorWithToken(node->op, "Generate: Not implemented");
}

void GenerateRValue(struct ASTNode *node) {
  Generate(node);
  if (!node->expr_type || node->expr_type->type != kASTTypeLValueOf) return;
  printf("mov %s, [%s]\n", reg_names_64[node->reg], reg_names_64[node->reg]);
}

int main(int argc, char *argv[]) {
  struct CompilerArgs args;
  ParseCompilerArgs(&args, argc, argv);

  InitTokenTypeNames();

  fprintf(stderr, "input:\n%s\n", args.input);
  Tokenize(args.input);
  PrintTokens();

  struct ASTNode *ast = Parse();
  PrintASTNode(ast);
  fputc('\n', stderr);

  Analyze(ast);
  PrintASTNode(ast);
  fputc('\n', stderr);

  printf(".intel_syntax noprefix\n");
  printf(".text\n");
  printf(".global %smain\n", symbol_prefix);
  printf("%smain:\n", symbol_prefix);
  printf("push rbp\n");
  printf("mov rbp, rsp\n");
  Generate(ast);
  printf("mov rsp, rbp\n");
  printf("pop rbp\n");
  printf("ret\n");
}
