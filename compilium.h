#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n);

#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))

enum NodeType {
  kNodeNone,
  kNodeToken,
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
};

enum TokenType {
  kTokenDecimalNumber,
  kTokenOctalNumber,
  kTokenIdent,
  kTokenKwChar,
  kTokenKwFor,
  kTokenKwIf,
  kTokenKwInt,
  kTokenKwReturn,
  kTokenKwSizeof,
  kTokenKwStruct,
  kTokenKwVoid,
  kTokenCharLiteral,
  kTokenStringLiteral,
  kTokenPunctuator,
  kTokenLineComment,
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
  union {
    struct {
      int reg;
      struct Node *expr_type;
      struct Node *op;
      struct Node *left;
      struct Node *right;
      struct Node *init;
      struct Node *cond;
      struct Node *updt;
      struct Node *body;
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
      union {
        struct {
          // kASTExprFuncCall
          struct Node *func_expr;
          struct Node *arg_expr_list;
          struct Node *arg_var_list;
          int stack_size_needed;
        };
      };
    };
    struct {
      // kASTFuncDef
      struct Node *func_body;
      struct Node *func_type;
      struct Node *func_name_token;
    };
    struct {
      // kASTStruct, kTypeStruct
      struct Node *tag;
    };
    struct {
      // kNodeToken
      enum TokenType token_type;
      struct Node *next_token;
      const char *begin;
      int length;
      const char *src_str;
      int line;
    };
  };
};

_Noreturn void Error(const char *fmt, ...);
_Noreturn void __assert(const char *expr_str, const char *file, int line);

_Noreturn void ErrorWithToken(struct Node *t, const char *fmt, ...);

void PushToList(struct Node *list, struct Node *node);
void PushKeyValueToList(struct Node *list, const char *key, struct Node *value);

struct Node *AllocList();
int GetSizeOfList(struct Node *list);
struct Node *GetNodeAt(struct Node *list, int index);
struct Node *GetNodeByTokenKey(struct Node *list, struct Node *key);

extern const char *symbol_prefix;

#define NUM_OF_SCRATCH_REGS 4
extern const char *reg_names_64[NUM_OF_SCRATCH_REGS + 1];
extern const char *reg_names_32[NUM_OF_SCRATCH_REGS + 1];
extern const char *reg_names_8[NUM_OF_SCRATCH_REGS + 1];

#define NUM_OF_PARAM_REGISTERS 6
extern const char *param_reg_names_64[NUM_OF_PARAM_REGISTERS];
extern const char *param_reg_names_32[NUM_OF_PARAM_REGISTERS];
extern const char *param_reg_names_8[NUM_OF_PARAM_REGISTERS];

// @analyzer.c
void Analyze(struct Node *node);

// @ast.c
bool IsToken(struct Node *n);
bool IsTokenWithType(struct Node *n, enum TokenType type);
struct Node *AllocNode(enum NodeType type);
struct Node *CreateASTBinOp(struct Node *t, struct Node *left,
                            struct Node *right);
struct Node *CreateASTUnaryPrefixOp(struct Node *t, struct Node *right);
struct Node *CreateASTExprStmt(struct Node *t, struct Node *left);
struct Node *CreateASTFuncDef(struct Node *func_decl, struct Node *func_body);

struct Node *CreateASTKeyValue(const char *key, struct Node *value);

struct Node *CreateASTLocalVar(int byte_offset, struct Node *var_type);

struct Node *CreateTypeBase(struct Node *t);

struct Node *CreateTypeLValue(struct Node *type);

struct Node *CreateTypePointer(struct Node *type);
struct Node *CreateTypeFunction(struct Node *return_type,
                                struct Node *arg_type_list);
struct Node *GetArgTypeList(struct Node *func_type);
struct Node *CreateTypeStruct(struct Node *tag_token);
struct Node *CreateTypeAttrIdent(struct Node *ident_token, struct Node *type);
struct Node *CreateASTIdent(struct Node *ident);
void PrintASTNode(struct Node *n);

// @generate.c
void Generate(struct Node *ast);

// @parser.c
extern struct Node *toplevel_names;
void InitParser(struct Node *head_token);
struct Node *Parse(struct Node *passed_tokens);

// @token.c
bool IsToken(struct Node *n);
struct Node *AllocToken(const char *src_str, int line, const char *begin,
                        int length, enum TokenType type);
const char *CreateTokenStr(struct Node *t);
int IsEqualTokenWithCStr(struct Node *t, const char *s);
void PrintTokenSequence(struct Node *t);
void PrintToken(struct Node *t);
void PrintTokenBrief(struct Node *t);
void PrintTokenStrToFile(struct Node *t, FILE *fp);

// @tokenizer.c
struct Node *CreateToken(const char *input);
struct Node *Tokenize(const char *input);

// @type.c
int IsSameTypeExceptAttr(struct Node *a, struct Node *b);
struct Node *GetTypeWithoutAttr(struct Node *t);
struct Node *GetIdentifierTokenFromTypeAttr(struct Node *t);
struct Node *GetRValueType(struct Node *t);
int GetSizeOfType(struct Node *t);
struct Node *CreateType(struct Node *decl_spec, struct Node *decltor);
struct Node *CreateTypeFromDecl(struct Node *decl);
