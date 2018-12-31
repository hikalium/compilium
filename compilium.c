#include "compilium.h"

struct CompilerArgs {
  const char *input;
};

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
    if (IsEqualTokenWithCStr(t, "return")) t->type = kTokenKwReturn;
    if (IsEqualTokenWithCStr(t, "char")) t->type = kTokenKwChar;
    if (IsEqualTokenWithCStr(t, "int")) t->type = kTokenKwInt;
    if (IsEqualTokenWithCStr(t, "void")) t->type = kTokenKwVoid;
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

#define NUM_OF_SCRATCH_REGS 4
const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1] = {NULL, "rdi", "rsi", "r8",
                                                     "r9"};
const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1] = {NULL, "edi", "esi", "r8d",
                                                     "r9d"};
const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1] = {NULL, "dil", "sil", "r8b",
                                                    "r9b"};

#define NUM_OF_PARAM_REGISTERS 6
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
      Analyze(node->right);
      FreeReg(node->right->reg);
      return;
    }
  }
  ErrorWithToken(node->op, "Analyze: Not implemented");
}

struct Node *str_list;
void Generate(struct Node *node) {
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Generate(GetNodeAt(node, i));
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    GenerateRValue(node->func_expr);
    printf("push %s\n", reg_names_64[node->func_expr->reg]);
    int i;
    assert(GetSizeOfList(node->arg_expr_list) <= NUM_OF_PARAM_REGISTERS);
    for (i = 0; i < GetSizeOfList(node->arg_expr_list); i++) {
      struct Node *n = GetNodeAt(node->arg_expr_list, i);
      GenerateRValue(n);
      printf("push %s\n", reg_names_64[n->reg]);
    }
    for (i--; i >= 0; i--) {
      printf("pop %s\n", param_reg_names_64[i]);
    }
    printf("pop rax\n");
    printf("call rax\n");
    return;
  } else if (node->type == kASTFuncDef) {
    Generate(node->func_body);
    return;
  }
  assert(node && node->op);
  if (node->type == kASTExpr) {
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
      if (node->expr_type->type == kTypeFunction) {
        const char *label_name = CreateTokenStr(node->op);
        printf(".global %s%s\n", symbol_prefix, label_name);
        printf("mov %s, [rip + %s%s@GOTPCREL]\n", reg_names_64[node->reg],
               symbol_prefix, label_name);
        return;
      }
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
  if (node->type == kASTExprStmt) {
    if (node->left) Generate(node->left);
    return;
  } else if (node->type == kASTList) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Generate(GetNodeAt(node, i));
    }
    return;
  } else if (node->type == kASTDecl) {
    return;
  } else if (node->type == kASTJumpStmt) {
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

void GenerateRValue(struct Node *node) {
  Generate(node);
  if (!node->expr_type || node->expr_type->type != kTypeLValue) return;
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

  fprintf(stderr, "input:\n%s\n", args.input);
  struct Node *tokens = Tokenize(args.input);
  PrintASTNode(tokens);

  struct Node *ast = Parse(tokens);
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
    struct Node *n = GetNodeAt(str_list, i);
    printf("L%d: ", n->label_number);
    printf(".asciz ");
    PrintTokenStrToFile(n->op, stdout);
    putchar('\n');
  }
}
