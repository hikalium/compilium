#include "include/stdarg.h"
#include "include/stdbool.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"

char *strndup(const char *s, size_t n);
char *strdup(const char *s);

#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))

enum NodeType {
  kNodeNone,
  kNodeToken,
  kNodeStructMember,
  kNodeMacroReplacement,
  //
  kASTExpr,
  kASTExprFuncCall,
  kASTList,
  kASTExprStmt,
  kASTJumpStmt,
  kASTSelectionStmt,
  kASTIdent,
  kASTDirectDecltor,
  kASTDecltor,
  kASTDecl,
  kASTForStmt,
  kASTWhileStmt,
  kASTFuncDef,
  kASTKeyValue,
  kASTLocalVar,
  kASTStructSpec,
  //
  kTypeBase,
  kTypeLValue,
  kTypePointer,
  kTypeFunction,
  kTypeAttrIdent,
  kTypeStruct,
  kTypeArray,
};

enum TokenType {
  kTokenUnknownChar,
  kTokenDelimiter,
  kTokenZeroWidthNoBreakSpace,
  kTokenIntegerConstant,
  kTokenIdent,
  kTokenKwBreak,
  kTokenKwChar,
  kTokenKwConst,
  kTokenKwContinue,
  kTokenKwElse,
  kTokenKwExtern,
  kTokenKwFor,
  kTokenKwIf,
  kTokenKwInt,
  kTokenKwLong,
  kTokenKwReturn,
  kTokenKwSizeof,
  kTokenKwStatic,
  kTokenKwStruct,
  kTokenKwTypedef,
  kTokenKwUnsigned,
  kTokenKwVoid,
  kTokenKwWhile,
  kTokenCharLiteral,
  kTokenStringLiteral,
  kTokenPunctuator,
  kTokenLineComment,
  kTokenBlockCommentBegin,
  kTokenBlockCommentEnd,
};

/*
Node if-stmt:
  stmt->cond = cond-expr
  stmt->left = true-stmt
  stmt->right = false-stmt or null

Node func-call-expr:
  expr->func_expr
  expr->arg_expr_list

Node expr-stmt:
  stmt->op = token(;)
  stmt->left = node
*/

struct Node {
  enum NodeType type;
  int reg;
  struct Node *expr_type;
  struct Node *op;
  struct Node *left;
  struct Node *right;
  struct Node *init;
  struct Node *cond;
  struct Node *updt;
  struct Node *body;
  struct Node *if_true_stmt;
  struct Node *if_else_stmt;
  struct Node *decltor_init_expr;
  struct Node *decltor_init_stmt;
  struct Node *struct_member_dict;
  struct Node *struct_member_ent_type;
  struct Node *struct_member_decl;
  int struct_member_ent_ofs;
  // for list
  int capacity;
  int size;
  struct Node **nodes;
  // for key value
  const char *key;
  struct Node *value;
  // for local var
  int byte_offset;
  // for string literal
  int label_number;
  // kASTExprFuncCall
  struct Node *func_expr;
  struct Node *arg_expr_list;
  struct Node *arg_var_list;
  int stack_size_needed;
  // kASTFuncDef
  struct Node *func_body;
  struct Node *func_type;
  struct Node *func_name_token;
  struct Node *tag;
  struct Node *type_struct_spec;
  struct Node *type_array_type_of;
  struct Node *type_array_index_decl;
  // kNodeToken
  enum TokenType token_type;
  struct Node *next_token;
  const char *begin;
  int length;
  const char *src_str;
  int line;
};

_Noreturn void Error(const char *fmt, ...);
_Noreturn void __assert(const char *expr_str, const char *file, int line);

void PrintTokenLine(struct Node *t);
_Noreturn void ErrorWithToken(struct Node *t, const char *fmt, ...);

void PushToList(struct Node *list, struct Node *node);
void PushKeyValueToList(struct Node *list, const char *key, struct Node *value);

struct Node *AllocList();
int GetSizeOfList(struct Node *list);
struct Node *GetNodeAt(struct Node *list, int index);
struct Node *GetNodeByTokenKey(struct Node *list, struct Node *key);

extern const char *symbol_prefix;
extern const char *include_path;

#define NUM_OF_SCRATCH_REGS 10
extern const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1];
extern const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1];
extern const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1];

#define NUM_OF_PARAM_REGISTERS 6
extern const char *param_reg_names_64[NUM_OF_PARAM_REGISTERS];
extern const char *param_reg_names_32[NUM_OF_PARAM_REGISTERS];
extern const char *param_reg_names_8[NUM_OF_PARAM_REGISTERS];

// @analyzer.c
struct SymbolEntry *Analyze(struct Node *node);

// @ast.c
bool IsToken(struct Node *n);
bool IsTokenWithType(struct Node *n, enum TokenType type);
bool IsASTList(struct Node *);
bool IsASTDeclOfTypedef(struct Node *n);
bool IsASTDeclOfExtern(struct Node *n);
struct Node *AllocNode(enum NodeType type);
struct Node *CreateASTBinOp(struct Node *t, struct Node *left,
                            struct Node *right);
struct Node *CreateASTUnaryPrefixOp(struct Node *t, struct Node *right);
struct Node *CreateASTUnaryPostfixOp(struct Node *left, struct Node *t);
struct Node *CreateASTExprStmt(struct Node *t, struct Node *left);
struct Node *CreateASTFuncDef(struct Node *func_decl, struct Node *func_body);

struct Node *CreateASTKeyValue(const char *key, struct Node *value);

struct Node *CreateASTLocalVar(int byte_offset, struct Node *var_type);

struct Node *CreateTypeBase(struct Node *t);

struct Node *CreateTypeLValue(struct Node *type);

struct Node *CreateTypePointer(struct Node *type);
struct Node *CreateTypeFunction(struct Node *return_type,
                                struct Node *arg_type_list);
struct Node *GetReturnTypeOfFunction(struct Node *);
struct Node *GetArgTypeList(struct Node *func_type);
struct Node *CreateTypeStruct(struct Node *tag_token, struct Node *struct_spec);
struct Node *CreateTypeAttrIdent(struct Node *ident_token, struct Node *type);
struct Node *CreateASTIdent(struct Node *ident);
struct Node *CreateTypeArray(struct Node *type_of, struct Node *index_decl);
struct Node *CreateMacroReplacement(struct Node *args_tokens,
                                    struct Node *to_tokens);
void PrintASTNode(struct Node *n);

// @compilium.c
const char *ReadFile(FILE *fp);

// @generate.c
void Generate(struct Node *ast, struct SymbolEntry *);

// @parser.c
extern struct Node *toplevel_names;
void InitParser(struct Node **);
struct Node *Parse(struct Node **passed_tokens);

// @preprocessor.c
void Preprocess(struct Node **head_holder, struct Node *replacement_list);

// @struct.c
struct SymbolEntry;
int CalcStructSize(struct Node *spec);
int CalcStructAlign(struct Node *spec);
void AddMemberOfStructFromDecl(struct Node *struct_spec, struct Node *decl);
struct Node *FindStructMember(struct Node *struct_type, struct Node *key_token);
void ResolveTypesOfMembersOfStruct(struct SymbolEntry *ctx, struct Node *spec);

// @symbol.c
enum SymbolType {
  kSymbolLocalVar,
  kSymbolGlobalVar,
  kSymbolExternVar,
  kSymbolFuncDef,
  kSymbolFuncDeclType,
  kSymbolStructType,
};
struct SymbolEntry {
  enum SymbolType type;
  struct SymbolEntry *prev;
  const char *key;
  struct Node *value;
};
int GetLastLocalVarOffset(struct SymbolEntry *);
struct Node *AddLocalVar(struct SymbolEntry **ctx, const char *key,
                         struct Node *var_type);
void AddExternVar(struct SymbolEntry **ctx, const char *key,
                  struct Node *var_type);
void AddGlobalVar(struct SymbolEntry **ctx, const char *key,
                  struct Node *var_type);
struct Node *FindExternVar(struct SymbolEntry *e, struct Node *key_token);
struct Node *FindGlobalVar(struct SymbolEntry *e, struct Node *key_token);
struct Node *FindLocalVar(struct SymbolEntry *e, struct Node *key_token);
void AddFuncDef(struct SymbolEntry **ctx, const char *key,
                struct Node *func_def);
struct Node *FindFuncDef(struct SymbolEntry *e, struct Node *key_token);
void AddFuncDeclType(struct SymbolEntry **ctx, const char *key,
                     struct Node *func_decl);
struct Node *FindFuncDeclType(struct SymbolEntry *e, struct Node *key_token);
void AddStructType(struct SymbolEntry **, const char *, struct Node *);
struct Node *FindStructType(struct SymbolEntry *, struct Node *);

// @token.c
bool IsToken(struct Node *n);
struct Node *AllocToken(const char *src_str, int line, const char *begin,
                        int length, enum TokenType type);
struct Node *DuplicateToken(struct Node *base_token);
struct Node *DuplicateTokenSequence(struct Node *base_head);
char *CreateTokenStr(struct Node *t);
int IsEqualTokenWithCStr(struct Node *t, const char *s);
void PrintTokenSequence(struct Node *t);
void OutputTokenSequenceAsCSource(struct Node *t);
void PrintToken(struct Node *t);
void PrintTokenBrief(struct Node *t);
void PrintTokenStrToFile(struct Node *t, FILE *fp);

void InitTokenStream(struct Node **head_token);
struct Node *PeekToken(void);
struct Node *ReadToken(enum TokenType type);
struct Node *ConsumeToken(enum TokenType type);
struct Node *ConsumeTokenStr(const char *s);
struct Node *ExpectTokenStr(const char *s);
struct Node *ConsumePunctuator(const char *s);
struct Node *ExpectPunctuator(const char *s);
struct Node *NextToken(void);
void RemoveCurrentToken(void);
void RemoveTokensTo(struct Node *end);
void InsertTokens(struct Node *);
void InsertTokensWithIdentReplace(struct Node *seq, struct Node *rep_list);
struct Node **RemoveDelimiterTokens(struct Node **);

// @tokenizer.c
struct Node *CreateToken(const char *input);
struct Node *Tokenize(const char *input);

// @type.c
int IsSameTypeExceptAttr(struct Node *a, struct Node *b);
int IsLValueType(struct Node *t);
struct Node *GetTypeWithoutAttr(struct Node *t);
struct Node *GetIdentifierTokenFromTypeAttr(struct Node *t);
struct Node *GetRValueType(struct Node *t);
int GetSizeOfType(struct Node *t);
int GetAlignOfType(struct Node *t);
struct Node *CreateTypeInContext(struct SymbolEntry *ctx,
                                 struct Node *decl_spec, struct Node *decltor);
struct Node *CreateType(struct Node *decl_spec, struct Node *decltor);
struct Node *CreateTypeFromDecl(struct Node *decl);
struct Node *CreateTypeFromDeclInContext(struct SymbolEntry *ctx,
                                         struct Node *decl);
