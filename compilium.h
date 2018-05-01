#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 64
#define MAX_TOKENS 2048
#define MAX_INPUT_SIZE 8192

typedef enum {
  kIdentifier,
  kStringLiteral,
  kCharacterLiteral,
  kInteger,
  kPunctuator,
} TokenType;

typedef struct {
  char str[MAX_TOKEN_LEN];
  TokenType type;
} Token;

#define TOKEN_LIST_SIZE 32
typedef struct {
  const Token *tokens[TOKEN_LIST_SIZE];
  int used;
} TokenList;

typedef enum {
  kRoot,
  kVarDef,
  kFuncDecl,
  kFuncDef,
  kCompStmt,
  kExprStmt,
  kReturnStmt,
  kForStatement,
} ASTType;

typedef struct AST_NODE ASTNode;

#define AST_NODE_LIST_SIZE 64
typedef struct {
  ASTNode *nodes[AST_NODE_LIST_SIZE];
  int used;
} ASTNodeList;

typedef struct {
  ASTNodeList *root_list;
} ASTDataRoot;

typedef struct {
  TokenList *type_tokens;
  const Token *name;
} ASTDataVarDef;

typedef struct {
  ASTNode *type_and_name;
  ASTNodeList *arg_list;
} ASTDataFuncDecl;

typedef struct {
  ASTNode *func_decl;
  ASTNode *comp_stmt;
} ASTDataFuncDef;

typedef struct {
  ASTNodeList *stmt_list;
} ASTDataCompStmt;

typedef struct {
  TokenList *expr;
} ASTDataExprStmt;

typedef struct {
  ASTNode *expr_stmt;
} ASTDataReturnStmt;

typedef struct {
  TokenList *init_expression;
  TokenList *cond_expression;
  TokenList *updt_expression;
  ASTNode *body_comp_stmt;
} ASTDataStatementFor;

struct AST_NODE {
  ASTType type;
  union {
    ASTDataRoot root;
    ASTDataStatementFor for_stmt;
    ASTDataReturnStmt return_stmt;
    ASTDataCompStmt comp_stmt;
    ASTDataFuncDecl func_decl;
    ASTDataFuncDef func_def;
    ASTDataExprStmt expr_stmt;
    ASTDataVarDef var_def;
  } data;
};

// @ast.c
ASTNode *AllocateASTNode(ASTType type);
void PrintASTNode(const ASTNode *node, int depth);
ASTNodeList *AllocateASTNodeList();
void AppendASTNodeToList(ASTNodeList *list, ASTNode *node);
void PrintASTNodeList(ASTNodeList *list, int depth);

// @error.c
void Error(const char *fmt, ...);

// @generate.c
void Generate(FILE *fp, const ASTNode *node);

// @parser.c
ASTNode *Parse();

// @token.c
void AddToken(const char *begin, const char *end, TokenType type);
int IsEqualToken(const Token *token, const char *s);
int IsKeyword(const Token *token);
const Token *GetTokenAt(int index);
int GetNumOfTokens();
void SetNumOfTokens(int num_of_tokens);
TokenList *AllocateTokenList();
void AppendTokenToList(TokenList *list, const Token *token);
void PrintTokenList(const TokenList *list);

// @tokenizer.c
char *ReadFile(const char *file_name);
void Tokenize(const char *p);
