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

static struct Node *SkipDelimiterTokensInLogicalLine(struct Node *t) {
  while (t && t->token_type == kTokenDelimiter &&
         !IsEqualTokenWithCStr(t, "\n"))
    t = t->next_token;
  return t;
}

static const char *CreateStrFromTokenRange(struct Node *begin,
                                           struct Node *end) {
  assert(begin);
  int len = 0;
  for (struct Node *t = begin; t && t != end; t = t->next_token) {
    len += t->length;
  }
  return strndup(begin->begin, len);
}

#define INITIAL_INPUT_SIZE 8192
static const char *ReadFile(FILE *fp) {
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

static void PreprocessRemoveBlock(void) {
  for (struct Node *t = PeekToken(); t; t = t->next_token) {
    if (!IsEqualTokenWithCStr(t, "#")) {
      continue;
    }
    t = SkipDelimiterTokensInLogicalLine(t->next_token);
    if (!IsEqualTokenWithCStr(t, "endif")) {
      continue;
    }
    RemoveTokensTo(t);
    break;
  }
}

static struct Node *TryReadIdentListWrappedByParens(struct Node **tp) {
  // If ( ident_list ) is read, this function returns cloned tokens of
  // ident_list without commas and tp is advanced to next token.
  // If not, this function returns NULL and tp is unchanged.
  struct Node *t = *tp;
  if (!IsEqualTokenWithCStr(t, "(")) {
    return NULL;
  }
  struct Node *ident_list_head = NULL;
  struct Node **ident_list_last_holder = &ident_list_head;
  for (t = SkipDelimiterTokensInLogicalLine(t->next_token); t;
       t = SkipDelimiterTokensInLogicalLine(t->next_token)) {
    if (IsEqualTokenWithCStr(t, ")")) break;
    *ident_list_last_holder = DuplicateToken(t);
    ident_list_last_holder = &(*ident_list_last_holder)->next_token;
    t = SkipDelimiterTokensInLogicalLine(t->next_token);
    if (!IsEqualTokenWithCStr(t, ",")) break;
  }
  if (!IsEqualTokenWithCStr(t, ")")) {
    return NULL;
  }
  // To distinguish function-like macro with zero args and
  // token level replacement macro, add ) at the end of args
  // to ensure args is not NULL
  *ident_list_last_holder = DuplicateToken(t);
  *tp = SkipDelimiterTokensInLogicalLine(t->next_token);
  return ident_list_head;
}

static void PreprocessBlock(struct Node *replacement_list, int level) {
  struct Node *t;
  while (PeekToken()) {
    if ((t = ConsumeTokenStr("__LINE__"))) {
      char s[32];
      snprintf(s, sizeof(s), "%d", t->line);
      t->token_type = kTokenDecimalNumber;
      t->begin = t->src_str = strdup(s);
      t->length = strlen(t->begin);
      continue;
    }
    if ((t = ReadToken(kTokenLineComment))) {
      while (t && !IsEqualTokenWithCStr(t, "\n")) t = t->next_token;
      RemoveTokensTo(t);
      continue;
    }
    if ((t = ReadToken(kTokenBlockCommentBegin))) {
      while (t && !IsTokenWithType(t, kTokenBlockCommentEnd)) t = t->next_token;
      if (IsTokenWithType(t, kTokenBlockCommentEnd)) t = t->next_token;
      RemoveTokensTo(t);
      continue;
    }
    if (IsEqualTokenWithCStr((t = PeekToken()), "#")) {
      assert(t);
      t = SkipDelimiterTokensInLogicalLine(t->next_token);
      if (IsEqualTokenWithCStr(t, "define")) {
        assert(t);
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        struct Node *from = t;
        t = t->next_token;
        struct Node *ident_list = TryReadIdentListWrappedByParens(&t);
        t = SkipDelimiterTokensInLogicalLine(t);
        assert(t);
        struct Node *to_token_head = NULL;
        struct Node **to_token_last_holder = &to_token_head;
        while (t && !IsEqualTokenWithCStr(t, "\n")) {
          *to_token_last_holder = DuplicateToken(t);
          to_token_last_holder = &(*to_token_last_holder)->next_token;
          t = t->next_token;
        }
        assert(IsEqualTokenWithCStr(t, "\n"));
        RemoveTokensTo(t->next_token);
        PushKeyValueToList(replacement_list, CreateTokenStr(from),
                           CreateMacroReplacement(ident_list, to_token_head));
        continue;
      }
      if (IsEqualTokenWithCStr(t, "include")) {
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        if (!IsEqualTokenWithCStr(t, "<")) {
          ErrorWithToken(t, "Expected < here");
        }
        struct Node *markL = t;
        t = t->next_token;
        struct Node *begin = t;
        while (t && !IsEqualTokenWithCStr(t, ">")) {
          t = t->next_token;
        }
        if (!t) {
          ErrorWithToken(markL,
                         "Unexpected EOF. > is expected to match with this.");
        }
        struct Node *end = t;
        const char *fname = CreateStrFromTokenRange(begin, end);
        RemoveTokensTo(end->next_token);
        if (!include_path) {
          ErrorWithToken(markL,
                         "Include path is not provided in compiler args");
        }
        char path[256];
        if (sizeof(path) <= strlen(include_path) + strlen(fname)) {
          ErrorWithToken(markL, "Include path is too long");
        }
        strcpy(path, include_path);
        strcat(path, fname);
        FILE *fp = fopen(path, "rb");
        if (!fp) {
          ErrorWithToken(markL, "File not found: %s", path);
        }
        const char *include_input = ReadFile(fp);
        InsertTokens(Tokenize(include_input));
        fclose(fp);
        continue;
      }
      if (IsEqualTokenWithCStr(t, "ifdef")) {
        struct Node *ifdef_token = t;
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        struct Node *e;
        bool cond = (e = GetNodeByTokenKey(replacement_list, t));
        // defined
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        RemoveTokensTo(t);
        if (cond) {
          PreprocessBlock(replacement_list, level + 1);
        } else {
          PreprocessRemoveBlock();
        }
        t = PeekToken();
        if (!IsEqualTokenWithCStr(t, "endif")) {
          ErrorWithToken(ifdef_token,
                         "Unexpected eof. Expected #endif to match with this.");
        }
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        RemoveTokensTo(t);
        continue;
      }
      if (IsEqualTokenWithCStr(t, "endif")) {
        if (level == 0) {
          ErrorWithToken(t, "Unexpected endif here");
        }
        RemoveTokensTo(t);
        return;
      }
      ErrorWithToken(NextToken(), "Not a valid macro");
    }
    struct Node *e;
    if ((e = GetNodeByTokenKey(replacement_list, (t = PeekToken())))) {
      assert(e->type == kNodeMacroReplacement);
      struct Node *rep = DuplicateTokenSequence(e->value);
      RemoveCurrentToken();
      if (!e->arg_expr_list) {
        // ident replace macro case
        InsertTokens(rep);
        continue;
      }
      // function-like macro case
      t = SkipDelimiterTokensInLogicalLine(t->next_token);
      if (!IsEqualTokenWithCStr(t, "(")) ErrorWithToken(t, "Expected ( here");
      t = t->next_token;
      struct Node *it;
      struct Node *arg_rep_list = AllocList();
      for (it = e->arg_expr_list; it; it = it->next_token) {
        if (IsEqualTokenWithCStr(it, ")")) break;
        struct Node *arg_token_head = NULL;
        struct Node **arg_token_last_holder = &arg_token_head;
        t = SkipDelimiterTokensInLogicalLine(t);
        for (; t; t = t->next_token) {
          if (IsEqualTokenWithCStr(t, ")") || IsEqualTokenWithCStr(t, ","))
            break;
          *arg_token_last_holder = DuplicateToken(t);
          arg_token_last_holder = &(*arg_token_last_holder)->next_token;
        }
        PushKeyValueToList(arg_rep_list, CreateTokenStr(it),
                           CreateMacroReplacement(NULL, arg_token_head));
        if (IsEqualTokenWithCStr(t, ")")) break;
        t = t->next_token;
      }
      if (!IsEqualTokenWithCStr(t, ")")) ErrorWithToken(t, "Expected ) here");
      RemoveTokensTo(t->next_token);
      // Insert & replace args
      InsertTokensWithIdentReplace(rep, arg_rep_list);
      continue;
    }
    NextToken();
  }
}

void Preprocess(struct Node **head_holder) {
  InitTokenStream(head_holder);
  PreprocessBlock(AllocList(), 0);
}

int main(int argc, char *argv[]) {
  ParseCompilerArgs(argc, argv);
  const char *input = ReadFile(stdin);

  struct Node *tokens = Tokenize(input);

  fputs("Preprocess begin\n", stderr);
  Preprocess(&tokens);
  if (is_preprocess_only) {
    OutputTokenSequenceAsCSource(tokens);
    return 0;
  }

  fputs("Parse begin\n", stderr);
  struct Node *ast = Parse(&tokens);
  PrintASTNode(ast);
  fputc('\n', stderr);

  fputs("Analyze begin\n", stderr);
  Analyze(ast);
  PrintASTNode(ast);
  fputc('\n', stderr);

  Generate(ast);
  return 0;
}
