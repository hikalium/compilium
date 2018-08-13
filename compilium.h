#define MAX_TOKEN_LEN 64
#define MAX_INPUT_SIZE 8192

#define DefToAST(type) AST##type *ToAST##type(ASTNode *node)

#define GenToAST(Type) \
  AST##Type *ToAST##Type(ASTNode *node) { \
    if (!node || node->type != kAST##Type) return NULL; \
    return (AST##Type *)node; \
  }

#define DefAllocAST(type) AST##type *AllocAST##type(void)

#define GenAllocAST(Type) \
  AST##Type *AllocAST##Type(void) { \
    AST##Type *node = (AST##Type *)calloc(1, sizeof(AST##Type)); \
    node->type = kAST##Type; \
    return node; \
  }

typedef enum {
  kIdentifier,
  kStringLiteral,
  kCharacterLiteral,
  kInteger,
  kPunctuator,
  kKeyword,
} TokenType;

typedef enum {
  kASTFuncDef,
  kASTCompStmt,
  kASTExprUnaryPreOp,
  kASTExprUnaryPostOp,
  kASTExprBinOp,
  kASTExprFuncCall,
  kASTInteger,
  kASTString,
  kASTExprStmt,
  kASTJumpStmt,
  kASTCondStmt,
  kASTIfStmt,
  kASTWhileStmt,
  kASTForStmt,
  kASTILOp,
  kASTList,
  kASTKeyword,
  kASTDecltor,
  kASTDirectDecltor,
  kASTIdent,
  kASTDecl,
  kASTParamDecl,
  kASTPointer,
  kASTStructDecl,
  kASTStructSpec,
  //
  kASTDict,
  kASTVar,
  kASTLabel,
  kASTType,
  //
  kNumOfASTNodeType
} ASTNodeType;

typedef enum {
  kTypeNone,
  kTypeLValueOf,
  kTypePointerOf,
  kTypeArrayOf,
  kTypeStruct,
  kTypeFunction,
  kTypeVoid,
  kTypeChar,
  kTypeInt,
} BasicType;

typedef enum {
  kILOpNop,
  kILOpAdd,
  kILOpSub,
  kILOpMul,
  kILOpDiv,
  kILOpMod,
  kILOpAnd,
  kILOpXor,
  kILOpOr,
  kILOpNot,
  kILOpNegate,
  kILOpLogicalAnd,
  kILOpLogicalOr,
  kILOpLogicalNot,
  kILOpShiftLeft,
  kILOpShiftRight,
  kILOpIncrement,
  kILOpDecrement,
  kILOpCmpG,
  kILOpCmpGE,
  kILOpCmpL,
  kILOpCmpLE,
  kILOpCmpE,
  kILOpCmpNE,
  kILOpLoad8,
  kILOpLoad32,
  kILOpLoad64,
  kILOpStore8,
  kILOpStore32,
  kILOpStore64,
  kILOpLoadImm,
  kILOpLoadIdent,
  kILOpLoadArg,
  kILOpFuncBegin,
  kILOpFuncEnd,
  kILOpReturn,
  kILOpCall,
  kILOpCallParam,
  kILOpLoadLocalVarAddr,
  kILOpLabel,
  kILOpJmp,
  kILOpJmpIfZero,
  kILOpJmpIfNotZero,
  kILOpSetLogicalValue,
  kILOpAssign,
  //
  kNumOfILOpFunc
} ILOpType;

typedef enum {
  kKernelDarwin,
  kKernelLinux,
} KernelType;

typedef struct TOKEN_LIST TokenList;
typedef struct TOKEN_STREAM TokenStream;
typedef struct AST_LIST ASTList;
typedef struct AST_DICT ASTDict;
typedef struct CONTEXT Context;
typedef struct AST_LABEL ASTLabel;
typedef struct AST_VAR ASTVar;
typedef struct AST_TYPE ASTType;

typedef struct {
  int vreg_id;
  int spill_index;
  int real_reg;
} Register;

typedef struct {
  char str[MAX_TOKEN_LEN + 1];
  TokenType type;
  const char *filename;
  int line;
} Token;

typedef struct {
  ASTNodeType type;
} ASTNode;

typedef struct {
  ASTNodeType type;
  const Token *token;
} ASTKeyword;

typedef struct {
  ASTNodeType type;
  const Token *token;
  ASTVar *local_var;
  ASTType *var_type;
} ASTIdent;

typedef struct {
  ASTNodeType type;
  ASTList *root_list;
} ASTRoot;

typedef struct {
  ASTNodeType type;
  ASTList *stmt_list;
} ASTCompStmt;

typedef struct {
  ASTNodeType type;
  const Token *op;
  ASTNode *expr;
  ASTType *expr_type;
} ASTExprUnaryPreOp;

typedef struct {
  ASTNodeType type;
  const Token *op;
  ASTNode *expr;
  ASTType *expr_type;
} ASTExprUnaryPostOp;

typedef struct {
  ASTNodeType type;
  const Token *op;
  ASTNode *left;
  ASTNode *right;
  ASTType *expr_type;
} ASTExprBinOp;

typedef struct {
  ASTNodeType type;
  ASTNode *func;
  ASTNode *args;
} ASTExprFuncCall;

typedef struct {
  ASTNodeType type;
  int value;
} ASTInteger;

typedef struct {
  ASTNodeType type;
  const char *str;
} ASTString;

typedef struct {
  ASTNodeType type;
  ASTNode *expr;
} ASTExprStmt;

typedef struct {
  ASTNodeType type;
  ASTKeyword *kw;
  ASTNode *param;
} ASTJumpStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *cond_expr;
  ASTNode *true_expr;
  ASTNode *false_expr;
  ASTType *expr_type;
} ASTCondStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *cond_expr;
  ASTNode *true_stmt;
  ASTNode *false_stmt;
} ASTIfStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *cond_expr;
  ASTNode *body_stmt;
  ASTLabel *begin_label;
  ASTLabel *end_label;
} ASTWhileStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *init_expr;
  ASTNode *cond_expr;
  ASTNode *updt_expr;
  ASTNode *body_stmt;
  ASTLabel *begin_label;
  ASTLabel *end_label;
} ASTForStmt;

typedef struct {
  ASTNodeType type;
  ILOpType op;
  Register *dst;
  Register *left;
  Register *right;
  ASTNode *ast_node;
} ASTILOp;

typedef struct AST_DIRECT_DECLTOR ASTDirectDecltor;
struct AST_DIRECT_DECLTOR {
  ASTNodeType type;
  const Token *bracket_token;
  ASTDirectDecltor *direct_decltor;
  ASTNode *data;
};

typedef struct {
  ASTNodeType type;
  ASTList *decl_specs;
  ASTList *init_decltors;
} ASTDecl;

typedef struct {
  ASTNodeType type;
  ASTList *decl_specs;
  ASTNode *decltor;
} ASTParamDecl;

typedef struct AST_POINTER ASTPointer;
struct AST_POINTER {
  ASTNodeType type;
  ASTPointer *pointer;
};

typedef struct {
  ASTNodeType type;
  ASTPointer *pointer;
  ASTDirectDecltor *direct_decltor;
} ASTDecltor;

typedef struct {
  ASTNodeType type;
  ASTList *decl_specs;
  ASTDecltor *decltor;
  ASTCompStmt *comp_stmt;
  Context *context;
  ASTType *func_type;
} ASTFuncDef;

typedef struct {
  ASTNodeType type;
  ASTList *spec_qual_list;
  ASTList *struct_decltor_list;
} ASTStructDecl;

typedef struct {
  ASTNodeType type;
  const Token *ident;
  ASTList *struct_decl_list;
} ASTStructSpec;

struct AST_VAR {
  ASTNodeType type;
  int ofs;
  const char *name;
  ASTType *var_type;
};

struct AST_LABEL {
  ASTNodeType type;
  int label_number;
};

// @analyzer.c
void Analyze(ASTNode *root);

// @ast.c
void InitASTNodeTypeName();
const char *GetASTNodeTypeName(ASTNode *node);

ASTNode *ToASTNode(void *node);
DefToAST(FuncDef);
DefToAST(CompStmt);
DefToAST(ExprUnaryPreOp);
DefToAST(ExprUnaryPostOp);
DefToAST(ExprBinOp);
DefToAST(ExprFuncCall);
DefToAST(Integer);
DefToAST(String);
DefToAST(ExprStmt);
DefToAST(JumpStmt);
DefToAST(CondStmt);
DefToAST(IfStmt);
DefToAST(WhileStmt);
DefToAST(ForStmt);
DefToAST(ILOp);
DefToAST(List);
DefToAST(Keyword);
DefToAST(Decltor);
DefToAST(DirectDecltor);
DefToAST(Ident);
DefToAST(Decl);
DefToAST(ParamDecl);
DefToAST(StructDecl);
DefToAST(StructSpec);
DefToAST(Pointer);
DefToAST(Dict);
DefToAST(Var);
DefToAST(Label);

DefAllocAST(FuncDef);
DefAllocAST(CompStmt);
DefAllocAST(ExprUnaryPreOp);
DefAllocAST(ExprUnaryPostOp);
DefAllocAST(ExprBinOp);
DefAllocAST(ExprFuncCall);
DefAllocAST(Integer);
DefAllocAST(String);
DefAllocAST(ExprStmt);
DefAllocAST(JumpStmt);
DefAllocAST(CondStmt);
DefAllocAST(IfStmt);
DefAllocAST(WhileStmt);
DefAllocAST(ForStmt);
DefAllocAST(ILOp);
ASTList *AllocASTList(int capacity);
DefAllocAST(Keyword);
DefAllocAST(Decltor);
DefAllocAST(DirectDecltor);
DefAllocAST(Ident);
DefAllocAST(Decl);
DefAllocAST(ParamDecl);
DefAllocAST(StructDecl);
DefAllocAST(StructSpec);
DefAllocAST(Pointer);
ASTDict *AllocASTDict(int capacity);
DefAllocAST(Var);
DefAllocAST(Label);

ASTInteger *AllocAndInitASTInteger(int value);
ASTString *AllocAndInitASTString(const char *str);
ASTIdent *AllocAndInitASTIdent(const Token *token);
ASTKeyword *AllocAndInitASTKeyword(const Token *token);
ASTNode *AllocAndInitASTExprBinOp(const Token *op, ASTNode *left,
                                  ASTNode *right);
ASTNode *AllocAndInitASTExprFuncCall(ASTNode *func, ASTNode *args);
ASTVar *AllocAndInitASTVar(ASTList *decl_specs, ASTDecltor *decltor,
                           Context *struct_names);

const Token *GetIdentTokenFromDecltor(ASTDecltor *decltor);
const Token *GetIdentTokenFromDecltor(ASTDecltor *decltor);
const Token *GetFuncNameTokenFromFuncDef(ASTFuncDef *func_def);

void PrintASTNode(void *node, int depth);
void DebugPrintASTNode(void *node);

void PushASTNodeToList(ASTList *list, ASTNode *node);
ASTNode *PopASTNodeFromList(ASTList *list);
ASTNode *GetASTNodeAt(const ASTList *list, int index);
void SetASTNodeAt(ASTList *list, int index, ASTNode *node);
int GetSizeOfASTList(const ASTList *list);
ASTNode *GetLastASTNode(const ASTList *list);

void AppendASTNodeToDict(ASTDict *dict, const char *key, ASTNode *node);
ASTNode *FindASTNodeInDict(ASTDict *dict, const char *key);
ASTNode *GetASTNodeInDictAt(const ASTDict *dict, int index);
int GetSizeOfASTDict(const ASTDict *dict);

// @context.c
Context *AllocContext(const Context *parent);
ASTNode *FindInContext(const Context *context, const char *key);
ASTNode *FindIdentInContext(const Context *context, ASTIdent *ident);
int GetSizeOfContext(const Context *context);
int GetAlignOfContext(const Context *context);
ASTVar *AppendLocalVarToContext(Context *context, ASTList *decl_specs,
                                ASTDecltor *decltor, Context *struct_names);
ASTVar *AppendStructMemberToContext(Context *context, ASTList *decl_specs,
                                    ASTDecltor *decltor, Context *struct_names);
void AppendTypeToContext(Context *context, const char *name, ASTType *type);
void SetBreakLabelInContext(Context *context, ASTLabel *label);
ASTLabel *GetBreakLabelInContext(Context *context);
void PrintContext(const Context *context);

// @error.c
_Noreturn void Error(const char *fmt, ...);
_Noreturn void ErrorWithASTNode(void *node, const char *fmt, ...);
void Warning(const char *fmt, ...);

// @generate.c
void InitILOpTypeName();
const char *GetILOpTypeName(ILOpType type);
void GenerateCode(FILE *fp, ASTList *il, KernelType kernel_type);

// @il.c
ASTList *GenerateIL(ASTNode *root);

// @parser.c
ASTNode *Parse(TokenList *tokens);

// @token.c
Token *AllocToken(const char *s, TokenType type);
Token *AllocTokenWithSubstring(const char *begin, const char *end,
                               TokenType type, const char *filename, int line);
int IsEqualToken(const Token *token, const char *s);
int IsTypeToken(const Token *token);
void DebugPrintToken(const Token *token);
void PrintToken(const Token *token);

TokenList *AllocTokenList(int capacity);
void AppendTokenToList(TokenList *list, const Token *token);
const Token *GetTokenAt(const TokenList *list, int index);
int GetSizeOfTokenList(const TokenList *list);
void SetSizeOfTokenList(TokenList *list, int size);
void PrintTokenList(const TokenList *list);

TokenStream *AllocAndInitTokenStream(const TokenList *list);
const Token *PopToken(TokenStream *stream);
void UnpopToken(TokenStream *stream);
int GetStreamPos(TokenStream *stream);
int SeekStream(TokenStream *stream, int pos);
const Token *PeekToken(const TokenStream *stream);
int IsNextToken(TokenStream *stream, const char *str);
int IsNextTokenInList(TokenStream *stream, const char *list[]);
const Token *ConsumeToken(TokenStream *stream, const char *str);
const Token *ExpectToken(TokenStream *stream, const char *str);
void DebugPrintTokenStream(const char *s, const TokenStream *stream);

// @tokenizer.c
char *ReadFile(const char *file_name);
void Tokenize(TokenList *tokens, const char *p, const char *filename);

// @type.c
DefToAST(Type);
DefAllocAST(Type);
ASTType *AllocAndInitBasicType(BasicType basic_type);
ASTType *AllocAndInitASTTypePointerOf(ASTType *pointer_of);
ASTType *AllocAndInitASTTypeLValueOf(ASTType *lvalue_of);
ASTType *AllocAndInitASTTypeArrayOf(ASTType *array_of, int num_of_elements);
ASTType *AllocAndInitASTType(ASTList *decl_specs, ASTDecltor *decltor,
                             Context *struct_names);
int IsEqualASTType(ASTType *a, ASTType *b);
int IsBasicType(ASTType *node, BasicType type);
int IsTypePointer(ASTType *node);
int IsTypeStructLValue(ASTType *type);
ASTType *GetRValueTypeOf(ASTType *node);
ASTType *GetDereferencedTypeOf(ASTType *node);
ASTType *ConvertFromArrayToPointer(ASTType *node);
int GetSizeOfType(ASTType *node);
int GetAlignOfType(ASTType *node);
const char *GetStructTagFromType(ASTType *type);
Context *GetStructContextFromType(ASTType *type);
ASTType *GetExprTypeOfASTNode(ASTNode *node);
void PrintASTType(ASTType *node);
void DebugPrintASTType(ASTType *type);
