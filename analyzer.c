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

static void AddLocalVar(struct Node *list, const char *key,
                        struct Node *var_type) {
  int ofs = 0;
  if (GetSizeOfList(list)) {
    struct Node *n = GetNodeAt(list, GetSizeOfList(list) - 1);
    assert(n && n->type == kASTKeyValue);
    struct Node *v = n->value;
    assert(v && v->type == kASTLocalVar);
    ofs = v->byte_offset;
  }
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct Node *local_var = CreateASTLocalVar(ofs, var_type);
  PushKeyValueToList(list, key, local_var);
}

static struct Node *var_context;

void Analyze(struct Node *node) {
  assert(node);
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Analyze(GetNodeAt(node, i));
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    node->reg = AllocReg();
    // TODO: support expe_type other than int
    node->expr_type = CreateTypeBase(CreateToken("int"));
    Analyze(node->func_expr);
    FreeReg(node->func_expr->reg);
    for (int i = 0; i < GetSizeOfList(node->arg_expr_list); i++) {
      struct Node *n = GetNodeAt(node->arg_expr_list, i);
      Analyze(n);
      FreeReg(n->reg);
    }
    return;
  } else if (node->type == kASTFuncDef) {
    Analyze(node->func_body);
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
      Analyze(node->right);
      node->reg = node->right->reg;
      node->expr_type = node->right->expr_type;
      return;
    } else if (node->op->type == kTokenIdent) {
      struct Node *ident_info = GetNodeByTokenKey(var_context, node->op);
      if (ident_info) {
        if (ident_info->type == kASTLocalVar) {
          node->byte_offset = ident_info->byte_offset;
          node->reg = AllocReg();
          node->expr_type = CreateTypeLValue(ident_info->expr_type);
          return;
        }
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
      Analyze(node->cond);
      Analyze(node->left);
      Analyze(node->right);
      FreeReg(node->left->reg);
      FreeReg(node->right->reg);
      assert(
          IsSameTypeExceptAttr(node->left->expr_type, node->right->expr_type));
      node->reg = node->cond->reg;
      node->expr_type = GetRValueType(node->right->expr_type);
      return;
    } else if (!node->left && node->right) {
      Analyze(node->right);
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
      Analyze(node->left);
      Analyze(node->right);
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
    Analyze(node->left);
    if (node->left->reg) FreeReg(node->left->reg);
    return;
  } else if (node->type == kASTList) {
    var_context = AllocList();
    for (int i = 0; i < GetSizeOfList(node); i++) {
      Analyze(GetNodeAt(node, i));
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
      Analyze(node->right);
      FreeReg(node->right->reg);
      return;
    }
  } else if (node->type == kASTSelectionStmt) {
    if (node->op->type == kTokenKwIf) {
      Analyze(node->cond);
      FreeReg(node->cond->reg);
      Analyze(node->left);
      assert(!node->right);
      return;
    }
  }
  ErrorWithToken(node->op, "Analyze: Not implemented");
}
