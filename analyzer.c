#include "compilium.h"

static ASTType *AnalyzeNode(ASTNode *node, Context *context) {
  printf("Analyzing AST%s...\n", GetASTNodeTypeName(node));
  if (node->type == kASTList) {
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      AnalyzeNode(child_node, context);
    }
    return NULL;
  } else if (node->type == kASTFuncDef) {
    ASTFuncDef *def = ToASTFuncDef(node);
    def->func_type = AllocAndInitASTType(def->decl_specs, def->decltor);
    AppendTypeToContext(identifiers, GetIdentTokenOfType(def->func_type)->str,
                        def->func_type);
    context = AllocContext(context);
    def->context = context;
    ASTDirectDecltor *args_decltor = def->decltor->direct_decltor;
    ASTList *param_decl_list = ToASTList(args_decltor->data);
    if (param_decl_list) {
      for (int i = 0; i < GetSizeOfASTList(param_decl_list); i++) {
        ASTNode *param_node = GetASTNodeAt(param_decl_list, i);
        if (param_node->type == kASTKeyword) {
          if (!IsEqualToken(ToASTKeyword(param_node)->token, "..."))
            ErrorWithASTNode(param_node, "Unexpected node in param_decl_list");
          if (i != GetSizeOfASTList(param_decl_list) - 1)
            ErrorWithASTNode(param_node,
                             "... can be appeared only at end of arguments");
          def->has_variable_length_args = 1;
          break;
        }
        ASTParamDecl *param_decl = ToASTParamDecl(param_node);
        assert(param_decl);
        ASTKeyword *first_decl_spec_kw =
            ToASTKeyword(GetASTNodeAt(param_decl->decl_specs, 0));
        if (first_decl_spec_kw &&
            IsEqualToken(first_decl_spec_kw->token, "void")) {
          if (i != 0 || GetSizeOfASTList(param_decl->decl_specs) != 1)
            Error("func args should be (void)");
          break;
        }
        ASTDecltor *param_decltor = ToASTDecltor(param_decl->decltor);
        ASTVar *local_var = AppendLocalVarToContext(
            context, param_decl->decl_specs, param_decltor);
        SetASTNodeAt(param_decl_list, i, ToASTNode(local_var));
        local_var->arg_index = i + 1;
      }
    }
    AnalyzeNode(ToASTNode(def->comp_stmt), context);
    PrintContext(def->context);
    return NULL;
  } else if (node->type == kASTCompStmt) {
    ASTCompStmt *comp = ToASTCompStmt(node);
    ASTList *stmt_list = comp->stmt_list;
    for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
      AnalyzeNode(GetASTNodeAt(stmt_list, i), context);
    }
    return NULL;
  } else if (node->type == kASTDecl) {
    ASTDecl *decl = ToASTDecl(node);
    if (IsTypedefDeclSpecs(decl->decl_specs)) return NULL;
    if (!decl->init_decltors) {
      // Declaration only
      ASTType *type = AllocAndInitASTType(decl->decl_specs, NULL);
      if (IsBasicType(type, kTypeStruct)) {
        AppendTypeToContext(struct_names, GetStructTagFromType(type), type);
        PrintContext(struct_names);
      }
      return NULL;
    }
    for (int i = 0; i < GetSizeOfASTList(decl->init_decltors); i++) {
      ASTDecltor *decltor = ToASTDecltor(GetASTNodeAt(decl->init_decltors, i));
      ASTType *type = AllocAndInitASTType(decl->decl_specs, decltor);
      if (IsRootContext(context)) {
        if (IsBasicType(type, kTypeFunction)) {
          AppendTypeToContext(identifiers, GetIdentTokenOfType(type)->str,
                              type);
          continue;
        }
        ASTIdent *ident = GetIdentFromDecltor(decltor);
        ident->local_var =
            AppendGlobalVarToContext(context, decl->decl_specs, decltor);
        ident->local_var->is_external = IsExternDeclSpecs(decl->decl_specs);
      } else {
        if (IsBasicType(type, kTypeFunction))
          Error("Function declaration is not allowed here");
        AppendLocalVarToContext(context, decl->decl_specs, decltor);
        if (decltor->initializer)
          AnalyzeNode(ToASTNode(decltor->initializer), context);
      }
    }
    return NULL;
  } else if (node->type == kASTExprStmt) {
    const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
    if (!expr_stmt->expr) return NULL;  // expr is opt
    AnalyzeNode(expr_stmt->expr, context);
    return NULL;
  } else if (node->type == kASTExprBinOp) {
    ASTExprBinOp *bin_op = ToASTExprBinOp(node);
    ASTType *left_type = AnalyzeNode(bin_op->left, context);
    ASTType *right_type = AnalyzeNode(bin_op->right, context);
    if (IsEqualToken(bin_op->op, "=")) {
      if (!IsBasicType(left_type, kTypeLValueOf)) {
        ErrorWithASTNode(node, "left operand should be an lvalue");
      }
      bin_op->expr_type = GetRValueTypeOf(left_type);
      return bin_op->expr_type;
    } else if (IsEqualToken(bin_op->op, ".")) {
      ASTType *left_type = AnalyzeNode(bin_op->left, context);
      Context *struct_members = GetStructContextFromType(left_type);
      ASTVar *member = ToASTVar(
          FindIdentInContext(struct_members, ToASTIdent(bin_op->right)));
      if (!member)
        ErrorWithASTNode(bin_op, "No member named %s found",
                         ToASTIdent(bin_op->right)->token->str);
      bin_op->expr_type = AllocAndInitASTTypeLValueOf(member->var_type);
      return bin_op->expr_type;
    } else if (IsEqualToken(bin_op->op, "->")) {
      ASTType *left_type = AnalyzeNode(bin_op->left, context);
      if (!left_type)
        ErrorWithASTNode(bin_op, "left operand of -> has null type");
      ASTType *left_deref_type = GetDereferencedTypeOf(left_type);
      Context *struct_members = GetStructContextFromType(left_deref_type);
      ASTVar *member = ToASTVar(
          FindIdentInContext(struct_members, ToASTIdent(bin_op->right)));
      if (!member)
        ErrorWithASTNode(bin_op, "No member named %s found",
                         ToASTIdent(bin_op->right)->token->str);
      bin_op->expr_type = AllocAndInitASTTypeLValueOf(member->var_type);
      return bin_op->expr_type;
    } else if (IsEqualToken(bin_op->op, "&&") ||
               IsEqualToken(bin_op->op, "||")) {
      return AllocAndInitBasicType(kTypeInt);
    }
    left_type = GetRValueTypeOf(left_type);
    right_type = GetRValueTypeOf(right_type);
    if (IsBasicType(left_type, kTypePointerOf)) {
      if (!IsBasicType(right_type, kTypeInt))
        ErrorWithASTNode(node, "right operand should be an int");
      bin_op->expr_type = left_type;
    } else if (IsBasicType(right_type, kTypePointerOf)) {
      if (!IsBasicType(left_type, kTypeInt))
        ErrorWithASTNode(node, "left operand should be an int");
      bin_op->expr_type = right_type;
    } else if (IsBasicType(left_type, kTypeArrayOf)) {
      if (!IsBasicType(right_type, kTypeInt))
        ErrorWithASTNode(node, "right operand should be an int");
      bin_op->expr_type = ConvertFromArrayToPointer(left_type);
    } else if (IsBasicType(right_type, kTypeArrayOf)) {
      if (!IsBasicType(left_type, kTypeInt))
        ErrorWithASTNode(node, "left operand should be an int");
      bin_op->expr_type = ConvertFromArrayToPointer(right_type);
    } else if (IsEqualASTType(left_type, right_type)) {
      bin_op->expr_type = left_type;
    } else {
      PrintASTType(left_type);
      PrintASTType(right_type);
      ErrorWithASTNode(node, "Type check failed");
    }
    return bin_op->expr_type;
  } else if (node->type == kASTInteger) {
    return AllocAndInitBasicType(kTypeInt);
  } else if (node->type == kASTString) {
    return AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeChar));
  } else if (node->type == kASTIdent) {
    ASTIdent *ident = ToASTIdent(node);
    ASTNode *var = FindIdentInContext(context, ident);
    if (var && (var->type == kASTVar)) {
      ident->local_var = ToASTVar(var);
      ident->var_type = AllocAndInitASTTypeLValueOf(ident->local_var->var_type);
      return ident->var_type;
    }
    ASTNode *ident_info = FindIdentInContext(identifiers, ident);
    if (!ident_info) return NULL;
    if (ident_info->type == kASTInteger) {
      ident->var_type = AllocAndInitBasicType(kTypeInt);
      return ident->var_type;
    }
    ErrorWithASTNode(node, "Unknown identifier");
  } else if (node->type == kASTForStmt) {
    ASTForStmt *stmt = ToASTForStmt(node);

    stmt->begin_label = AllocASTLabel();
    stmt->end_label = AllocASTLabel();
    stmt->continue_label = AllocASTLabel();

    ASTLabel *org_break_label = GetBreakLabelInContext(context);
    ASTLabel *org_continue_label = GetContinueLabelInContext(context);
    SetBreakLabelInContext(context, stmt->end_label);
    SetContinueLabelInContext(context, stmt->continue_label);

    AnalyzeNode(stmt->init_expr, context);
    AnalyzeNode(stmt->cond_expr, context);
    AnalyzeNode(stmt->body_stmt, context);
    AnalyzeNode(stmt->updt_expr, context);

    SetContinueLabelInContext(context, org_continue_label);
    SetBreakLabelInContext(context, org_break_label);
    return NULL;
  } else if (node->type == kASTJumpStmt) {
    ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
    if (IsEqualToken(jump_stmt->kw->token, "return")) {
      if (jump_stmt->param) {
        AnalyzeNode(jump_stmt->param, context);
      }
      // TODO: Impl return type check
      return NULL;
    } else if (IsEqualToken(jump_stmt->kw->token, "break")) {
      ASTLabel *break_label = GetBreakLabelInContext(context);
      if (!break_label)
        Error("break-stmt should be used within iteration-stmt");
      jump_stmt->param = ToASTNode(break_label);
      return NULL;
    } else if (IsEqualToken(jump_stmt->kw->token, "continue")) {
      ASTLabel *continue_label = GetContinueLabelInContext(context);
      if (!continue_label)
        Error("continue-stmt should be used within iteration-stmt");
      jump_stmt->param = ToASTNode(continue_label);
      return NULL;
    }
    Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
  } else if (node->type == kASTIfStmt) {
    ASTIfStmt *if_stmt = ToASTIfStmt(node);
    AnalyzeNode(if_stmt->cond_expr, context);
    AnalyzeNode(if_stmt->true_stmt, context);
    if (if_stmt->false_stmt) AnalyzeNode(if_stmt->false_stmt, context);
    return NULL;
  } else if (node->type == kASTExprFuncCall) {
    ASTExprFuncCall *expr_func_call = ToASTExprFuncCall(node);
    if (IsEqualToken(expr_func_call->func_ident->token, "__builtin_va_start")) {
      ASTList *arg_list = ToASTList(expr_func_call->args);
      if (!arg_list || GetSizeOfASTList(arg_list) != 2)
        Error("__builtin_va_start requires 2 args");
      AnalyzeNode(GetASTNodeAt(arg_list, 0), context);
      AnalyzeNode(GetASTNodeAt(arg_list, 1), context);
      expr_func_call->func_type =
          AllocAndInitASTTypeFunction(AllocAndInitBasicType(kTypeVoid));
      return expr_func_call->func_type;
    };
    expr_func_call->func_type =
        ToASTType(FindIdentInContext(identifiers, expr_func_call->func_ident));
    if (!expr_func_call->func_type) {
      ErrorWithASTNode(node, "Undefined function");
    }
    if (expr_func_call->args) {
      ASTList *arg_list = ToASTList(expr_func_call->args);
      if (!arg_list) Error("arg_list is not an ASTList");
      if (GetSizeOfASTList(arg_list) > 8) Error("Too many params");
      for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
        ASTNode *arg_node = GetASTNodeAt(arg_list, i);
        AnalyzeNode(arg_node, context);
      }
    }
    return GetReturningTypeFromFunctionType(expr_func_call->func_type);
  } else if (node->type == kASTExprUnaryPreOp) {
    ASTExprUnaryPreOp *op = ToASTExprUnaryPreOp(node);
    if (op->expr && op->expr->type != kASTType) {
      op->expr_type = AnalyzeNode(op->expr, context);
    }
    if (IsEqualToken(op->op, "*")) {
      op->expr_type = GetDereferencedTypeOf(op->expr_type);
    } else if (IsEqualToken(op->op, "&")) {
      op->expr_type =
          AllocAndInitASTTypePointerOf(GetRValueTypeOf(op->expr_type));
    } else if (IsEqualToken(op->op, "sizeof")) {
      op->expr_type = AllocAndInitBasicType(kTypeInt);
    } else {
      op->expr_type = GetRValueTypeOf(op->expr_type);
    }
    return op->expr_type;
  } else if (node->type == kASTExprUnaryPostOp) {
    ASTExprUnaryPostOp *op = ToASTExprUnaryPostOp(node);
    ASTType *expr_type = AnalyzeNode(op->expr, context);
    op->expr_type = GetRValueTypeOf(expr_type);
    return op->expr_type;
  } else if (node->type == kASTCondStmt) {
    ASTCondStmt *cond_stmt = ToASTCondStmt(node);
    AnalyzeNode(cond_stmt->cond_expr, context);
    ASTType *true_expr_type = AnalyzeNode(cond_stmt->true_expr, context);
    // ASTType *false_expr_type = AnalyzeNode(cond_stmt->false_expr, context);
    // if (!IsEqualASTType(true_expr_type, false_expr_type))
    //  ErrorWithASTNode(node, "expressions of condition-statement should have
    //  same types.");
    cond_stmt->expr_type = GetRValueTypeOf(true_expr_type);
    return cond_stmt->expr_type;
  } else if (node->type == kASTWhileStmt) {
    ASTWhileStmt *stmt = ToASTWhileStmt(node);
    stmt->begin_label = AllocASTLabel();
    stmt->end_label = AllocASTLabel();

    ASTLabel *org_break_label = GetBreakLabelInContext(context);
    ASTLabel *org_continue_label = GetContinueLabelInContext(context);
    SetBreakLabelInContext(context, stmt->end_label);
    SetContinueLabelInContext(context, stmt->begin_label);

    AnalyzeNode(stmt->cond_expr, context);
    AnalyzeNode(stmt->body_stmt, context);

    SetContinueLabelInContext(context, org_continue_label);
    SetBreakLabelInContext(context, org_break_label);
    return NULL;
  } else if (node->type == kASTExprCast) {
    ASTExprCast *cast_expr = ToASTExprCast(node);
    AnalyzeNode(cast_expr->expr, context);
    return cast_expr->to_expr_type;
  }
  ErrorWithASTNode(node, "Analyzing AST%s is not implemented.",
                   GetASTNodeTypeName(node));
}

void Analyze(ASTNode *root) {
  Context *context = AllocContext(NULL);
  AnalyzeNode(root, context);

  puts("\nAST after Analyze:");
  DebugPrintASTNode(root);
  puts("\nidentifiers after Analyze:");
  PrintContext(identifiers);
}
