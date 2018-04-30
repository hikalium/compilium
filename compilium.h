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
  kNone,
  kInclude,
  kVarDef,
  kFuncDecl,
  kFuncDef,
  kCompStatement,
  kExpressionStatement,
  kReturnStatement,
  kForStatement,
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
      ASTNodeList *stmt_list;
    } comp_stmt;
    struct {
      TokenList *expression;
    } expression_stmt;
    struct {
      ASTNode *expression_stmt;
    } return_stmt;
    struct {
      TokenList *init_expression;
      TokenList *cond_expression;
      TokenList *updt_expression;
      ASTNode *body_comp_stmt;
    } for_stmt;
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

// @parser.c
ASTNodeList *Parse();

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

