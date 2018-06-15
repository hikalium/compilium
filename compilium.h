#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 64
#define MAX_INPUT_SIZE 8192

typedef enum {
  kIdentifier,
  kStringLiteral,
  kCharacterLiteral,
  kInteger,
  kPunctuator,
} TokenType;

typedef struct {
  char str[MAX_TOKEN_LEN + 1];
  TokenType type;
} Token;

typedef struct TOKEN_LIST TokenList;

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
  kILOp,
} ASTType;

typedef struct AST_NODE ASTNode;
typedef struct AST_NODE_LIST ASTNodeList;

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
  kOpUndefined,
  kOpAdd,
  kOpSub,
  kOpMul,
  kOpFuncCall,
  kNumOfExprBinOp
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

typedef enum {
  kILOpAdd,
  kILOpSub,
  kILOpMul,
  kILOpLoadImm,
  kILOpFuncBegin,
  kILOpFuncEnd,
  kILOpReturn,
  //
  kNumOfILOpFunc
} ILOpType;

typedef struct {
  ILOpType op;
  int dst_reg;    // 0: unused
  int left_reg;   // 0: unused
  int right_reg;  // 0: unused
  const ASTNode *ast_node;
} ASTDataILOp;

struct AST_NODE {
  ASTType type;
  union {
    // TODO: change struct names to match with ASTType
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
    ASTDataILOp il_op;
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
const ASTDataILOp *GetDataAsILOpOfType(const ASTNode *node, ILOpType type);

ASTNode *AllocateASTNode(ASTType type);

ASTNode *AllocateASTNodeAsExprVal(const Token *token);
ASTNode *AllocateASTNodeAsExprBinOp(ASTExprBinOpType op_type);
void SetOperandOfExprBinOp(ASTNode *node, ASTNode *left, ASTNode *right);

ASTNode *AllocateASTNodeAsILOp(ILOpType op, int dst_reg, int left_reg,
                               int right_reg, const ASTNode *ast_node);

void PrintASTNode(const ASTNode *node, int depth);
ASTNodeList *AllocateASTNodeList(int capacity);
void PushASTNodeToList(ASTNodeList *list, ASTNode *node);
ASTNode *PopASTNodeFromList(ASTNodeList *list);
ASTNode *GetASTNodeAt(const ASTNodeList *list, int index);
int GetSizeOfASTNodeList(const ASTNodeList *list);
ASTNode *GetLastASTNode(const ASTNodeList *list);
void PrintASTNodeList(ASTNodeList *list, int depth);

// @error.c
void Error(const char *fmt, ...);

// @generate.c
void Generate(FILE *fp, const ASTNode *root);

// @parser.c
ASTNode *Parse(TokenList *tokens);

// @token.c
Token *AllocateToken(const char *s, TokenType type);
Token *AllocateTokenWithSubstring(const char *begin, const char *end,
                                  TokenType type);
int IsEqualToken(const Token *token, const char *s);
int IsKeyword(const Token *token);
int IsTypeToken(const Token *token);
void SetNumOfTokens(int num_of_tokens);
TokenList *AllocateTokenList();
void AppendTokenToList(TokenList *list, const Token *token);
const Token *GetTokenAt(TokenList *list, int index);
int GetSizeOfTokenList(const TokenList *list);
void SetSizeOfTokenList(TokenList *list, int size);
void PrintToken(const Token *token);
void PrintTokenList(const TokenList *list);

// @tokenizer.c
char *ReadFile(const char *file_name);
void Tokenize(TokenList *tokens, const char *p);
