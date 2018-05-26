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
  int used;  // TODO: rename used -> size
} TokenList;

typedef enum {
  kRoot,
  kVarDef,
  kFuncDecl,
  kFuncDef,
  kCompStmt,
  kExprBinOp,
  kExprVal,
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

typedef enum {
  kOpAdd,
} ASTExprBinOpType;

typedef struct {
  ASTExprBinOpType op_type;
  ASTNode *left;
  ASTNode *right;
} ASTDataExprBinOp;

typedef struct {
  const Token *token;
} ASTDataExprVal;

typedef struct {
  ASTNode *expr;
} ASTDataExprStmt;

typedef struct {
  ASTNode *expr_stmt;
} ASTDataReturnStmt;

typedef struct {
  ASTNode *init_expr;
  ASTNode *cond_expr;
  ASTNode *updt_expr;
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
    ASTDataExprBinOp expr_bin_op;
    ASTDataExprVal expr_val;
    ASTDataExprStmt expr_stmt;
    ASTDataVarDef var_def;
  } data;
};

// @ast.c
const ASTDataRoot *GetDataAsRoot(const ASTNode *node);
const ASTDataVarDef *GetDataAsVarDef(const ASTNode *node);
const ASTDataFuncDecl *GetDataAsFuncDecl(const ASTNode *node);
const ASTDataFuncDef *GetDataAsFuncDef(const ASTNode *node);
const ASTDataCompStmt *GetDataAsCompStmt(const ASTNode *node);
const ASTDataExprBinOp *GetDataAsExprBinOp(const ASTNode *node);
const ASTDataExprVal *GetDataAsExprVal(const ASTNode *node);
const ASTDataExprStmt *GetDataAsExprStmt(const ASTNode *node);
const ASTDataReturnStmt *GetDataAsReturnStmt(const ASTNode *node);

ASTNode *AllocateASTNode(ASTType type);
ASTNode *AllocateASTNodeAsExprVal(const Token *token);
ASTNode *AllocateASTNodeAsExprBinOp(ASTExprBinOpType op_type);
void SetOperandOfExprBinOp(ASTNode *node, ASTNode *left, ASTNode *right);

void PrintASTNode(const ASTNode *node, int depth);
ASTNodeList *AllocateASTNodeList();
void PushASTNodeToList(ASTNodeList *list, ASTNode *node);
ASTNode *PopASTNodeFromList(ASTNodeList *list);
void PrintASTNodeList(ASTNodeList *list, int depth);

// @error.c
void Error(const char *fmt, ...);

// @generate.c
void Generate(FILE *fp, const ASTNode *node);

// @parser.c
ASTNode *Parse();

// @token.c
Token *AllocateToken(const char *s, TokenType type);
void AddToken(const char *begin, const char *end, TokenType type);
int IsEqualToken(const Token *token, const char *s);
int IsKeyword(const Token *token);
const Token *GetTokenAt(int index);
int GetNumOfTokens();
void SetNumOfTokens(int num_of_tokens);
TokenList *AllocateTokenList();
void AppendTokenToList(TokenList *list, const Token *token);
void PrintToken(const Token *token);
void PrintTokenList(const TokenList *list);

// @tokenizer.c
char *ReadFile(const char *file_name);
void Tokenize(const char *p);
