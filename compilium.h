#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n);

#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))

enum ASTType {
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

struct ASTNode {
  enum ASTType type;
  union {
    struct {
      int reg;
      struct ASTNode *expr_type;
      struct ASTNode *op;
      struct ASTNode *left;
      struct ASTNode *right;
      struct ASTNode *cond;
      // for list
      int capacity;
      int size;
      struct ASTNode **nodes;
      // for key value
      const char *key;
      struct ASTNode *value;
      // for local var
      int byte_offset;
      // for string literal
      int label_number;
    };
    struct {
      const char *begin;
      int length;
      const char *src_str;
    };
  };
};

_Noreturn void Error(const char *fmt, ...);
_Noreturn void __assert(const char *expr_str, const char *file, int line);

_Noreturn void ErrorWithToken(struct ASTNode *t, const char *fmt, ...);

void PushToList(struct ASTNode *list, struct ASTNode *node);
void PushKeyValueToList(struct ASTNode *list, const char *key,
                        struct ASTNode *value);

void PrintASTNode(struct ASTNode *n);
struct ASTNode *AllocList();

struct ASTNode *NextToken();
struct ASTNode *ConsumeToken(enum ASTType type);
struct ASTNode *ExpectToken(enum ASTType type);
struct ASTNode *ConsumePunctuator(const char *s);
struct ASTNode *ExpectPunctuator(const char *s);

// @ast.c
struct ASTNode *AllocASTNode(enum ASTType type);
struct ASTNode *AllocAndInitASTNodeBinOp(struct ASTNode *t,
                                         struct ASTNode *left,
                                         struct ASTNode *right);
struct ASTNode *AllocAndInitASTNodeUnaryPrefixOp(struct ASTNode *t,
                                                 struct ASTNode *right);
struct ASTNode *AllocAndInitASTNodeExprStmt(struct ASTNode *t,
                                            struct ASTNode *left);

struct ASTNode *AllocAndInitASTNodeKeyValue(const char *key,
                                            struct ASTNode *value);

struct ASTNode *AllocAndInitASTNodeLocalVar(int byte_offset,
                                            struct ASTNode *var_type);

struct ASTNode *AllocAndInitBaseType(struct ASTNode *t);

struct ASTNode *AllocAndInitLValueOf(struct ASTNode *type);

struct ASTNode *AllocAndInitPointerOf(struct ASTNode *type);
struct ASTNode *AllocAndInitFunctionType(struct ASTNode *return_type,
                                         struct ASTNode *arg_type_list);
struct ASTNode *AllocAndInitASTNodeIdent(struct ASTNode *ident);

// @parser.c
struct ASTNode *Parse();
