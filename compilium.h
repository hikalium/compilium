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
  //
  kTokenLowerBound,
  kTokenDecimalNumber,
  kTokenOctalNumber,
  kTokenIdent,
  kTokenKwReturn,
  kTokenKwChar,
  kTokenKwInt,
  kTokenKwSizeof,
  kTokenCharLiteral,
  kTokenStringLiteral,
  kTokenPunctuator,
  kTokenUpperBound,
  //
  kASTExpr,
  kASTList,
  kASTExprStmt,
  kASTJumpStmt,
  kASTIdent,
  kASTDirectDecltor,
  kASTDecltor,
  kASTDecl,
  kASTKeyValue,
  kASTLocalVar,
  //
  kTypeBase,
  kTypeLValue,
  kTypePointer,
  kTypeFunction,
};

struct Node {
  enum NodeType type;
  union {
    struct {
      int reg;
      struct Node *expr_type;
      struct Node *op;
      struct Node *left;
      struct Node *right;
      struct Node *cond;
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
    };
    struct {
      // kToken...
      const char *begin;
      int length;
      const char *src_str;
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

// @ast.c
struct Node *AllocASTNode(enum NodeType type);
struct Node *AllocAndInitASTNodeBinOp(struct Node *t, struct Node *left,
                                      struct Node *right);
struct Node *AllocAndInitASTNodeUnaryPrefixOp(struct Node *t,
                                              struct Node *right);
struct Node *AllocAndInitASTNodeExprStmt(struct Node *t, struct Node *left);

struct Node *AllocAndInitASTNodeKeyValue(const char *key, struct Node *value);

struct Node *AllocAndInitASTNodeLocalVar(int byte_offset,
                                         struct Node *var_type);

struct Node *CreateTypeBase(struct Node *t);

struct Node *CreateTypeLValue(struct Node *type);

struct Node *CreateTypePointer(struct Node *type);
struct Node *CreateTypeFunction(struct Node *return_type,
                                struct Node *arg_type_list);
struct Node *AllocAndInitASTNodeIdent(struct Node *ident);

struct Node *AllocToken(const char *src_str, const char *begin, int length,
                        enum NodeType type);

const char *CreateTokenStr(struct Node *t);
int IsEqualTokenWithCStr(struct Node *t, const char *s);
void PrintTokenStrToFile(struct Node *t, FILE *fp);

void PrintASTNode(struct Node *n);

// @parser.c
struct Node *Parse();

// tokenizer.c
struct Node *CreateToken(const char *input);

// @type.c
int IsSameType(struct Node *a, struct Node *b);
struct Node *GetRValueType(struct Node *t);
int GetSizeOfType(struct Node *t);
struct Node *CreateType(struct Node *decl_spec, struct Node *decltor);
