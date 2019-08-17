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

const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1] = {NULL, "rdi", "rsi", "r8",
                                                     "r9"};
const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1] = {NULL, "edi", "esi", "r8d",
                                                     "r9d"};
const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1] = {NULL, "dil", "sil", "r8b",
                                                    "r9b"};
const char *param_reg_names_64[NUM_OF_PARAM_REGISTERS] = {"rdi", "rsi", "rdx",
                                                          "rcx", "r8",  "r9"};
const char *param_reg_names_32[NUM_OF_PARAM_REGISTERS] = {"edi", "esi", "edx",
                                                          "ecx", "r8d", "r9d"};
const char *param_reg_names_8[NUM_OF_PARAM_REGISTERS] = {"dl", "sil", "dl",
                                                         "cl", "r8b", "r9b"};

void Preprocess(struct Node **p) {
  if (!p || !*p) return;
  while (*p) {
    if (IsTokenWithType(*p, kTokenIdent) &&
        IsEqualTokenWithCStr(*p, "__LINE__")) {
      char s[32];
      snprintf(s, sizeof(s), "%d", (*p)->line);
      (*p)->token_type = kTokenDecimalNumber;
      (*p)->begin = (*p)->src_str = strdup(s);
      (*p)->length = strlen((*p)->begin);
      p = &((*p)->next_token);
      continue;
    }
    if (IsTokenWithType(*p, kTokenLineComment)) {
      int target_line = (*p)->line;
      struct Node *n = *p;
      while (n && n->line == target_line) n = n->next_token;
      *p = n;
      continue;
    }
    if (IsTokenWithType(*p, kTokenBlockCommentBegin)) {
      struct Node *n = *p;
      while (n && !IsTokenWithType(n, kTokenBlockCommentEnd)) n = n->next_token;
      assert(n);
      *p = n->next_token;
      continue;
    }
    p = &(*p)->next_token;
  }
}

#define INITIAL_INPUT_SIZE 8192
int main(int argc, char *argv[]) {
  ParseCompilerArgs(argc, argv);
  int buf_size = INITIAL_INPUT_SIZE;
  char *input = malloc(buf_size);
  int input_size = 0;
  int c;
  while ((c = getchar()) != EOF) {
    input[input_size++] = c;
    if (input_size == buf_size) {
      buf_size <<= 1;
      assert((input = realloc(input, buf_size)));
    }
  }
  assert(input_size < buf_size);
  input[input_size] = 0;

  fprintf(stderr, "input:\n%s\n", input);
  struct Node *tokens = Tokenize(input);
  PrintTokenSequence(tokens);

  Preprocess(&tokens);
  PrintTokenSequence(tokens);

  struct Node *ast = Parse(tokens);
  PrintASTNode(ast);
  fputc('\n', stderr);

  Analyze(ast);
  PrintASTNode(ast);
  fputc('\n', stderr);

  Generate(ast);
  return 0;
}
