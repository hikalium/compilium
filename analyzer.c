#include "compilium.h"

static int reg_used_table[NUM_OF_SCRATCH_REGS];

static int AllocReg() {
  for (int i = 1; i <= NUM_OF_SCRATCH_REGS; i++) {
    if (!reg_used_table[i]) {
      reg_used_table[i] = 1;
      return i;
    }
  }
  Error("No more regs");
}

static void FreeReg(int reg) {
  assert(1 <= reg && reg <= NUM_OF_SCRATCH_REGS);
  reg_used_table[reg] = 0;
}

struct VarContext {
  struct VarContext *parent;
  struct Node *list;
};

struct VarContext *AllocVarContext(struct VarContext *parent) {
  struct VarContext *ctx = calloc(1, sizeof(struct VarContext));
  ctx->parent = parent;
  ctx->list = AllocList();
  return ctx;
}

static void AddLocalVar(struct VarContext *ctx, const char *key,
                        struct Node *var_type) {
  assert(ctx);
  int ofs = 0;
  if (GetSizeOfList(ctx->list)) {
    struct Node *n = GetNodeAt(ctx->list, GetSizeOfList(ctx->list) - 1);
    assert(n && n->type == kASTKeyValue);
    struct Node *v = n->value;
    assert(v && v->type == kASTLocalVar);
    ofs = v->byte_offset;
  }
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct Node *local_var = CreateASTLocalVar(ofs, var_type);
  PushKeyValueToList(ctx->list, key, local_var);
}

static struct Node *FindLocalVar(struct VarContext *ctx,
                                 struct Node *key_token) {
  struct Node *n = GetNodeByTokenKey(ctx->list, key_token);
  if (!n || n->type != kASTLocalVar) return NULL;
  return n;
}

static struct VarContext *var_context;

static void AnalyzeNode(struct Node *node) {
  assert(node);
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      AnalyzeNode(GetNodeAt(node, i));
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    node->reg = AllocReg();
    // TODO: support expe_type other than int
    node->expr_type = CreateTypeBase(CreateToken("int"));
    AnalyzeNode(node->func_expr);
    FreeReg(node->func_expr->reg);
    for (int i = 0; i < GetSizeOfList(node->arg_expr_list); i++) {
      struct Node *n = GetNodeAt(node->arg_expr_list, i);
      AnalyzeNode(n);
      FreeReg(n->reg);
    }
    return;
  } else if (node->type == kASTFuncDef) {
    AnalyzeNode(node->func_body);
    return;
  }
  assert(node->op);
  if (node->type == kASTExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber ||
        node->op->type == kTokenCharLiteral) {
      node->reg = AllocReg();
      node->expr_type = CreateTypeBase(CreateToken("int"));
      return;
    } else if (node->op->type == kTokenStringLiteral) {
      node->reg = AllocReg();
      node->expr_type = CreateTypePointer(CreateTypeBase(CreateToken("char")));
      return;
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      AnalyzeNode(node->right);
      node->reg = node->right->reg;
      node->expr_type = node->right->expr_type;
      return;
    } else if (node->op->type == kTokenIdent) {
      struct Node *ident_info = FindLocalVar(var_context, node->op);
      if (ident_info) {
        node->byte_offset = ident_info->byte_offset;
        node->reg = AllocReg();
        node->expr_type = CreateTypeLValue(ident_info->expr_type);
        return;
      }
      ident_info = GetNodeByTokenKey(toplevel_names, node->op);
      if (ident_info) {
        if (ident_info->type == kTypeFunction) {
          node->reg = AllocReg();
          node->expr_type = ident_info;
          return;
        }
      }
      ErrorWithToken(node->op, "Unknown identifier");
    } else if (node->cond) {
      AnalyzeNode(node->cond);
      AnalyzeNode(node->left);
      AnalyzeNode(node->right);
      FreeReg(node->left->reg);
      FreeReg(node->right->reg);
      assert(
          IsSameTypeExceptAttr(node->left->expr_type, node->right->expr_type));
      node->reg = node->cond->reg;
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    } else if (!node->left && node->right) {
      AnalyzeNode(node->right);
      if (node->op->type == kTokenKwSizeof) {
        node->reg = AllocReg();
        node->expr_type = CreateTypeBase(CreateToken("int"));
        return;
      }
      node->reg = node->right->reg;
      if (IsEqualTokenWithCStr(node->op, "&")) {
        node->expr_type =
            CreateTypePointer(GetRValueType(node->right->expr_type));
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "*")) {
        struct Node *rtype = GetRValueType(node->right->expr_type);
        assert(rtype && rtype->type == kTypePointer);
        node->expr_type = CreateTypeLValue(rtype->right);
        return;
      }
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    } else if (node->left && node->right) {
      AnalyzeNode(node->left);
      AnalyzeNode(node->right);
      if (IsEqualTokenWithCStr(node->op, "=") ||
          IsEqualTokenWithCStr(node->op, ",")) {
        FreeReg(node->left->reg);
        node->reg = node->right->reg;
        node->expr_type = GetRValueType(node->right->expr_type);
        return;
      }
      FreeReg(node->right->reg);
      node->reg = node->left->reg;
      node->expr_type = GetRValueType(node->left->expr_type);
      return;
    }
  }
  if (node->type == kASTExprStmt) {
    if (!node->left) return;
    AnalyzeNode(node->left);
    if (node->left->reg) FreeReg(node->left->reg);
    return;
  } else if (node->type == kASTList) {
    var_context = AllocVarContext(NULL);
    for (int i = 0; i < GetSizeOfList(node); i++) {
      AnalyzeNode(GetNodeAt(node, i));
    }
    return;
  } else if (node->type == kASTDecl) {
    struct Node *type = CreateType(node->op, node->right);
    assert(type && type->type == kTypeAttrIdent);
    AddLocalVar(var_context, CreateTokenStr(type->left), type->right);
    return;
  } else if (node->type == kASTJumpStmt) {
    if (node->op->type == kTokenKwReturn) {
      if (!node->right) return;
      AnalyzeNode(node->right);
      FreeReg(node->right->reg);
      return;
    }
  } else if (node->type == kASTSelectionStmt) {
    if (node->op->type == kTokenKwIf) {
      AnalyzeNode(node->cond);
      FreeReg(node->cond->reg);
      AnalyzeNode(node->left);
      assert(!node->right);
      return;
    }
  }
  ErrorWithToken(node->op, "AnalyzeNode: Not implemented");
}

void Analyze(struct Node *ast) { AnalyzeNode(ast); }
