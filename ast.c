#include "compilium.h"

struct ASTNode *AllocASTNode(enum ASTType type) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->type = type;
  return node;
}

struct ASTNode *AllocAndInitASTNodeBinOp(struct ASTNode *t,
                                         struct ASTNode *left,
                                         struct ASTNode *right) {
  if (!right) ErrorWithToken(t, "Expected expression after binary operator");
  struct ASTNode *op = AllocASTNode(kASTTypeExpr);
  op->op = t;
  op->left = left;
  op->right = right;
  return op;
}

struct ASTNode *AllocAndInitASTNodeUnaryPrefixOp(struct ASTNode *t,
                                                 struct ASTNode *right) {
  if (!right) ErrorWithToken(t, "Expected expression after prefix operator");
  struct ASTNode *op = AllocASTNode(kASTTypeExpr);
  op->op = t;
  op->right = right;
  return op;
}

struct ASTNode *AllocAndInitASTNodeExprStmt(struct ASTNode *t,
                                            struct ASTNode *left) {
  struct ASTNode *op = AllocASTNode(kASTTypeExprStmt);
  op->op = t;
  op->left = left;
  return op;
}

struct ASTNode *AllocAndInitASTNodeKeyValue(const char *key,
                                            struct ASTNode *value) {
  struct ASTNode *n = AllocASTNode(kASTTypeKeyValue);
  n->key = key;
  n->value = value;
  return n;
}

struct ASTNode *AllocAndInitASTNodeLocalVar(int byte_offset,
                                            struct ASTNode *var_type) {
  struct ASTNode *n = AllocASTNode(kASTTypeLocalVar);
  n->byte_offset = byte_offset;
  n->expr_type = var_type;
  return n;
}

struct ASTNode *AllocAndInitBaseType(struct ASTNode *t) {
  struct ASTNode *n = AllocASTNode(kASTTypeBaseType);
  n->op = t;
  return n;
}

struct ASTNode *AllocAndInitLValueOf(struct ASTNode *type) {
  struct ASTNode *n = AllocASTNode(kASTTypeLValueOf);
  n->right = type;
  return n;
}

struct ASTNode *AllocAndInitPointerOf(struct ASTNode *type) {
  struct ASTNode *n = AllocASTNode(kASTTypePointerOf);
  n->right = type;
  return n;
}

struct ASTNode *AllocAndInitFunctionType(struct ASTNode *return_type,
                                         struct ASTNode *arg_type_list) {
  struct ASTNode *n = AllocASTNode(kASTTypeFunctionType);
  n->left = return_type;
  n->right = arg_type_list;
  return n;
}

struct ASTNode *AllocAndInitASTNodeIdent(struct ASTNode *ident) {
  struct ASTNode *n = AllocASTNode(kASTTypeIdent);
  n->op = ident;
  return n;
}
