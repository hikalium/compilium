#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum OSType {
  kOSDarwin,
  kOSLinux,
};

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
  kNumOfTokenTypeNames
};

struct Token {
  const char *begin;
  int length;
  enum TokenTypes type;
  const char *src_str;
};

#define NUM_OF_TOKENS 32
struct Token tokens[NUM_OF_TOKENS];
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

void __assert(const char *expr_str, const char *file, int line) {
  Error("Assertion failed: %s at %s:%d\n", expr_str, file, line);
}

const char *token_type_names[kNumOfTokenTypeNames];

void TestASTList(void);
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
    } else if (strcmp(argv[i], "--run-unittest=ASTList") == 0) {
      TestASTList();
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
}

const char *GetTokenTypeName(enum TokenTypes type) {
  assert(0 <= type && type < kNumOfTokenTypeNames);
  assert(token_type_names[type]);
  return token_type_names[type];
}

void AddToken(const char *src_str, const char *begin, int length,
              enum TokenTypes type) {
  assert(tokens_used < NUM_OF_TOKENS);
  tokens[tokens_used].begin = begin;
  tokens[tokens_used].length = length;
  tokens[tokens_used].type = type;
  tokens[tokens_used].src_str = src_str;
  tokens_used++;
}

void Tokenize(const char *src) {
  const char *s = src;
  while (*s) {
    if (*s <= ' ') {
      s++;
      continue;
    }
    if ('1' <= *s && *s <= '9') {
      int length = 0;
      while ('0' <= s[length] && s[length] <= '9') {
        length++;
      }
      AddToken(src, s, length, kTokenDecimalNumber);
      s += length;
      continue;
    }
    if ('0' == *s) {
      int length = 0;
      while ('0' <= s[length] && s[length] <= '7') {
        length++;
      }
      AddToken(src, s, length, kTokenOctalNumber);
      s += length;
      continue;
    }
    if (('A' <= *s && *s <= 'Z') || ('a' <= *s && *s <= 'z') || *s == '_') {
      int length = 0;
      while (('A' <= s[length] && s[length] <= 'Z') ||
             ('a' <= s[length] && s[length] <= 'z') || s[length] == '_' ||
             ('0' <= s[length] && s[length] <= '9')) {
        length++;
      }
      enum TokenTypes type = kTokenIdent;
      if (strncmp(s, "return", length) == 0) type = kTokenKwReturn;
      AddToken(src, s, length, type);
      s += length;
      continue;
    }
    if ('+' == *s) {
      AddToken(src, s++, 1, kTokenPlus);
      continue;
    }
    if ('-' == *s) {
      AddToken(src, s++, 1, kTokenMinus);
      continue;
    }
    if ('*' == *s) {
      AddToken(src, s++, 1, kTokenStar);
      continue;
    }
    if ('/' == *s) {
      AddToken(src, s++, 1, kTokenSlash);
      continue;
    }
    if ('%' == *s) {
      AddToken(src, s++, 1, kTokenPercent);
      continue;
    }
    if ('&' == *s) {
      if (s[1] == '&') {
        AddToken(src, s, 2, kTokenBoolAnd);
        s += 2;
        continue;
      }
      AddToken(src, s++, 1, kTokenBitAnd);
      continue;
    }
    if ('^' == *s) {
      AddToken(src, s++, 1, kTokenBitXor);
      continue;
    }
    if ('|' == *s) {
      if (s[1] == '|') {
        AddToken(src, s, 2, kTokenBoolOr);
        s += 2;
        continue;
      }
      AddToken(src, s++, 1, kTokenBitOr);
      continue;
    }
    if ('<' == *s) {
      if (s[1] == '<') {
        AddToken(src, s, 2, kTokenShiftLeft);
        s += 2;
        continue;
      }
      if (s[1] == '=') {
        AddToken(src, s, 2, kTokenLessThanEq);
        s += 2;
        continue;
      }
      AddToken(src, s++, 1, kTokenLessThan);
      continue;
    }
    if ('>' == *s) {
      if (s[1] == '>') {
        AddToken(src, s, 2, kTokenShiftRight);
        s += 2;
        continue;
      }
      if (s[1] == '=') {
        AddToken(src, s, 2, kTokenGreaterThanEq);
        s += 2;
        continue;
      }
      AddToken(src, s++, 1, kTokenGreaterThan);
      continue;
    }
    if ('=' == *s) {
      if (s[1] == '=') {
        AddToken(src, s, 2, kTokenEq);
        s += 2;
        continue;
      }
    }
    if ('!' == *s) {
      if (s[1] == '=') {
        AddToken(src, s, 2, kTokenNotEq);
        s += 2;
        continue;
      }
      AddToken(src, s++, 1, kTokenBoolNot);
      continue;
    }
    if ('~' == *s) {
      AddToken(src, s++, 1, kTokenBitNot);
      continue;
    }
    if ('?' == *s) {
      AddToken(src, s++, 1, kTokenConditional);
      continue;
    }
    if (':' == *s) {
      AddToken(src, s++, 1, kTokenColon);
      continue;
    }
    if (',' == *s) {
      AddToken(src, s++, 1, kTokenComma);
      continue;
    }
    if (';' == *s) {
      AddToken(src, s++, 1, kTokenSemicolon);
      continue;
    }
    Error("Unexpected char %c", *s);
  }
}

void PrintToken(struct Token *t) {
  fprintf(stderr, "(Token %.*s type=%s)", t->length, t->begin,
          GetTokenTypeName(t->type));
}

void PrintTokens() {
  for (int i = 0; i < tokens_used; i++) {
    struct Token *t = &tokens[i];
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
      tokens[token_stream_index].type == type) {
    return &tokens[token_stream_index++];
  }
  return NULL;
}

struct Token *ExpectToken(enum TokenTypes type) {
  if (token_stream_index < tokens_used &&
      tokens[token_stream_index].type == type) {
    return &tokens[token_stream_index++];
  }
  ErrorWithToken(&tokens[token_stream_index], "Expected %s here",
                 GetTokenTypeName(type));
}

struct Token *NextToken() {
  if (token_stream_index < tokens_used) {
    return &tokens[token_stream_index++];
  }
  return NULL;
}

enum ASTType {
  kASTTypeNone,
  kASTTypePrimaryExpr,
  kASTTypeBinOp,
  kASTTypeUnaryPrefixOp,
  kASTTypeCondExpr,
  kASTTypeList,
  kASTTypeExprStmt,
};

struct ASTNode {
  enum ASTType type;
  struct Token *op;
  int reg;
  struct ASTNode *left;
  struct ASTNode *right;
  struct ASTNode *cond;
  // for list
  int capacity;
  int size;
  struct ASTNode **nodes;
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

struct ASTNode *AllocASTList() {
  return AllocASTNode(kASTTypeList);
}

void PushToASTList(struct ASTNode *list, struct ASTNode *node) {
  if (list->size >= list->capacity) {
    list->capacity = (list->capacity + 1) * 2;
    list->nodes =
        realloc(list->nodes, sizeof(struct ASTNode *) * list->capacity);
    assert(list->nodes);
  }
  assert(list->size < list->capacity);
  list->nodes[list->size++] = node;
}

int GetSizeOfASTList(struct ASTNode *list) {
  assert(list && list->type == kASTTypeList);
  return list->size;
}

struct ASTNode *GetNodeAt(struct ASTNode *list, int index) {
  assert(list && list->type == kASTTypeList);
  assert(0 <= index && index < list->size);
  return list->nodes[index];
}

void TestASTList() {
  fprintf(stderr, "Testing ASTList...");

  struct ASTNode *list = AllocASTList();
  struct ASTNode *item1 = AllocASTNode(kASTTypeNone);
  struct ASTNode *item2 = AllocASTNode(kASTTypeNone);
  assert(list);

  PushToASTList(list, item1);
  assert(GetSizeOfASTList(list) == 1);
  PushToASTList(list, item2);
  assert(GetSizeOfASTList(list) == 2);

  assert(GetNodeAt(list, 0) == item1);
  assert(GetNodeAt(list, 1) == item2);

  int base_capacity = list->capacity;
  while (GetSizeOfASTList(list) <= base_capacity) {
    PushToASTList(list, item1);
  }
  assert(list->capacity > base_capacity);
  assert(GetNodeAt(list, 0) == item1);
  assert(GetNodeAt(list, 1) == item2);
  assert(GetNodeAt(list, GetSizeOfASTList(list) - 1) == item1);

  fprintf(stderr, "PASS\n");
  exit(EXIT_SUCCESS);
}

void PrintASTNode(struct ASTNode *n) {
  fprintf(stderr, "(");
  PrintToken(n->op);
  fprintf(stderr, " reg: %d", n->reg);
  if (n->left) {
    fprintf(stderr, " left: ");
    PrintASTNode(n->left);
  }
  if (n->right) {
    fprintf(stderr, " right: ");
    PrintASTNode(n->right);
  }
  fprintf(stderr, ")");
}

struct ASTNode *ParseCastExpr();

struct ASTNode *ParsePrimaryExpr() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenDecimalNumber)) ||
      (t = ConsumeToken(kTokenOctalNumber))) {
    struct ASTNode *op = AllocASTNode(kASTTypePrimaryExpr);
    op->op = t;
    return op;
  }
  return NULL;
}

struct ASTNode *ParseUnaryExpr() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenPlus)) || (t = ConsumeToken(kTokenMinus)) ||
      (t = ConsumeToken(kTokenBitNot)) || (t = ConsumeToken(kTokenBoolNot))) {
    return AllocAndInitASTNodeUnaryPrefixOp(t, ParseCastExpr());
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

struct ASTNode *ParseExpr() {
  struct ASTNode *op = ParseConditionalExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenComma))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseConditionalExpr());
  }
  return op;
}

struct ASTNode *ParseExprStmt() {
  struct ASTNode *expr = ParseExpr();
  return AllocAndInitASTNodeExprStmt(ExpectToken(kTokenSemicolon), expr);
}

struct ASTNode *ParseReturnStmt() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenKwReturn))) {
    struct ASTNode *expr = ParseExpr();
    ExpectToken(kTokenSemicolon);
    return AllocAndInitASTNodeUnaryPrefixOp(t, expr);
  }
  return NULL;
}

struct ASTNode *Parse() {
  struct ASTNode *ast = ParseReturnStmt();
  struct Token *t;
  if (!(t = NextToken())) return ast;
  ErrorWithToken(t, "Unexpected token");
}

#define NUM_OF_SCRATCH_REGS 4
const char *reg_names_64[NUM_OF_SCRATCH_REGS] = {"rdi", "rsi", "r8", "r9"};
const char *reg_names_8[NUM_OF_SCRATCH_REGS] = {"dil", "sil", "r8b", "r9b"};

int reg_used_table[NUM_OF_SCRATCH_REGS];

int AllocReg() {
  for (int i = 0; i < NUM_OF_SCRATCH_REGS; i++) {
    if (!reg_used_table[i]) {
      reg_used_table[i] = 1;
      return i;
    }
  }
  Error("No more regs");
}
void FreeReg(int reg) {
  assert(0 <= reg && reg < NUM_OF_SCRATCH_REGS);
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

void Generate(struct ASTNode *node) {
  assert(node && node->op);
  if (node->op->type == kTokenDecimalNumber ||
      node->op->type == kTokenOctalNumber) {
    node->reg = AllocReg();

    printf("mov %s, %ld\n", reg_names_64[node->reg],
           strtol(node->op->begin, NULL, 0));
    return;
  }
  if (node->type == kASTTypeExprStmt) {
    if (node->left) {
      Generate(node->left);
    }
    return;
  }
  if (!node->left) {
    // unary prefix operators
    assert(node->right);
    Generate(node->right);
    node->reg = node->right->reg;
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
    if (node->op->type == kTokenKwReturn) {
      printf("mov rax, %s\n", reg_names_64[node->reg]);
      printf("ret\n");
      return;
    }
    ErrorWithToken(node->op, "Generate: Not implemented unary prefix op");
  }
  // binary operators with shortcircuit eval
  if (node->op->type == kTokenBoolAnd) {
    Generate(node->left);
    node->reg = node->left->reg;
    int skip_label = GetLabelNumber();
    EmitConvertToBool(node->reg, node->left->reg);
    printf("jz L%d\n", skip_label);
    Generate(node->right);
    EmitConvertToBool(node->reg, node->right->reg);
    FreeReg(node->right->reg);
    printf("L%d:\n", skip_label);
    return;
  }
  if (node->op->type == kTokenBoolOr) {
    Generate(node->left);
    node->reg = node->left->reg;
    int skip_label = GetLabelNumber();
    EmitConvertToBool(node->reg, node->left->reg);
    printf("jnz L%d\n", skip_label);
    Generate(node->right);
    EmitConvertToBool(node->reg, node->right->reg);
    FreeReg(node->right->reg);
    printf("L%d:\n", skip_label);
    return;
  }
  // conditional expr
  if (node->op->type == kTokenConditional) {
    Generate(node->cond);
    node->reg = node->cond->reg;
    int false_label = GetLabelNumber();
    int end_label = GetLabelNumber();
    EmitConvertToBool(node->cond->reg, node->cond->reg);
    printf("jz L%d\n", false_label);
    Generate(node->left);
    printf("mov %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->left->reg]);
    FreeReg(node->left->reg);
    printf("jmp L%d\n", end_label);
    printf("L%d:\n", false_label);
    Generate(node->right);
    printf("mov %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    FreeReg(node->right->reg);
    printf("L%d:\n", end_label);
    return;
  }
  // comma expr
  if (node->op->type == kTokenComma) {
    Generate(node->left);
    FreeReg(node->left->reg);
    Generate(node->right);
    node->reg = node->right->reg;
    return;
  }

  // binary operators
  Generate(node->left);
  Generate(node->right);
  node->reg = node->left->reg;
  FreeReg(node->right->reg);

  if (node->op->type == kTokenPlus) {
    printf("add %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    return;
  }
  if (node->op->type == kTokenMinus) {
    printf("sub %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    return;
  }
  if (node->op->type == kTokenStar) {
    // rdx:rax <- rax * r/m
    printf("xor rdx, rdx\n");
    printf("mov rax, %s\n", reg_names_64[node->reg]);
    printf("imul %s\n", reg_names_64[node->right->reg]);
    printf("mov %s, rax\n", reg_names_64[node->reg]);
    return;
  }
  if (node->op->type == kTokenSlash) {
    // rax <- rdx:rax / r/m
    printf("xor rdx, rdx\n");
    printf("mov rax, %s\n", reg_names_64[node->reg]);
    printf("idiv %s\n", reg_names_64[node->right->reg]);
    printf("mov %s, rax\n", reg_names_64[node->reg]);
    return;
  }
  if (node->op->type == kTokenPercent) {
    // rdx <- rdx:rax / r/m
    printf("xor rdx, rdx\n");
    printf("mov rax, %s\n", reg_names_64[node->reg]);
    printf("idiv %s\n", reg_names_64[node->right->reg]);
    printf("mov %s, rdx\n", reg_names_64[node->reg]);
    return;
  }
  if (node->op->type == kTokenShiftLeft) {
    // r/m <<= CL
    printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
    printf("sal %s, cl\n", reg_names_64[node->reg]);
    return;
  }
  if (node->op->type == kTokenShiftRight) {
    // r/m >>= CL
    printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
    printf("sar %s, cl\n", reg_names_64[node->reg]);
    return;
  }
  if (node->op->type == kTokenLessThan) {
    printf("cmp %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("setl %s\n", reg_names_8[node->reg]);
    printf("movzx %s, %s\n", reg_names_64[node->reg], reg_names_8[node->reg]);
    return;
  }
  if (node->op->type == kTokenGreaterThan) {
    printf("cmp %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("setg %s\n", reg_names_8[node->reg]);
    printf("movzx %s, %s\n", reg_names_64[node->reg], reg_names_8[node->reg]);
    return;
  }
  if (node->op->type == kTokenLessThanEq) {
    printf("cmp %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("setle %s\n", reg_names_8[node->reg]);
    printf("movzx %s, %s\n", reg_names_64[node->reg], reg_names_8[node->reg]);
    return;
  }
  if (node->op->type == kTokenGreaterThanEq) {
    printf("cmp %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("setge %s\n", reg_names_8[node->reg]);
    printf("movzx %s, %s\n", reg_names_64[node->reg], reg_names_8[node->reg]);
    return;
  }
  if (node->op->type == kTokenEq) {
    printf("cmp %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("sete %s\n", reg_names_8[node->reg]);
    printf("movzx %s, %s\n", reg_names_64[node->reg], reg_names_8[node->reg]);
    return;
  }
  if (node->op->type == kTokenNotEq) {
    printf("cmp %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    printf("setne %s\n", reg_names_8[node->reg]);
    printf("movzx %s, %s\n", reg_names_64[node->reg], reg_names_8[node->reg]);
    return;
  }
  if (node->op->type == kTokenBitAnd) {
    printf("and %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    return;
  }
  if (node->op->type == kTokenBitXor) {
    printf("xor %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    return;
  }
  if (node->op->type == kTokenBitOr) {
    printf("or %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    return;
  }
  ErrorWithToken(node->op, "Generate: Not implemented");
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

  printf(".intel_syntax noprefix\n");
  printf(".text\n");
  printf(".global %smain\n", symbol_prefix);
  printf("%smain:\n", symbol_prefix);
  Generate(ast);
  printf("ret\n");

  PrintASTNode(ast);
  fputc('\n', stderr);
  return 0;
}
