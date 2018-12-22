#include "compilium.h"

struct CompilerArgs {
  const char *input;
};

const char *symbol_prefix;

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
  token_type_names[kTokenIdent] = "Ident";
  token_type_names[kTokenKwReturn] = "`return`";
  token_type_names[kTokenKwChar] = "`char`";
  token_type_names[kTokenKwInt] = "`int`";
  token_type_names[kTokenKwSizeof] = "`sizeof`";
  token_type_names[kTokenCharLiteral] = "CharLiteral";
  token_type_names[kTokenStringLiteral] = "StringLiteral";
  token_type_names[kTokenPunctuator] = "Punctuator";
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

struct Token *CreateNextToken(const char *p, const char *src) {
  while (*p <= ' ') {
    if (!*p) return NULL;
    p++;
  }
  if ('1' <= *p && *p <= '9') {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '9') {
      length++;
    }
    return AllocToken(src, p, length, kTokenDecimalNumber);
  } else if ('0' == *p) {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '7') {
      length++;
    }
    return AllocToken(src, p, length, kTokenDecimalNumber);
  } else if (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') ||
             *p == '_') {
    int length = 0;
    while (('A' <= p[length] && p[length] <= 'Z') ||
           ('a' <= p[length] && p[length] <= 'z') || p[length] == '_' ||
           ('0' <= p[length] && p[length] <= '9')) {
      length++;
    }
    struct Token *t = AllocToken(src, p, length, kTokenIdent);
    if (IsEqualTokenWithCStr(t, "return")) t->type = kTokenKwReturn;
    if (IsEqualTokenWithCStr(t, "char")) t->type = kTokenKwChar;
    if (IsEqualTokenWithCStr(t, "int")) t->type = kTokenKwInt;
    if (IsEqualTokenWithCStr(t, "sizeof")) t->type = kTokenKwSizeof;
    return t;
  } else if ('\'' == *p) {
    int length = 1;
    while (p[length] && p[length] != '\'') {
      if (p[length] == '\\' && p[length + 1]) {
        length++;
      }
      length++;
    }
    if (p[length] != '\'') {
      Error("Expected end of char literal (')");
    }
    length++;
    return AllocToken(src, p, length, kTokenCharLiteral);
  } else if ('"' == *p) {
    int length = 1;
    while (p[length] && p[length] != '"') {
      if (p[length] == '\\' && p[length + 1]) {
        length++;
      }
      length++;
    }
    if (p[length] != '"') {
      Error("Expected end of string literal (\")");
    }
    length++;
    return AllocToken(src, p, length, kTokenStringLiteral);
  } else if ('&' == *p) {
    if (p[1] == '&') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('|' == *p) {
    if (p[1] == '|') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('<' == *p) {
    if (p[1] == '<') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    } else if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('>' == *p) {
    if (p[1] == '>') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    } else if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('=' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('!' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('^' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('+' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('-' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('*' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('/' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('%' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('~' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('?' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (':' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (',' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (';' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('{' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('}' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('(' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (')' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  }
  Error("Unexpected char %c", *p);
}

struct Token *CreateToken(const char *input) {
  return CreateNextToken(input, input);
}

void PrintToken(struct Token *t);
void Tokenize(const char *input) {
  const char *p = input;
  struct Token *t;
  while ((t = CreateNextToken(p, input))) {
    AddToken(t);
    PrintToken(t);
    fputc('\n', stderr);
    p = t->begin + t->length;
  }
}

void PrintToken(struct Token *t) {
  fprintf(stderr, "(Token %.*s type=%s)", t->length, t->begin,
          GetTokenTypeName(t->type));
}

void PrintTokenBrief(struct Token *t) {
  assert(t);
  fprintf(stderr, "(%s<%.*s>)", GetTokenTypeName(t->type), t->length, t->begin);
}

void PrintTokenStrToFile(struct Token *t, FILE *fp) {
  fprintf(fp, "%.*s", t->length, t->begin);
}

void PrintTokenStr(struct Token *t) { PrintTokenStrToFile(t, stderr); }

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
  ErrorWithToken(tokens[token_stream_index], "Expected token %s here",
                 GetTokenTypeName(type));
}

struct Token *ConsumePunctuator(const char *s) {
  if (token_stream_index < tokens_used &&
      IsEqualTokenWithCStr(tokens[token_stream_index], s)) {
    return tokens[token_stream_index++];
  }
  return NULL;
}

struct Token *ExpectPunctuator(const char *s) {
  if (token_stream_index < tokens_used &&
      IsEqualTokenWithCStr(tokens[token_stream_index], s)) {
    return tokens[token_stream_index++];
  }
  ErrorWithToken(tokens[token_stream_index], "Expected token %s here", s);
}

struct Token *NextToken() {
  if (token_stream_index >= tokens_used) return NULL;
  return tokens[token_stream_index++];
}

struct ASTNode *AllocASTNode(enum ASTType type) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->type = type;
  return node;
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
    if (t->op->type == kTokenKwInt) return 4;
    if (t->op->type == kTokenKwChar) return 1;
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

  assert(GetSizeOfType(int_type) == 4);
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
  if (!n) {
    fprintf(stderr, "(null)");
    return;
  }
  if (n->type == kASTTypeList) {
    fprintf(stderr, "[\n");
    for (int i = 0; i < GetSizeOfList(n); i++) {
      if (i) fprintf(stderr, ",\n");
      PrintPadding(depth + 1);
      PrintASTNodeSub(GetNodeAt(n, i), depth + 1);
    }
    fprintf(stderr, "\n");
    PrintPadding(depth);
    fprintf(stderr, "]");
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
  } else if (n->type == kASTTypeFunctionType) {
    fprintf(stderr, "FuncType(returns: ");
    PrintASTNodeSub(n->left, depth + 1);
    fprintf(stderr, ", args: ");
    PrintASTNodeSub(n->right, depth + 1);
    fprintf(stderr, ")");
    return;
  }
  fprintf(stderr, "(op=");
  if (n->op) PrintTokenBrief(n->op);
  if (n->expr_type) {
    fprintf(stderr, ":");
    PrintASTNodeSub(n->expr_type, depth + 1);
  }
  if (n->reg) fprintf(stderr, " reg: %d", n->reg);
  if (n->left) {
    fprintf(stderr, " L= ");
    PrintASTNodeSub(n->left, depth + 1);
  }
  if (n->right) {
    fprintf(stderr, " R= ");
    PrintASTNodeSub(n->right, depth + 1);
  }
  fprintf(stderr, ")");
}

void PrintASTNode(struct ASTNode *n) { PrintASTNodeSub(n, 0); }

#define NUM_OF_SCRATCH_REGS 4
const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1] = {NULL, "rdi", "rsi", "r8",
                                                     "r9"};
const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1] = {NULL, "edi", "esi", "r8d",
                                                     "r9d"};
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

void AddLocalVar(struct ASTNode *list, const char *key,
                 struct ASTNode *var_type) {
  int ofs = 0;
  if (GetSizeOfList(list)) {
    struct ASTNode *n = GetNodeAt(list, GetSizeOfList(list) - 1);
    assert(n && n->type == kASTTypeKeyValue);
    struct ASTNode *v = n->value;
    assert(v && v->type == kASTTypeLocalVar);
    ofs = v->byte_offset;
  }
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct ASTNode *local_var = AllocAndInitASTNodeLocalVar(ofs, var_type);
  PushKeyValueToList(list, key, local_var);
}

struct ASTNode *CreateType(struct Token *decl_spec, struct ASTNode *decltor);
struct ASTNode *CreateTypeFromDecltor(struct ASTNode *decltor,
                                      struct ASTNode *type) {
  assert(decltor);
  struct ASTNode *pointer = decltor->left;
  if (pointer) {
    struct ASTNode *p = pointer;
    while (p->right) {
      p = p->right;
    }
    p->right = type;
    type = pointer;
  }
  struct ASTNode *direct_decltor = decltor->right;
  assert(direct_decltor->type == kASTTypeDirectDecltor);
  if (IsEqualTokenWithCStr(direct_decltor->op, "(")) {
    struct ASTNode *arg_type_list = AllocList();
    if (direct_decltor->right) {
      PushToList(arg_type_list, CreateType(direct_decltor->right->op,
                                           direct_decltor->right->right));
    }
    return AllocAndInitFunctionType(type, arg_type_list);
  }
  if (direct_decltor->op) {
    type->value = AllocAndInitASTNodeIdent(direct_decltor->op);
    return type;
  }
  assert(false);
}

struct ASTNode *CreateType(struct Token *decl_spec, struct ASTNode *decltor) {
  struct ASTNode *type = AllocAndInitBaseType(decl_spec);
  if (!decltor) return type;
  return CreateTypeFromDecltor(decltor, type);
}

void GenerateRValue(struct ASTNode *node);
struct ASTNode *var_context;
void Analyze(struct ASTNode *node) {
  if (node->type == kASTTypeList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Analyze(GetNodeAt(node, i));
    }
    return;
  }
  assert(node && node->op);
  if (node->type == kASTTypeExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber ||
        node->op->type == kTokenCharLiteral) {
      node->reg = AllocReg();
      node->expr_type = AllocAndInitBaseType(CreateToken("int"));
      return;
    } else if (node->op->type == kTokenStringLiteral) {
      node->reg = AllocReg();
      node->expr_type =
          AllocAndInitPointerOf(AllocAndInitBaseType(CreateToken("char")));
      return;
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
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
    } else if (node->cond) {
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
    } else if (!node->left && node->right) {
      Analyze(node->right);
      if (node->op->type == kTokenKwSizeof) {
        node->reg = AllocReg();
        node->expr_type = AllocAndInitBaseType(CreateToken("int"));
        return;
      }
      node->reg = node->right->reg;
      if (IsEqualTokenWithCStr(node->op, "&")) {
        node->expr_type =
            AllocAndInitPointerOf(GetRValueType(node->right->expr_type));
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "*")) {
        struct ASTNode *rtype = GetRValueType(node->right->expr_type);
        assert(rtype && rtype->type == kASTTypePointerOf);
        node->expr_type = AllocAndInitLValueOf(rtype->right);
        return;
      }
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    } else if (node->left && node->right) {
      Analyze(node->left);
      Analyze(node->right);
      if (IsEqualTokenWithCStr(node->op, "=") ||
          IsEqualTokenWithCStr(node->op, ",")) {
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
  }
  if (node->type == kASTTypeExprStmt) {
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
    struct ASTNode *type = CreateType(node->op, node->right);
    assert(type && type->value);
    AddLocalVar(var_context, CreateTokenStr(type->value->op), type);
    return;
  } else if (node->type == kASTTypeJumpStmt) {
    if (node->op->type == kTokenKwReturn) {
      Analyze(node->right);
      FreeReg(node->right->reg);
      return;
    }
  }
  ErrorWithToken(node->op, "Analyze: Not implemented");
}

struct ASTNode *str_list;
void Generate(struct ASTNode *node) {
  if (node->type == kASTTypeList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Generate(GetNodeAt(node, i));
    }
    return;
  }
  assert(node && node->op);
  if (node->type == kASTTypeExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber) {
      printf("mov %s, %ld\n", reg_names_64[node->reg],
             strtol(node->op->begin, NULL, 0));
      return;
    } else if (node->op->type == kTokenCharLiteral) {
      if (node->op->length == (1 + 1 + 1)) {
        printf("mov %s, %d\n", reg_names_64[node->reg], node->op->begin[1]);
        return;
      }
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      Generate(node->right);
      return;
    } else if (node->op->type == kTokenIdent) {
      printf("lea %s, [rbp - %d]\n", reg_names_64[node->reg],
             node->byte_offset);
      return;
    } else if (node->op->type == kTokenStringLiteral) {
      int str_label = GetLabelNumber();
      printf("lea %s, [rip + L%d]\n", reg_names_64[node->reg], str_label);
      node->label_number = str_label;
      PushToList(str_list, node);
      return;
    } else if (node->cond) {
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
    } else if (!node->left && node->right) {
      if (node->op->type == kTokenKwSizeof) {
        printf("mov %s, %d\n", reg_names_64[node->reg],
               GetSizeOfType(node->right->expr_type));
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "&")) {
        Generate(node->right);
        return;
      }
      GenerateRValue(node->right);
      if (IsEqualTokenWithCStr(node->op, "+")) {
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "-")) {
        printf("neg %s\n", reg_names_64[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "~")) {
        printf("not %s\n", reg_names_64[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "!")) {
        EmitConvertToBool(node->reg, node->reg);
        printf("setz %s\n", reg_names_8[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "*")) {
        return;
      }
      ErrorWithToken(node->op, "Generate: Not implemented unary prefix op");
    } else if (node->left && node->right) {
      if (IsEqualTokenWithCStr(node->op, "&&")) {
        GenerateRValue(node->left);
        int skip_label = GetLabelNumber();
        EmitConvertToBool(node->reg, node->left->reg);
        printf("jz L%d\n", skip_label);
        GenerateRValue(node->right);
        EmitConvertToBool(node->reg, node->right->reg);
        printf("L%d:\n", skip_label);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "||")) {
        GenerateRValue(node->left);
        int skip_label = GetLabelNumber();
        EmitConvertToBool(node->reg, node->left->reg);
        printf("jnz L%d\n", skip_label);
        GenerateRValue(node->right);
        EmitConvertToBool(node->reg, node->right->reg);
        printf("L%d:\n", skip_label);
        return;
      } else if (IsEqualTokenWithCStr(node->op, ",")) {
        Generate(node->left);
        GenerateRValue(node->right);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "=")) {
        Generate(node->left);
        GenerateRValue(node->right);
        int size = GetSizeOfType(node->right->expr_type);
        if (size == 8) {
          printf("mov [%s], %s\n", reg_names_64[node->left->reg],
                 reg_names_64[node->right->reg]);
          return;
        } else if (size == 4) {
          printf("mov [%s], %s\n", reg_names_64[node->left->reg],
                 reg_names_32[node->right->reg]);
          return;
        } else if (size == 1) {
          printf("mov [%s], %s\n", reg_names_64[node->left->reg],
                 reg_names_8[node->right->reg]);
          return;
        }
        ErrorWithToken(node->op, "Assigning %d bytes is not implemented.",
                       size);
      }
      GenerateRValue(node->left);
      GenerateRValue(node->right);
      if (IsEqualTokenWithCStr(node->op, "+")) {
        printf("add %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "-")) {
        printf("sub %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "*")) {
        // rdx:rax <- rax * r/m
        printf("xor rdx, rdx\n");
        printf("mov rax, %s\n", reg_names_64[node->reg]);
        printf("imul %s\n", reg_names_64[node->right->reg]);
        printf("mov %s, rax\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "/")) {
        // rax <- rdx:rax / r/m
        printf("xor rdx, rdx\n");
        printf("mov rax, %s\n", reg_names_64[node->reg]);
        printf("idiv %s\n", reg_names_64[node->right->reg]);
        printf("mov %s, rax\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "%")) {
        // rdx <- rdx:rax / r/m
        printf("xor rdx, rdx\n");
        printf("mov rax, %s\n", reg_names_64[node->reg]);
        printf("idiv %s\n", reg_names_64[node->right->reg]);
        printf("mov %s, rdx\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "<<")) {
        // r/m <<= CL
        printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
        printf("sal %s, cl\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, ">>")) {
        // r/m >>= CL
        printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
        printf("sar %s, cl\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "<")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "l");
        return;
      } else if (IsEqualTokenWithCStr(node->op, ">")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "g");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "<=")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "le");
        return;
      } else if (IsEqualTokenWithCStr(node->op, ">=")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "ge");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "==")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "e");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "!=")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "ne");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "&")) {
        printf("and %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "^")) {
        printf("xor %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "|")) {
        printf("or %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      }
    }
  }
  if (node->type == kASTTypeExprStmt) {
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
  }
  ErrorWithToken(node->op, "Generate: Not implemented");
}

void GenerateRValue(struct ASTNode *node) {
  Generate(node);
  if (!node->expr_type || node->expr_type->type != kASTTypeLValueOf) return;
  int size = GetSizeOfType(GetRValueType(node->expr_type));
  if (size == 8) {
    printf("mov %s, [%s]\n", reg_names_64[node->reg], reg_names_64[node->reg]);
    return;
  } else if (size == 4) {
    printf("movsxd %s, dword ptr[%s]\n", reg_names_64[node->reg],
           reg_names_64[node->reg]);
    return;
  } else if (size == 1) {
    printf("movsx %s, byte ptr[%s]\n", reg_names_64[node->reg],
           reg_names_64[node->reg]);
    return;
  }
  ErrorWithToken(node->op, "Dereferencing %d bytes is not implemented.", size);
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

  str_list = AllocList();
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

  printf(".data\n");
  for (int i = 0; i < GetSizeOfList(str_list); i++) {
    struct ASTNode *n = GetNodeAt(str_list, i);
    printf("L%d: ", n->label_number);
    printf(".asciz ");
    PrintTokenStrToFile(n->op, stdout);
    putchar('\n');
  }
}
