#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum OSType {
  kOSDarwin,
  kOSLinux,
};

struct CompilerArgs {
  enum OSType os_type;
  const char *input;
};

#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))

enum TokenTypes {
  kTokenDecimalNumber,
  kTokenOctalNumber,
  kTokenPlus,
  kNumOfTokenTypeNames
};

struct Token {
  const char *begin;
  int length;
  enum TokenTypes type;
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

void ParseCompilerArgs(struct CompilerArgs *args, int argc, char **argv) {
  args->os_type = kOSDarwin;
  args->input = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--os_type") == 0) {
      i++;
      if (strcmp(argv[i], "Darwin") == 0) {
        args->os_type = kOSDarwin;
      } else if (strcmp(argv[i], "Linux") == 0) {
        args->os_type = kOSLinux;
      } else {
        Error("Unknown os type %s", argv[i]);
      }
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
}

const char *GetTokenTypeName(const struct Token *t) {
  assert(t && 0 <= t->type && t->type < kNumOfTokenTypeNames);
  assert(token_type_names[t->type]);
  return token_type_names[t->type];
}

void AddToken(const char *begin, int length, enum TokenTypes type) {
  assert(tokens_used < NUM_OF_TOKENS);
  tokens[tokens_used].begin = begin;
  tokens[tokens_used].length = length;
  tokens[tokens_used].type = type;
  tokens_used++;
}

void Tokenize(const char *s) {
  while (*s) {
    if ('1' <= *s && *s <= '9') {
      int length = 0;
      while ('0' <= s[length] && s[length] <= '9') {
        length++;
      }
      AddToken(s, length, kTokenDecimalNumber);
      s += length;
      continue;
    }
    if ('0' == *s) {
      int length = 0;
      while ('0' <= s[length] && s[length] <= '7') {
        length++;
      }
      AddToken(s, length, kTokenOctalNumber);
      s += length;
      continue;
    }
    if ('+' == *s) {
      AddToken(s++, 1, kTokenPlus);
      continue;
    }
    Error("Unexpected char %c", *s);
  }
}

void PrintToken(struct Token *t) {
  fprintf(stderr, "(Token %.*s type=%s)", t->length, t->begin,
          GetTokenTypeName(t));
}

void PrintTokens() {
  for (int i = 0; i < tokens_used; i++) {
    struct Token *t = &tokens[i];
    PrintToken(t);
    fputc('\n', stderr);
  }
}

int token_stream_index;
struct Token *ConsumeToken(enum TokenTypes type) {
  if (token_stream_index < tokens_used &&
      tokens[token_stream_index].type == type) {
    return &tokens[token_stream_index++];
  }
  return NULL;
}

struct ASTNode {
  struct Token *op;
  int reg;
  struct ASTNode *left;
  struct ASTNode *right;
};

struct ASTNode *AllocASTNode() {
  return calloc(1, sizeof(struct ASTNode));
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

struct ASTNode *ParsePrimaryExpr() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenDecimalNumber)) ||
      (t = ConsumeToken(kTokenOctalNumber))) {
    struct ASTNode *op = AllocASTNode();
    op->op = t;
    return op;
  }
  return NULL;
}

struct ASTNode *ParseAddExpr() {
  struct ASTNode *op = ParsePrimaryExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumeToken(kTokenPlus))) {
    struct ASTNode *right = ParsePrimaryExpr();
    if (!right) Error("Expected expression after +");
    struct ASTNode *new_op = AllocASTNode();
    new_op->op = t;
    new_op->left = op;
    new_op->right = right;
    op = new_op;
  }
  return op;
}

struct ASTNode *Parse() {
  return ParseAddExpr();
}

#define NUM_OF_SCRATCH_REGS 6
const char *reg_names_64[NUM_OF_SCRATCH_REGS] = {"rdi", "rsi", "rdx",
                                                 "rcx", "r8",  "r9"};

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

void Generate(struct ASTNode *node) {
  assert(node && node->op);
  if (node->op->type == kTokenDecimalNumber ||
      node->op->type == kTokenOctalNumber) {
    node->reg = AllocReg();

    printf("mov %s, %ld\n", reg_names_64[node->reg],
           strtol(node->op->begin, NULL, 0));
    return;
  }
  if (node->op->type == kTokenPlus) {
    Generate(node->left);
    Generate(node->right);
    node->reg = node->left->reg;
    FreeReg(node->right->reg);

    printf("add %s, %s\n", reg_names_64[node->reg],
           reg_names_64[node->right->reg]);
    return;
  }
}

int main(int argc, char *argv[]) {
  struct CompilerArgs args;
  ParseCompilerArgs(&args, argc, argv);

  InitTokenTypeNames();

  Tokenize(args.input);
  PrintTokens();

  struct ASTNode *ast = Parse();

  printf(".intel_syntax noprefix\n");
  printf(".text\n");
  printf(".global _main\n");
  printf("%smain:\n", args.os_type == kOSDarwin ? "_" : "");
  Generate(ast);
  printf("mov rax, %s\n", reg_names_64[ast->reg]);
  printf("ret\n");

  PrintASTNode(ast);
  fputc('\n', stderr);
  return 0;
}
