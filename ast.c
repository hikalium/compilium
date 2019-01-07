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

struct Node *CreateASTFuncDef(struct Node *func_decl, struct Node *func_body) {
  assert(func_decl && func_decl->type == kASTDecl);
  assert(func_body && func_body->type == kASTList);
  struct Node *n = AllocNode(kASTFuncDef);
  n->func_body = func_body;
  struct Node *type = CreateTypeFromDecl(func_decl);
  assert(type);
  n->func_name_token = type->left;
  assert(IsToken(n->func_name_token));
  n->func_type = GetTypeWithoutAttr(type);
  assert(n->func_type && n->func_type->type == kTypeFunction);
  return n;
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
  assert(arg_type_list && arg_type_list->type == kASTList);
  struct Node *n = AllocNode(kTypeFunction);
  n->left = return_type;
  n->right = arg_type_list;
  return n;
}

struct Node *CreateTypeStruct(struct Node *tag_token) {
  assert(IsToken(tag_token));
  struct Node *n = AllocNode(kTypeStruct);
  n->tag = tag_token;
  return n;
}

struct Node *CreateTypeAttrIdent(struct Node *ident_token, struct Node *type) {
  assert(ident_token && IsToken(ident_token));
  struct Node *n = AllocNode(kTypeAttrIdent);
  n->left = ident_token;
  n->right = type;
  return n;
}

struct Node *CreateASTIdent(struct Node *ident) {
  struct Node *n = AllocNode(kASTIdent);
  n->op = ident;
  return n;
}

static void PrintPadding(int depth) {
  for (int i = 0; i < depth; i++) {
    fputc(' ', stderr);
  }
}

static void PrintASTNodeSub(struct Node *n, int depth) {
  if (!n) {
    fprintf(stderr, "(null)");
    return;
  }
  if (IsToken(n)) {
    PrintTokenBrief(n);
    return;
  }
  if (n->type == kASTList) {
    fprintf(stderr, "[");
    if (GetSizeOfList(n) == 0) {
      fprintf(stderr, "]");
      return;
    }
    for (int i = 0; i < GetSizeOfList(n); i++) {
      fprintf(stderr, "%s\n", i ? "," : "");
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
    PrintASTNodeSub(n->right, depth);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kTypePointer) {
    fprintf(stderr, "pointer_of<");
    PrintASTNodeSub(n->right, depth);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kTypeFunction) {
    fprintf(stderr, "function<returns: ");
    PrintASTNodeSub(n->left, depth);
    fprintf(stderr, ", args: ");
    PrintASTNodeSub(n->right, depth);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kTypeStruct) {
    fprintf(stderr, "struct<tag: ");
    PrintASTNodeSub(n->tag, depth);
    fprintf(stderr, ">");
    return;
  } else if (n->type == kTypeAttrIdent) {
    fputc('`', stderr);
    PrintTokenStrToFile(n->left, stderr);
    fputc('`', stderr);
    fprintf(stderr, " has a type: ");
    PrintASTNodeSub(n->right, depth);
    return;
  } else if (n->type == kASTFuncDef) {
    fprintf(stderr, "FuncDef ");
    PrintASTNodeSub(n->func_name_token, depth);
    fprintf(stderr, " : ");
    PrintASTNodeSub(n->func_type, depth);
    fprintf(stderr, "{\n");
    PrintPadding(depth + 1);
    PrintASTNodeSub(n->func_body, depth + 1);
    fprintf(stderr, "\n");
    PrintPadding(depth);
    fprintf(stderr, "}");
    return;
  } else if (n->type == kASTExprStmt) {
    PrintASTNodeSub(n->right, depth);
    fprintf(stderr, ";");
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

void PrintASTNode(struct Node *n) {
  PrintASTNodeSub(n, 0);
  fputc('\n', stderr);
}
