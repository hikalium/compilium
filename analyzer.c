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

struct SymbolEntry {
  struct SymbolEntry *prev;
  const char *key;
  struct Node *value;
};

static struct SymbolEntry *AllocSymbolEntry(struct SymbolEntry *prev,
                                            const char *key,
                                            struct Node *value) {
  struct SymbolEntry *e = calloc(1, sizeof(struct SymbolEntry));
  e->prev = prev;
  e->key = key;
  e->value = value;
  return e;
}

static int GetLastLocalVarOffset(struct SymbolEntry *last) {
  if (!last) return 0;
  assert(last->value && last->value->type == kASTLocalVar);
  return last->value->byte_offset;
}

static struct Node *AddLocalVar(struct SymbolEntry **ctx, const char *key,
                                struct Node *var_type) {
  assert(ctx);
  int ofs = GetLastLocalVarOffset(*ctx);
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct Node *local_var = CreateASTLocalVar(ofs, var_type);
  struct SymbolEntry *e = AllocSymbolEntry(*ctx, key, local_var);
  *ctx = e;
  return local_var;
}

static struct Node *FindLocalVar(struct SymbolEntry *e,
                                 struct Node *key_token) {
  while (e) {
    if (IsEqualTokenWithCStr(key_token, e->key)) {
      return e->value;
    }
    e = e->prev;
  }
  return NULL;
}

static void AnalyzeNode(struct Node *node, struct SymbolEntry **ctx) {
  assert(node);
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      AnalyzeNode(GetNodeAt(node, i), ctx);
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    node->stack_size_needed = (GetLastLocalVarOffset(*ctx) + 0xF) & ~0xF;
    node->reg = AllocReg();
    // TODO: support expe_type other than int
    node->expr_type = CreateTypeBase(CreateToken("int"));
    AnalyzeNode(node->func_expr, ctx);
    FreeReg(node->func_expr->reg);
    for (int i = 0; i < GetSizeOfList(node->arg_expr_list); i++) {
      struct Node *n = GetNodeAt(node->arg_expr_list, i);
      AnalyzeNode(n, ctx);
      FreeReg(n->reg);
    }
    return;
  } else if (node->type == kASTFuncDef) {
    struct SymbolEntry *saved_ctx = *ctx;
    struct Node *arg_type_list = GetArgTypeList(node->func_type);
    assert(arg_type_list);
    node->arg_var_list = AllocList();
    for (int i = 0; i < GetSizeOfList(arg_type_list); i++) {
      struct Node *arg_type_with_attr = GetNodeAt(arg_type_list, i);
      struct Node *arg_ident_token =
          GetIdentifierTokenFromTypeAttr(arg_type_with_attr);
      if (!arg_ident_token) {
        PushToList(node->arg_var_list, NULL);
        continue;
      }
      struct Node *arg_type = GetTypeWithoutAttr(arg_type_with_attr);
      assert(arg_type);
      struct Node *local_var =
          AddLocalVar(ctx, CreateTokenStr(arg_ident_token), arg_type);
      PushToList(node->arg_var_list, local_var);
    }
    AnalyzeNode(node->func_body, ctx);
    *ctx = saved_ctx;
    return;
  }
  assert(node->op);
  if (node->type == kASTExpr) {
    if (IsTokenWithType(node->op, kTokenDecimalNumber) ||
        IsTokenWithType(node->op, kTokenOctalNumber) ||
        IsTokenWithType(node->op, kTokenCharLiteral)) {
      node->reg = AllocReg();
      node->expr_type = CreateTypeBase(CreateToken("int"));
      return;
    } else if (IsTokenWithType(node->op, kTokenStringLiteral)) {
      node->reg = AllocReg();
      node->expr_type = CreateTypePointer(CreateTypeBase(CreateToken("char")));
      return;
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      AnalyzeNode(node->right, ctx);
      node->reg = node->right->reg;
      node->expr_type = node->right->expr_type;
      return;
    } else if (IsTokenWithType(node->op, kTokenIdent)) {
      struct Node *ident_info = FindLocalVar(*ctx, node->op);
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
      AnalyzeNode(node->cond, ctx);
      AnalyzeNode(node->left, ctx);
      AnalyzeNode(node->right, ctx);
      FreeReg(node->left->reg);
      FreeReg(node->right->reg);
      assert(
          IsSameTypeExceptAttr(node->left->expr_type, node->right->expr_type));
      node->reg = node->cond->reg;
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    } else if (!node->left && node->right) {
      AnalyzeNode(node->right, ctx);
      if (IsTokenWithType(node->op, kTokenKwSizeof)) {
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
      AnalyzeNode(node->left, ctx);
      AnalyzeNode(node->right, ctx);
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
    AnalyzeNode(node->left, ctx);
    if (node->left->reg) FreeReg(node->left->reg);
    return;
  } else if (node->type == kASTList) {
    struct SymbolEntry *saved_ctx = *ctx;
    for (int i = 0; i < GetSizeOfList(node); i++) {
      AnalyzeNode(GetNodeAt(node, i), ctx);
    }
    *ctx = saved_ctx;
    return;
  } else if (node->type == kASTDecl) {
    struct Node *type = CreateType(node->op, node->right);
    assert(type && type->type == kTypeAttrIdent);
    AddLocalVar(ctx, CreateTokenStr(type->left), type->right);
    assert(node->right->type == kASTDecltor);
    if (node->right->decltor_init_expr) {
      struct Node *left_expr = AllocNode(kASTExpr);
      left_expr->op = type->left;
      node->right->decltor_init_expr->left = left_expr;
      AnalyzeNode(node->right->decltor_init_expr, ctx);
      FreeReg(node->right->decltor_init_expr->reg);
    }
    return;
  } else if (node->type == kASTJumpStmt) {
    if (IsTokenWithType(node->op, kTokenKwReturn)) {
      if (!node->right) return;
      AnalyzeNode(node->right, ctx);
      FreeReg(node->right->reg);
      return;
    }
  } else if (node->type == kASTSelectionStmt) {
    if (IsTokenWithType(node->op, kTokenKwIf)) {
      AnalyzeNode(node->cond, ctx);
      FreeReg(node->cond->reg);
      AnalyzeNode(node->if_true_stmt, ctx);
      if (node->if_else_stmt) {
        AnalyzeNode(node->if_else_stmt, ctx);
      }
      return;
    }
  } else if (node->type == kASTForStmt) {
    AnalyzeNode(node->init, ctx);
    FreeReg(node->init->reg);
    AnalyzeNode(node->cond, ctx);
    FreeReg(node->cond->reg);
    AnalyzeNode(node->updt, ctx);
    FreeReg(node->updt->reg);
    AnalyzeNode(node->body, ctx);
    return;
  }
  ErrorWithToken(node->op, "AnalyzeNode: Not implemented");
}

void Analyze(struct Node *ast) {
  struct SymbolEntry *root_ctx = NULL;
  AnalyzeNode(ast, &root_ctx);
}
