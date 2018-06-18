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
  kASTFuncDecl,
  kASTFuncDef,
  kASTCompStmt,
  kASTExprBinOp,
  kASTConstant,
  kASTExprStmt,
  kASTJumpStmt,
  kASTForStmt,
  kASTILOp,
  kASTList,
  kASTKeyword,
  kASTDecltor,
  kASTDirectDecltor,
  kASTIdent,
  kASTDecl,
  //
  kNumOfASTType
} ASTType;

typedef struct AST_LIST ASTList;

typedef struct {
  ASTType type;
} ASTNode;

typedef struct {
  ASTType type;
  const Token *token;
} ASTKeyword;

typedef struct {
  ASTType type;
  ASTList *root_list;
} ASTRoot;

typedef struct {
  ASTType type;
  ASTNode *type_and_name;
  ASTList *arg_list;
} ASTFuncDecl;

typedef struct {
  ASTType type;
  ASTList *stmt_list;
} ASTCompStmt;

typedef struct {
  ASTType type;
  const Token *op;
  ASTNode *left;
  ASTNode *right;
} ASTExprBinOp;

typedef struct {
  ASTType type;
  const Token *token;
} ASTConstant;

typedef struct {
  ASTType type;
  ASTNode *expr;
} ASTExprStmt;

typedef struct {
  ASTType type;
  ASTKeyword *kw;
  ASTNode *param;
} ASTJumpStmt;

typedef struct {
  ASTType type;
  ASTNode *init_expr;
  ASTNode *cond_expr;
  ASTNode *updt_expr;
  ASTNode *body_comp_stmt;
} ASTForStmt;

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
  ASTType type;
  ILOpType op;
  int dst_reg;  // 0: unused
  int left_reg;  // 0: unused
  int right_reg;  // 0: unused
  ASTNode *ast_node;
} ASTILOp;

typedef struct {
  ASTType type;
  const Token *token;
} ASTIdent;

typedef struct AST_DIRECT_DECLTOR ASTDirectDecltor;
struct AST_DIRECT_DECLTOR {
  ASTType type;
  ASTDirectDecltor *direct_decltor;
  ASTNode *data;
};

typedef struct {
  ASTType type;
  ASTDirectDecltor *direct_decltor;
} ASTDecltor;

typedef struct {
  ASTType type;
  ASTList *decl_specs;
  ASTDecltor *decltor;
  ASTCompStmt *comp_stmt;
} ASTFuncDef;

typedef struct {
  ASTType type;
  ASTList *decl_specs;
  ASTList *init_decltors;;
} ASTDecl;

// @st.c
void InitASTTypeName();

ASTNode *ToASTNode(void *node);
#define DefToAST(type) AST##type *ToAST##type(ASTNode *node)
DefToAST(FuncDecl);
DefToAST(FuncDef);
DefToAST(CompStmt);
DefToAST(ExprBinOp);
DefToAST(Constant);
DefToAST(ExprStmt);
DefToAST(JumpStmt);
DefToAST(ForStmt);
DefToAST(ILOp);
DefToAST(List);
DefToAST(Keyword);
DefToAST(Decltor);
DefToAST(DirectDecltor);
DefToAST(Ident);
DefToAST(Decl);

#define DefAllocAST(type) AST##type *AllocAST##type()
DefAllocAST(FuncDecl);
DefAllocAST(FuncDef);
DefAllocAST(CompStmt);
DefAllocAST(ExprBinOp);
DefAllocAST(Constant);
DefAllocAST(ExprStmt);
DefAllocAST(JumpStmt);
DefAllocAST(ForStmt);
DefAllocAST(ILOp);
ASTList *AllocASTList(int capacity);
DefAllocAST(Keyword);
DefAllocAST(Decltor);
DefAllocAST(DirectDecltor);
DefAllocAST(Ident);
DefAllocAST(Decl);


ASTNode *AllocAndInitASTConstant(const Token *token);
ASTNode *AllocAndInitASTExprBinOp(const Token *op, ASTNode *left,
                                  ASTNode *right);

ASTNode *AllocateASTNodeAsILOp(ILOpType op, int dst_reg, int left_reg,
                               int right_reg, ASTNode *ast_node);

const char *GetIdentStrFromDecltor(ASTDecltor *decltor);
const char *GetIdentStrFromDecltor(ASTDecltor *decltor);
const char *GetFuncNameStrFromFuncDef(ASTFuncDef *func_def);

void PrintASTNode(ASTNode *node, int depth);
void PushASTNodeToList(ASTList *list, ASTNode *node);
ASTNode *PopASTNodeFromList(ASTList *list);
ASTNode *GetASTNodeAt(const ASTList *list, int index);
int GetSizeOfASTList(const ASTList *list);
ASTNode *GetLastASTNode(const ASTList *list);

// @error.c
void Error(const char *fmt, ...);

// @generate.c
void Generate(FILE *fp, ASTNode *root);

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
