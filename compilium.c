#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 64
#define MAX_TOKENS 2048
#define MAX_INPUT_SIZE 8192

void Error(const char *fmt, ...) {
  fprintf(stderr, "Error: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

typedef struct {
  char str[MAX_TOKEN_LEN];
} Token;

Token tokens[MAX_TOKENS];
int tokens_count;

void AddToken(const char *begin, const char *end) {
  if (end <= begin || (end - begin) >= MAX_TOKEN_LEN) {
    Error("Too long token");
  }
  if (tokens_count >= MAX_TOKENS) {
    Error("Too many token");
  }
  strncpy(tokens[tokens_count].str, begin, end - begin);
  printf("[%s]", tokens[tokens_count].str);
  tokens_count++;
}

int IsEqualToken(const Token *token, const char *s) {
  return strcmp(token->str, s) == 0;
}

const Token *GetTokenAt(int index) {
  if (index < 0 || tokens_count <= index) return NULL;
  return &tokens[index];
}

#define TOKEN_LIST_SIZE 10
typedef struct {
  const Token *tokens[TOKEN_LIST_SIZE];
  int used;
} TokenList;

TokenList *AllocateTokenList() {
  TokenList *list = malloc(sizeof(TokenList));
  list->used = 0;
  return list;
}

void AppendTokenToList(TokenList *list, const Token *token) {
  if (list->used >= TOKEN_LIST_SIZE) {
    Error("No more space in TokenList");
  }
  list->tokens[list->used++] = token;
}

void PrintTokenList(const TokenList *list) {
  for (int i = 0; i < list->used; i++) {
    printf("%s ", list->tokens[i]->str);
  }
}

char file_buf[MAX_INPUT_SIZE];
int file_buf_size;

void ReadFile(FILE *fp) {
  int file_buf_size = fread(file_buf, 1, MAX_INPUT_SIZE, fp);
  if (file_buf_size >= MAX_INPUT_SIZE) {
    Error("Too large input");
  }
  file_buf[file_buf_size] = 0;
  printf("Input size: %d\n", file_buf_size);
}

#define IS_IDENT_NODIGIT(c) \
  ((c) == '_' || ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z'))
#define IS_IDENT_DIGIT(c) (('0' <= (c) && (c) <= '9'))
void Tokenize() {
  const char *p = file_buf;
  const char *begin = NULL;
  const char *single_char_punctuators = "[](){}~?:;,\\";
  do {
    if (*p == ' ') {
      p++;
    } else if (*p == '\n') {
      p++;
      putchar('\n');
    } else if (IS_IDENT_NODIGIT(*p)) {
      begin = p++;
      while (IS_IDENT_NODIGIT(*p) || IS_IDENT_DIGIT(*p)) {
        p++;
      }
      AddToken(begin, p);
    } else if (IS_IDENT_DIGIT(*p)) {
      begin = p++;
      while (IS_IDENT_DIGIT(*p)) {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '"' || *p == '\'') {
      begin = p++;
      while (*p && *p != *begin) {
        if (*p == '\\') p++;
        p++;
      }
      if (*(p++) != *begin) {
        Error("Expected %c but got char 0x%02X", *begin, *p);
      }
      AddToken(begin, p);
    } else if (strchr(single_char_punctuators, *p)) {
      // single character punctuator
      begin = p++;
      AddToken(begin, p);
    } else if (*p == '#') {
      begin = p++;
      if (*p == '#') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '|' || *p == '&' || *p == '+' || *p == '/') {
      // | || |=
      // & && &=
      // + ++ +=
      // / // /=
      begin = p++;
      if (*p == *begin || *p == '=') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '-') {
      // - -- -= ->
      begin = p++;
      if (*p == *begin || *p == '-' || *p == '>') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '=' || *p == '!' || *p == '*') {
      // = ==
      // ! !=
      // * *=
      begin = p++;
      if (*p == '=') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '<' || *p == '>') {
      // < << <= <<=
      // > >> >= >>=
      begin = p++;
      if (*p == *begin) {
        p++;
        if (*p == '=') {
          p++;
        }
      } else if (*p == '=') {
        p++;
      }
      AddToken(begin, p);
    } else if (*p == '.') {
      // .
      // ...
      begin = p++;
      if (p[0] == '.' && p[1] == '.') {
        p += 2;
      }
      AddToken(begin, p);
    } else {
      Error("Unexpected char '%c'\n", *p);
    }
  } while (*p);
}

typedef enum {
  kNone,
  kInclude,
  kVarDef,
  kFuncDecl,
  kFuncDef,
  kCompStatement,
  kReturnStatement,
} ASTType;

typedef struct AST_NODE ASTNode;

#define AST_NODE_LIST_SIZE 64
typedef struct {
  ASTNode *nodes[AST_NODE_LIST_SIZE];
  int used;
} ASTNodeList;

struct AST_NODE {
  ASTType type;
  union {
    struct {
      TokenList *file_name_tokens;
    } directive_include;
    struct {
      TokenList *type_tokens;
      const Token *name;
    } var_def;
    struct {
      ASTNode *type_and_name;
      ASTNodeList *arg_list;
    } func_decl;
    struct {
      ASTNode *func_decl;
      ASTNode *comp_stmt;
    } func_def;
    struct {
      TokenList *expression;
    } return_stmt;
    struct {
      ASTNodeList *stmt_list;
    } comp_stmt;
  } data;
};

ASTNode *AllocateASTNode(ASTType type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = type;
  return node;
}

void PrintASTNodeList(ASTNodeList *list);
void PrintASTNode(const ASTNode *node) {
  if (node->type == kInclude) {
    printf("(Include: ");
    PrintTokenList(node->data.directive_include.file_name_tokens);
    printf(")");
  } else if (node->type == kVarDef) {
    printf("(VarDef: type=");
    PrintTokenList(node->data.var_def.type_tokens);
    printf(" name=%s", node->data.var_def.name->str);
    printf(")");
  } else if (node->type == kFuncDecl) {
    printf("(FuncDecl: type_and_name=");
    PrintASTNode(node->data.func_decl.type_and_name);
    printf(" arg_list=");
    PrintASTNodeList(node->data.func_decl.arg_list);
    printf(")");
  } else {
    Error("PrintASTNode not implemented for type %d", node->type);
  }
}

ASTNodeList *AllocateASTNodeList() {
  ASTNodeList *list = malloc(sizeof(ASTNodeList));
  list->used = 0;
  return list;
}

void AppendASTNodeToList(ASTNodeList *list, ASTNode *node) {
  if (list->used >= AST_NODE_LIST_SIZE) {
    Error("No more space in ASTNodeList");
  }
  list->nodes[list->used++] = node;
}

void PrintASTNodeList(ASTNodeList *list) {
  for (int i = 0; i < list->used; i++) {
    PrintASTNode(list->nodes[i]);
    putchar('\n');
  }
}

int IsTypeToken(const Token *token) {
  return IsEqualToken(token, "int") || IsEqualToken(token, "char");
}

ASTNode *TryReadAsVarDef(int index, int *after_index) {
  ASTNode *var_def = AllocateASTNode(kVarDef);
  const Token *token;
  TokenList *token_list = AllocateTokenList();
  var_def->data.var_def.type_tokens = token_list;

  token = GetTokenAt(index);
  if (IsEqualToken(token, "const")) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(++index);
  }
  if (IsTypeToken(token)) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(++index);
  } else {
    return NULL;
  }
  while (IsEqualToken(token, "*")) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(++index);
  }
  var_def->data.var_def.name = GetTokenAt(index++);
  *after_index = index;
  return var_def;
}

ASTNode *TryReadStatement(int index, int *after_index) {
  const Token *token;
  token = GetTokenAt(index++);
  if (IsEqualToken(token, "return")) {
    ASTNode *return_stmt = AllocateASTNode(kReturnStatement);
    TokenList *expression = AllocateTokenList();
    while((token = GetTokenAt(index++))){
      if(IsEqualToken(token, ";")) break;
      AppendTokenToList(expression, token);
    }
    return_stmt->data.return_stmt.expression = expression;
    *after_index = index;
    return return_stmt;
  }
  return NULL;
}
ASTNode *TryReadCompoundStatement(int index, int *after_index) {
  printf("Comp stmt\n");
  ASTNode *comp_stmt = AllocateASTNode(kCompStatement);
  const Token *token = GetTokenAt(index++);
  if (!IsEqualToken(token, "{")) {
    Error("Expected { but got %s", token->str);
  }
  //
  ASTNodeList *stmt_list = AllocateASTNodeList();
  ASTNode *stmt;
  while ((stmt = TryReadStatement(index, &index))) {
    AppendASTNodeToList(stmt_list, stmt);
  }
  comp_stmt->data.comp_stmt.stmt_list = stmt_list;
  //
  token = GetTokenAt(index++);
  if (!IsEqualToken(token, "}")) {
    Error("Expected } but got %s", token->str);
  }
  *after_index = index;
  return comp_stmt;
}

const ASTNodeList *Parse() {
  ASTNodeList *root = AllocateASTNodeList();
  int index = 0;
  const Token *token;
  ASTNode *tmp_node;
  for (;;) {
    token = GetTokenAt(index);
    if(!token) break;
    if (IsEqualToken(token, "\n")) {
      // skip
      index++;
    } else if (IsEqualToken(token, "#")) {
      index++;
      token = GetTokenAt(index++);
      if (IsEqualToken(token, "include")) {
        token = GetTokenAt(index++);
        ASTNode *node = AllocateASTNode(kInclude);
        if (IsEqualToken(token, "<")) {
          TokenList *tlist = AllocateTokenList();
          node->data.directive_include.file_name_tokens = tlist;
          while ((token = GetTokenAt(index++))) {
            if (IsEqualToken(token, ">")) break;
            AppendTokenToList(tlist, token);
          }
          if (!token) {
            Error("Expected > but got EOF");
          }
        } else {
          Error("Expected < but got %s", token->str);
        }
        PrintASTNode(node);
        putchar('\n');
        AppendASTNodeToList(root, node);
      } else if (IsEqualToken(token, "define")) {
        ASTNode *node = AllocateASTNode(kInclude);
        TokenList *tlist = AllocateTokenList();
        node->data.directive_include.file_name_tokens = tlist;
        while ((token = GetTokenAt(index++))) {
          if (IsEqualToken(token, ">")) break;
          AppendTokenToList(tlist, token);
        }
        if (!token) {
          Error("Expected > but got EOF");
        }
      } else {
        PrintASTNodeList(root);
        Error("Unknown preprocessor directive: %s", token->str);
      }
    } else if ((tmp_node = TryReadAsVarDef(index, &index))) {
      // func decl / def
      ASTNode *type_and_name = tmp_node;
      //
      token = GetTokenAt(index++);
      if (!IsEqualToken(token, "(")) {
        Error("Expected ( but got %s", token->str);
      }
      // args
      ASTNodeList *arg_list = AllocateASTNodeList();
      while ((tmp_node = TryReadAsVarDef(index, &index))) {
        AppendASTNodeToList(arg_list, tmp_node);
        if (!IsEqualToken(GetTokenAt(index), ",")) {
          break;
        }
        index++;
      }
      //
      token = GetTokenAt(index++);
      if (!IsEqualToken(token, ")")) {
        Error("Expected ) but got %s", token->str);
      }
      //
      ASTNode *func_decl = AllocateASTNode(kFuncDecl);
      func_decl->data.func_decl.type_and_name = type_and_name;
      func_decl->data.func_decl.arg_list = arg_list;
      // ; if decl, { if def
      token = GetTokenAt(index);
      if (IsEqualToken(token, ";")) {
        // decl
        index++;
        AppendASTNodeToList(root, func_decl);
      } else if (IsEqualToken(token, "{")) {
        // def
        ASTNode *func_def = AllocateASTNode(kFuncDef);
        ASTNode *comp_stmt = TryReadCompoundStatement(index, &index);
        func_def->data.func_def.func_decl = func_decl;
        func_def->data.func_def.comp_stmt = comp_stmt;
        AppendASTNodeToList(root, func_def);

      } else {
        Error("Expected ; or { but got %s", token->str);
      }
    } else {

      PrintASTNodeList(root);
      Error("Unexpected token: %s", token->str);
    }
  }
  return root;
}

int main(int argc, char *argv[]) {
  if (argc < 2) return 1;

  FILE *fp = fopen(argv[1], "rb");
  if (!fp) return 1;
  ReadFile(fp);
  fclose(fp);

  Tokenize();

  const ASTNodeList *ast;
  ast = Parse();
      PrintASTNodeList(ast);

  return 0;
}
