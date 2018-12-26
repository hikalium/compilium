#include "compilium.h"

struct Node *AllocNode(enum NodeType type) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->type = type;
  return node;
}

struct Node *CreateASTBinOp(struct Node *t, struct Node *left,
                            struct Node *right) {
  if (!right) ErrorWithToken(t, "Expected expression after binary operator");
  struct Node *op = AllocNode(kASTExpr);
  op->op = t;
  op->left = left;
  op->right = right;
  return op;
}

struct Node *CreateASTUnaryPrefixOp(struct Node *t, struct Node *right) {
  if (!right) ErrorWithToken(t, "Expected expression after prefix operator");
  struct Node *op = AllocNode(kASTExpr);
  op->op = t;
  op->right = right;
  return op;
}

struct Node *CreateASTExprStmt(struct Node *t, struct Node *left) {
  struct Node *op = AllocNode(kASTExprStmt);
  op->op = t;
  op->left = left;
  return op;
}

struct Node *CreateASTKeyValue(const char *key, struct Node *value) {
  struct Node *n = AllocNode(kASTKeyValue);
  n->key = key;
  n->value = value;
  return n;
}

struct Node *CreateASTLocalVar(int byte_offset, struct Node *var_type) {
  struct Node *n = AllocNode(kASTLocalVar);
  n->byte_offset = byte_offset;
  n->expr_type = var_type;
  return n;
}

struct Node *CreateTypeBase(struct Node *t) {
  struct Node *n = AllocNode(kTypeBase);
  n->op = t;
  return n;
}

struct Node *CreateTypeLValue(struct Node *type) {
  struct Node *n = AllocNode(kTypeLValue);
  n->right = type;
  return n;
}

struct Node *CreateTypePointer(struct Node *type) {
  struct Node *n = AllocNode(kTypePointer);
  n->right = type;
  return n;
}

struct Node *CreateTypeFunction(struct Node *return_type,
                                struct Node *arg_type_list) {
  struct Node *n = AllocNode(kTypeFunction);
  n->left = return_type;
  n->right = arg_type_list;
  return n;
}

struct Node *CreateASTIdent(struct Node *ident) {
  struct Node *n = AllocNode(kASTIdent);
  n->op = ident;
  return n;
}

struct Node *AllocToken(const char *src_str, const char *begin, int length,
                        enum NodeType type) {
  struct Node *t = AllocNode(type);
  t->begin = begin;
  t->length = length;
  t->type = type;
  t->src_str = src_str;
  return t;
}

const char *CreateTokenStr(struct Node *t) {
  return strndup(t->begin, t->length);
}

int IsEqualTokenWithCStr(struct Node *t, const char *s) {
  return strlen(s) == t->length && strncmp(t->begin, s, t->length) == 0;
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
  if (n->type == kASTList) {
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
  } else if (n->type == kTypeBase) {
    PrintTokenStrToFile(n->op, stderr);
    return;
  } else if (n->type == kTypeLValue) {
    fprintf(stderr, "lvalue<");
    PrintASTNodeSub(n->right, depth + 1);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kTypePointer) {
    fprintf(stderr, "*");
    PrintASTNodeSub(n->right, depth + 1);
    return;
  } else if (n->type == kTypeFunction) {
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
