#include "compilium.h"

struct Node *AllocASTNode(enum NodeType type) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->type = type;
  return node;
}

struct Node *AllocAndInitASTNodeBinOp(struct Node *t, struct Node *left,
                                      struct Node *right) {
  if (!right) ErrorWithToken(t, "Expected expression after binary operator");
  struct Node *op = AllocASTNode(kASTTypeExpr);
  op->op = t;
  op->left = left;
  op->right = right;
  return op;
}

struct Node *AllocAndInitASTNodeUnaryPrefixOp(struct Node *t,
                                              struct Node *right) {
  if (!right) ErrorWithToken(t, "Expected expression after prefix operator");
  struct Node *op = AllocASTNode(kASTTypeExpr);
  op->op = t;
  op->right = right;
  return op;
}

struct Node *AllocAndInitASTNodeExprStmt(struct Node *t, struct Node *left) {
  struct Node *op = AllocASTNode(kASTTypeExprStmt);
  op->op = t;
  op->left = left;
  return op;
}

struct Node *AllocAndInitASTNodeKeyValue(const char *key, struct Node *value) {
  struct Node *n = AllocASTNode(kASTTypeKeyValue);
  n->key = key;
  n->value = value;
  return n;
}

struct Node *AllocAndInitASTNodeLocalVar(int byte_offset,
                                         struct Node *var_type) {
  struct Node *n = AllocASTNode(kASTTypeLocalVar);
  n->byte_offset = byte_offset;
  n->expr_type = var_type;
  return n;
}

struct Node *AllocAndInitBaseType(struct Node *t) {
  struct Node *n = AllocASTNode(kASTTypeBaseType);
  n->op = t;
  return n;
}

struct Node *AllocAndInitLValueOf(struct Node *type) {
  struct Node *n = AllocASTNode(kASTTypeLValueOf);
  n->right = type;
  return n;
}

struct Node *AllocAndInitPointerOf(struct Node *type) {
  struct Node *n = AllocASTNode(kASTTypePointerOf);
  n->right = type;
  return n;
}

struct Node *AllocAndInitFunctionType(struct Node *return_type,
                                      struct Node *arg_type_list) {
  struct Node *n = AllocASTNode(kASTTypeFunctionType);
  n->left = return_type;
  n->right = arg_type_list;
  return n;
}

struct Node *AllocAndInitASTNodeIdent(struct Node *ident) {
  struct Node *n = AllocASTNode(kASTTypeIdent);
  n->op = ident;
  return n;
}
