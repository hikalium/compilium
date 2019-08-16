#include "compilium.h"

static int reg_used_table[NUM_OF_SCRATCH_REGS];
static struct Node *reg_node_table[NUM_OF_SCRATCH_REGS];

static void AllocReg(struct Node *n) {
  assert(n);
  for (int i = 1; i <= NUM_OF_SCRATCH_REGS; i++) {
    if (!reg_used_table[i]) {
      reg_used_table[i] = 1;
      reg_node_table[i] = n;
      n->reg = i;
      return;
    }
  }
  fprintf(stderr, "\n**** Allocated regs ****\n");
  for (int i = 1; i <= NUM_OF_SCRATCH_REGS; i++) {
    fprintf(stderr, "reg[%d]:\n", i);
    if (reg_node_table[i]->op) {
      PrintTokenLine(reg_node_table[i]->op);
    } else {
      fprintf(stderr, "Op info not found\n");
    }
  }
  fprintf(stderr, "\n**** Tried to allocate reg for ****\n");
  PrintASTNode(n);
  ErrorWithToken(n->op, "No free registers found");
}

static void FreeReg(int reg) {
  assert(1 <= reg && reg <= NUM_OF_SCRATCH_REGS);
  reg_used_table[reg] = 0;
  reg_node_table[reg] = NULL;
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
    AllocReg(node);
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
    AddFuncDef(ctx, CreateTokenStr(node->func_name_token), node);
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
      AllocReg(node);
      node->expr_type = CreateTypeBase(CreateToken("int"));
      return;
    } else if (IsTokenWithType(node->op, kTokenStringLiteral)) {
      AllocReg(node);
      node->expr_type = CreateTypePointer(CreateTypeBase(CreateToken("char")));
      return;
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      AnalyzeNode(node->right, ctx);
      node->reg = node->right->reg;
      node->expr_type = node->right->expr_type;
      return;
    } else if (IsEqualTokenWithCStr(node->op, "[")) {
      AnalyzeNode(node->left, ctx);
      AnalyzeNode(node->right, ctx);
      node->reg = node->left->reg;
      FreeReg(node->right->reg);
      node->expr_type = CreateTypeLValue(
          GetTypeWithoutAttr(node->left->expr_type)->type_array_type_of);
      return;
    } else if (IsEqualTokenWithCStr(node->op, ".") ||
               IsEqualTokenWithCStr(node->op, "->")) {
      AnalyzeNode(node->left, ctx);
      node->reg = node->left->reg;
      PrintASTNode(node->left->expr_type);
      assert(node->right && node->right->type == kNodeToken);
      if (IsEqualTokenWithCStr(node->op, ".")) {
        if (GetTypeWithoutAttr(node->left->expr_type)->type != kTypeStruct)
          ErrorWithToken(node->op, "left operand is not a struct");
        struct Node *member =
            FindStructMember(node->left->expr_type, node->right);
        PrintASTNode(member);
        node->byte_offset = member->struct_member_ent_ofs;
        node->expr_type = CreateTypeLValue(
            GetTypeWithoutAttr(member->struct_member_ent_type));
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "->")) {
        struct Node *left_type = GetTypeWithoutAttr(node->left->expr_type);
        PrintASTNode(left_type);
        assert(left_type->type == kTypePointer);
        struct Node *left_deref_type = left_type->right;
        assert(left_deref_type->type == kTypeStruct);
        struct Node *member = FindStructMember(left_deref_type, node->right);
        PrintASTNode(member);
        node->byte_offset = member->struct_member_ent_ofs;
        node->expr_type = CreateTypeLValue(
            GetTypeWithoutAttr(member->struct_member_ent_type));
        return;
      }
      assert(false);
    } else if (IsTokenWithType(node->op, kTokenIdent)) {
      struct Node *ident_info = FindLocalVar(*ctx, node->op);
      if (ident_info) {
        node->byte_offset = ident_info->byte_offset;
        AllocReg(node);
        enum NodeType expr_type =
            GetTypeWithoutAttr(ident_info->expr_type)->type;
        if (expr_type == kTypeStruct || expr_type == kTypeArray) {
          node->expr_type = ident_info->expr_type;
          return;
        }
        node->expr_type = CreateTypeLValue(ident_info->expr_type);
        return;
      }
      struct Node *func_def = FindFuncDef(*ctx, node->op);
      if (func_def) {
        AllocReg(node);
        node->expr_type = func_def->func_type;
        return;
      }
      struct Node *func_decl_type = FindFuncDeclType(*ctx, node->op);
      if (func_decl_type) {
        AllocReg(node);
        node->expr_type = GetTypeWithoutAttr(func_decl_type);
        return;
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
        FreeReg(node->right->reg);
        AllocReg(node);
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
    } else if (node->left && !node->right) {
      if (IsEqualTokenWithCStr(node->op, "++")) {
        AnalyzeNode(node->left, ctx);
        assert(IsLValueType(node->left->expr_type));
        node->reg = node->left->reg;
        node->expr_type = GetRValueType(node->left->expr_type);
        return;
      }
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
    assert(false);
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
    struct Node *raw_type = CreateTypeInContext(*ctx, node->op, node->right);
    PrintASTNode(raw_type);
    assert(raw_type);
    struct Node *type_ident = NULL;
    if (raw_type && raw_type->type == kTypeAttrIdent) {
      type_ident = raw_type->left;
    }
    struct Node *type = GetTypeWithoutAttr(raw_type);
    assert(type);

    if (type_ident && type->type == kTypeFunction) {
      AddFuncDeclType(ctx, CreateTokenStr(type_ident), raw_type);
      return;
    }
    if (!type_ident && type->type == kTypeStruct) {
      struct Node *spec = type->type_struct_spec;
      ResolveTypesOfMembersOfStruct(*ctx, spec);
      assert(type->tag);
      AddStructType(ctx, CreateTokenStr(type->tag), type);
      return;
    }
    assert(type && type_ident);
    AddLocalVar(ctx, CreateTokenStr(type_ident), type);
    assert(node->right->type == kASTDecltor);
    if (node->right->decltor_init_expr) {
      struct Node *left_expr = AllocNode(kASTExpr);
      left_expr->op = type_ident;
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
