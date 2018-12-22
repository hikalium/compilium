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

static void PrintPadding(int depth) {
  for (int i = 0; i < depth; i++) {
    fputc(' ', stderr);
  }
}

void PrintToken(struct Node *t) {
  fprintf(stderr, "(Token %.*s type=%d)", t->length, t->begin, t->type);
}

void PrintTokenBrief(struct Node *t) {
  assert(t);
  if (t->type == kTokenStringLiteral || t->type == kTokenCharLiteral) {
    fprintf(stderr, "%.*s", t->length, t->begin);
    return;
  }
  fprintf(stderr, "<%.*s>", t->length, t->begin);
}

void PrintTokenStrToFile(struct Node *t, FILE *fp) {
  fprintf(fp, "%.*s", t->length, t->begin);
}

static void PrintASTNodeSub(struct Node *n, int depth) {
  if (!n) {
    fprintf(stderr, "(null)");
    return;
  }
  if (kTokenLowerBound < n->type && n->type < kTokenUpperBound) {
    PrintTokenBrief(n);
    return;
  }
  if (n->type == kASTTypeList) {
    fprintf(stderr, "[\n");
    for (int i = 0; i < GetSizeOfList(n); i++) {
      if (i) fprintf(stderr, ",\n");
      PrintPadding(depth + 1);
      PrintASTNodeSub(GetNodeAt(n, i), depth + 1);
    }
    fprintf(stderr, "\n");
    PrintPadding(depth);
    fprintf(stderr, "]");
    return;
  } else if (n->type == kASTTypeBaseType) {
    PrintTokenStrToFile(n->op, stderr);
    return;
  } else if (n->type == kASTTypeLValueOf) {
    fprintf(stderr, "lvalue<");
    PrintASTNodeSub(n->right, depth + 1);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kASTTypePointerOf) {
    fprintf(stderr, "*");
    PrintASTNodeSub(n->right, depth + 1);
    return;
  } else if (n->type == kASTTypeFunctionType) {
    fprintf(stderr, "FuncType(returns: ");
    PrintASTNodeSub(n->left, depth + 1);
    fprintf(stderr, ", args: ");
    PrintASTNodeSub(n->right, depth + 1);
    fprintf(stderr, ")");
    return;
  }
  fprintf(stderr, "(op=");
  if (n->op) PrintTokenBrief(n->op);
  if (n->expr_type) {
    fprintf(stderr, ":");
    PrintASTNodeSub(n->expr_type, depth + 1);
  }
  if (n->reg) fprintf(stderr, " reg: %d", n->reg);
  if (n->left) {
    fprintf(stderr, " L= ");
    PrintASTNodeSub(n->left, depth + 1);
  }
  if (n->right) {
    fprintf(stderr, " R= ");
    PrintASTNodeSub(n->right, depth + 1);
  }
  fprintf(stderr, ")");
}

void PrintASTNode(struct Node *n) { PrintASTNodeSub(n, 0); }
