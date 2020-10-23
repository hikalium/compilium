#include "compilium.h"

const char *symbol_prefix;
const char *include_path;
bool is_preprocess_only = false;

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
static struct Node *ParseCompilerArgs(int argc, char **argv) {
  // returns replacement_list: ASTList which contains macro replacement
  struct Node *replacement_list = AllocList();
  symbol_prefix = "_";
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--target-os") == 0) {
      i++;
      if (strcmp(argv[i], "Darwin") == 0) {
        symbol_prefix = "_";
        // Define __APPLE__ macro
        PushKeyValueToList(replacement_list, "__APPLE__",
                           CreateMacroReplacement(NULL, NULL));
      } else if (strcmp(argv[i], "Linux") == 0) {
        symbol_prefix = "";
      } else {
        Error("Unknown os type %s", argv[i]);
      }
    } else if (strcmp(argv[i], "-I") == 0) {
      i++;
      include_path = argv[i];
      assert(include_path);
      if (include_path[strlen(include_path) - 1] != '/') {
        Error("Include path (-I <path>) should be ended with '/'");
      }
      fprintf(stderr, "Include path: %s\n", include_path);
    } else if (strcmp(argv[i], "--run-unittest=List") == 0) {
      TestList();
    } else if (strcmp(argv[i], "--run-unittest=Type") == 0) {
      TestType();
    } else if (strcmp(argv[i], "-E") == 0) {
      is_preprocess_only = true;
    } else {
      Error("Unknown argument: %s", argv[i]);
    }
  }
  return replacement_list;
}

void PrintTokenLine(struct Node *t) {
  assert(t);
  const char *line_begin = t->begin;
  while (t->src_str < line_begin) {
    if (line_begin[-1] == '\n') break;
    line_begin--;
  }

  fprintf(stderr, "Line %d:\n", t->line);

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
}

_Noreturn void ErrorWithToken(struct Node *t, const char *fmt, ...) {
  PrintTokenLine(t);

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

// System V AMD64 ABI:
//  args:
//    RDI, RSI, RDX, RCX, R8, R9
//  callee-saved(should be kept on return):
//    RBX, RBP, R12, R13, R14, R15
//  caller-saved(can be destroyed):
//    otherwise

// Compilium register plan:
//  RAX: reserved for return values
//  RCX: reserved for shift ops
//  RDX: 3rd parameter, reserved for div/mul ops
//  RBX: reserved (callee-saved)
//  RSP: reserved for stack pointer
//  RBP: reserved for frame pointer
//  RSI: 2nd parameter
//  RDI: 1st parameter
//  R8 : 5th parameter
//  R9 : 6th parameter
//  R10: scratch
//  R11: scratch
//  R12: reserved (callee-saved)
//  R13: reserved (callee-saved)
//  R14: reserved (callee-saved)
//  R15: reserved (callee-saved)

const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1] = {
    // padding
    NULL,
    // params
    "rdi", "rsi", "r8", "r9",
    // scratch
    "r10", "r11",
    // callee-saved
    "r12", "r13", "r14", "r15"};
const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1] = {
    // padding
    NULL,
    // params
    "edi", "esi", "r8d", "r9d",
    // scratch
    "r10d", "r11d",
    // callee-saved
    "r12d", "r13d", "r14d", "r15d"};
const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1] = {
    // padding
    NULL,
    // params
    "dil", "sil", "r8b", "r9b",
    // scratch
    "r10b", "r11b",
    // callee-saved
    "r12b", "r13b", "r14b", "r15d"};
const char *param_reg_names_64[NUM_OF_PARAM_REGISTERS] = {"rdi", "rsi", "rdx",
                                                          "rcx", "r8",  "r9"};
const char *param_reg_names_32[NUM_OF_PARAM_REGISTERS] = {"edi", "esi", "edx",
                                                          "ecx", "r8d", "r9d"};
const char *param_reg_names_8[NUM_OF_PARAM_REGISTERS] = {"dl", "sil", "dl",
                                                         "cl", "r8b", "r9b"};

#define INITIAL_INPUT_SIZE 8192
const char *ReadFile(FILE *fp) {
  int buf_size = INITIAL_INPUT_SIZE;
  char *input = malloc(buf_size);
  int input_size = 0;
  int c;
  while ((c = fgetc(fp)) != EOF) {
    input[input_size++] = c;
    if (input_size == buf_size) {
      buf_size <<= 1;
      assert((input = realloc(input, buf_size)));
    }
  }
  assert(input_size < buf_size);
  input[input_size] = 0;
  return input;
}

int main(int argc, char *argv[]) {
  struct Node *replacement_list = ParseCompilerArgs(argc, argv);
  const char *input = ReadFile(stdin);

  struct Node *tokens = Tokenize(input);

  fputs("Preprocess begin\n", stderr);
  Preprocess(&tokens, replacement_list);
  if (is_preprocess_only) {
    OutputTokenSequenceAsCSource(tokens);
    return 0;
  }

  fputs("Parse begin\n", stderr);
  struct Node *ast = Parse(&tokens);
  PrintASTNode(ast);
  fputc('\n', stderr);

  fputs("Analyze begin\n", stderr);
  struct SymbolEntry *ctx = Analyze(ast);
  PrintASTNode(ast);
  fputc('\n', stderr);

  Generate(ast, ctx);
  return 0;
}
