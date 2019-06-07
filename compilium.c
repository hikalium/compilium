#include "compilium.h"

const char *symbol_prefix;

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

void TestList(void);
void TestType(void);
void ParseCompilerArgs(int argc, char **argv) {
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
      Error("Unknown argument: %s", argv[i]);
    }
  }
}

struct Node *CreateNextToken(const char *p, const char *src) {
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
    struct Node *t = AllocToken(src, p, length, kTokenIdent);
    if (IsEqualTokenWithCStr(t, "char")) t->type = kTokenKwChar;
    if (IsEqualTokenWithCStr(t, "if")) t->type = kTokenKwIf;
    if (IsEqualTokenWithCStr(t, "int")) t->type = kTokenKwInt;
    if (IsEqualTokenWithCStr(t, "return")) t->type = kTokenKwReturn;
    if (IsEqualTokenWithCStr(t, "sizeof")) t->type = kTokenKwSizeof;
    if (IsEqualTokenWithCStr(t, "struct")) t->type = kTokenKwStruct;
    if (IsEqualTokenWithCStr(t, "void")) t->type = kTokenKwVoid;
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

struct Node *CreateToken(const char *input) {
  return CreateNextToken(input, input);
}

void PrintToken(struct Node *t);
struct Node *Tokenize(const char *input) {
  const char *p = input;
  struct Node *t;
  struct Node *tokens = AllocList();
  while ((t = CreateNextToken(p, input))) {
    PushToList(tokens, t);
    p = t->begin + t->length;
  }
  return tokens;
}

_Noreturn void ErrorWithToken(struct Node *t, const char *fmt, ...) {
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

struct Node *AllocList() {
  return AllocNode(kASTList);
}

void ExpandListSizeIfNeeded(struct Node *list) {
  if (list->size < list->capacity) return;
  list->capacity = (list->capacity + 1) * 2;
  list->nodes = realloc(list->nodes, sizeof(struct Node *) * list->capacity);
  assert(list->nodes);
  assert(list->size < list->capacity);
}

void PushToList(struct Node *list, struct Node *node) {
  ExpandListSizeIfNeeded(list);
  list->nodes[list->size++] = node;
}

void PushKeyValueToList(struct Node *list, const char *key,
                        struct Node *value) {
  assert(key && value);
  ExpandListSizeIfNeeded(list);
  list->nodes[list->size++] = CreateASTKeyValue(key, value);
}

int GetSizeOfList(struct Node *list) {
  assert(list && list->type == kASTList);
  return list->size;
}

struct Node *GetNodeAt(struct Node *list, int index) {
  assert(list && list->type == kASTList);
  assert(0 <= index && index < list->size);
  return list->nodes[index];
}

struct Node *GetNodeByTokenKey(struct Node *list, struct Node *key) {
  assert(list && list->type == kASTList);
  for (int i = 0; i < list->size; i++) {
    struct Node *n = list->nodes[i];
    if (n->type != kASTKeyValue) continue;
    if (IsEqualTokenWithCStr(key, n->key)) return n->value;
  }
  return NULL;
}

struct Node *GetNodeByKey(struct Node *list, const char *key) {
  assert(list && list->type == kASTList);
  for (int i = 0; i < list->size; i++) {
    struct Node *n = list->nodes[i];
    if (n->type != kASTKeyValue) continue;
    if (strcmp(n->key, key) == 0) return n->value;
  }
  return NULL;
}

void TestList() {
  fprintf(stderr, "Testing List...");

  struct Node *list = AllocList();
  struct Node *item1 = AllocNode(kNodeNone);
  struct Node *item2 = AllocNode(kNodeNone);
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

const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1] = {NULL, "rdi", "rsi", "r8",
                                                     "r9"};
const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1] = {NULL, "edi", "esi", "r8d",
                                                     "r9d"};
const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1] = {NULL, "dil", "sil", "r8b",
                                                    "r9b"};
const char *param_reg_names_64[NUM_OF_PARAM_REGISTERS] = {"rdi", "rsi", "rdx",
                                                          "rcx", "r8",  "r9"};

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

void AddLocalVar(struct Node *list, const char *key, struct Node *var_type) {
  int ofs = 0;
  if (GetSizeOfList(list)) {
    struct Node *n = GetNodeAt(list, GetSizeOfList(list) - 1);
    assert(n && n->type == kASTKeyValue);
    struct Node *v = n->value;
    assert(v && v->type == kASTLocalVar);
    ofs = v->byte_offset;
  }
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct Node *local_var = CreateASTLocalVar(ofs, var_type);
  PushKeyValueToList(list, key, local_var);
}

void GenerateRValue(struct Node *node);
struct Node *var_context;
extern struct Node *toplevel_names;
void Analyze(struct Node *node) {
  assert(node);
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Analyze(GetNodeAt(node, i));
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    node->reg = AllocReg();
    // TODO: support expe_type other than int
    node->expr_type = CreateTypeBase(CreateToken("int"));
    Analyze(node->func_expr);
    FreeReg(node->func_expr->reg);
    for (int i = 0; i < GetSizeOfList(node->arg_expr_list); i++) {
      struct Node *n = GetNodeAt(node->arg_expr_list, i);
      Analyze(n);
      FreeReg(n->reg);
    }
    return;
  } else if (node->type == kASTFuncDef) {
    Analyze(node->func_body);
    return;
  }
  assert(node->op);
  if (node->type == kASTExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber ||
        node->op->type == kTokenCharLiteral) {
      node->reg = AllocReg();
      node->expr_type = CreateTypeBase(CreateToken("int"));
      return;
    } else if (node->op->type == kTokenStringLiteral) {
      node->reg = AllocReg();
      node->expr_type = CreateTypePointer(CreateTypeBase(CreateToken("char")));
      return;
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      Analyze(node->right);
      node->reg = node->right->reg;
      node->expr_type = node->right->expr_type;
      return;
    } else if (node->op->type == kTokenIdent) {
      struct Node *ident_info = GetNodeByTokenKey(var_context, node->op);
      if (ident_info) {
        if (ident_info->type == kASTLocalVar) {
          node->byte_offset = ident_info->byte_offset;
          node->reg = AllocReg();
          node->expr_type = CreateTypeLValue(ident_info->expr_type);
          return;
        }
      }
      ident_info = GetNodeByTokenKey(toplevel_names, node->op);
      if (ident_info) {
        if (ident_info->type == kTypeFunction) {
          node->reg = AllocReg();
          node->expr_type = ident_info;
          return;
        }
      }
      ErrorWithToken(node->op, "Unknown identifier");
    } else if (node->cond) {
      Analyze(node->cond);
      Analyze(node->left);
      Analyze(node->right);
      FreeReg(node->left->reg);
      FreeReg(node->right->reg);
      assert(
          IsSameTypeExceptAttr(node->left->expr_type, node->right->expr_type));
      node->reg = node->cond->reg;
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    } else if (!node->left && node->right) {
      Analyze(node->right);
      if (node->op->type == kTokenKwSizeof) {
        node->reg = AllocReg();
        node->expr_type = CreateTypeBase(CreateToken("int"));
        return;
      }
      node->reg = node->right->reg;
      if (IsEqualTokenWithCStr(node->op, "&")) {
        node->expr_type =
            CreateTypePointer(GetRValueType(node->right->expr_type));
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "*")) {
        struct Node *rtype = GetRValueType(node->right->expr_type);
        assert(rtype && rtype->type == kTypePointer);
        node->expr_type = CreateTypeLValue(rtype->right);
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
  if (node->type == kASTExprStmt) {
    if (!node->left) return;
    Analyze(node->left);
    if (node->left->reg) FreeReg(node->left->reg);
    return;
  } else if (node->type == kASTList) {
    var_context = AllocList();
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Analyze(GetNodeAt(node, i));
    }
    return;
  } else if (node->type == kASTDecl) {
    struct Node *type = CreateType(node->op, node->right);
    assert(type && type->type == kTypeAttrIdent);
    AddLocalVar(var_context, CreateTokenStr(type->left), type->right);
    return;
  } else if (node->type == kASTJumpStmt) {
    if (node->op->type == kTokenKwReturn) {
      if (!node->right) return;
      Analyze(node->right);
      FreeReg(node->right->reg);
      return;
    }
  } else if (node->type == kASTSelectionStmt) {
    if (node->op->type == kTokenKwIf) {
      Analyze(node->cond);
      FreeReg(node->cond->reg);
      Analyze(node->left);
      assert(!node->right);
      return;
    }
  }
  ErrorWithToken(node->op, "Analyze: Not implemented");
}

#define MAX_INPUT_SIZE 4096
int main(int argc, char *argv[]) {
  ParseCompilerArgs(argc, argv);
  char *input = malloc(MAX_INPUT_SIZE);
  int input_size;
  for (input_size = 0;
       input_size < MAX_INPUT_SIZE && (input[input_size] = getchar()) != EOF;
       input_size++)
    ;
  assert(input_size < MAX_INPUT_SIZE);

  fprintf(stderr, "input:\n%s\n", input);
  struct Node *tokens = Tokenize(input);
  PrintASTNode(tokens);

  struct Node *ast = Parse(tokens);
  PrintASTNode(ast);
  fputc('\n', stderr);

  Analyze(ast);
  PrintASTNode(ast);
  fputc('\n', stderr);

  Generate(ast);
  return 0;
}
