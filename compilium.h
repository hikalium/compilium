#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n);

#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))

enum NodeType {
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
  kASTTypeNone,
  kASTTypeExpr,
  kASTTypeList,
  kASTTypeExprStmt,
  kASTTypeJumpStmt,
  kASTTypeIdent,
  kASTTypeDirectDecltor,
  kASTTypeDecltor,
  kASTTypeDecl,
  kASTTypeKeyValue,
  kASTTypeLocalVar,
  kASTTypeBaseType,
  kASTTypeLValueOf,
  kASTTypePointerOf,
  kASTTypeFunctionType,
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

struct Node *NextToken();
struct Node *ConsumeToken(enum NodeType type);
struct Node *ExpectToken(enum NodeType type);
struct Node *ConsumePunctuator(const char *s);
struct Node *ExpectPunctuator(const char *s);

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

struct Node *AllocAndInitBaseType(struct Node *t);

struct Node *AllocAndInitLValueOf(struct Node *type);

struct Node *AllocAndInitPointerOf(struct Node *type);
struct Node *AllocAndInitFunctionType(struct Node *return_type,
                                      struct Node *arg_type_list);
struct Node *AllocAndInitASTNodeIdent(struct Node *ident);
void PrintASTNode(struct Node *n);

void PrintTokenStrToFile(struct Node *t, FILE *fp);

// @parser.c
struct Node *Parse();
