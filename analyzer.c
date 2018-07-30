#include "compilium.h"

void AnalyzeNode(ASTNode *node, Context *context) {
  printf("Analyzing AST%s...\n", GetASTTypeName(node));
  if (node->type == kASTList) {
    // translation-unit
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kASTFuncDef) {
        AnalyzeNode(child_node, context);
      }
    }
  } else if (node->type == kASTFuncDef) {
    ASTFuncDef *def = ToASTFuncDef(node);

    context = AllocContext(context);
    def->context = context;
    ASTDirectDecltor *args_decltor = def->decltor->direct_decltor;
    ASTList *param_decl_list = ToASTList(args_decltor->data);
    if (param_decl_list) {
      PrintASTNode(ToASTNode(param_decl_list), 0);
      for (int i = 0; i < GetSizeOfASTList(param_decl_list); i++) {
        ASTParamDecl *param_decl =
            ToASTParamDecl(GetASTNodeAt(param_decl_list, i));
        ASTDecltor *param_decltor = ToASTDecltor(param_decl->decltor);
        ASTLocalVar *local_var = AppendLocalVarInContext(
            context, param_decl->decl_specs, param_decltor);
        SetASTNodeAt(param_decl_list, i, ToASTNode(local_var));
      }
    }
    AnalyzeNode(ToASTNode(def->comp_stmt), context);
  } else if (node->type == kASTCompStmt) {
    ASTCompStmt *comp = ToASTCompStmt(node);
    ASTList *stmt_list = comp->stmt_list;
    for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
      AnalyzeNode(GetASTNodeAt(stmt_list, i), context);
    }
  } else if (node->type == kASTDecl) {
    ASTDecl *decl = ToASTDecl(node);
    if (!decl) Error("node is not a Decl");
    for (int i = 0; i < GetSizeOfASTList(decl->init_decltors); i++) {
      ASTDecltor *decltor = ToASTDecltor(GetASTNodeAt(decl->init_decltors, i));
      AppendLocalVarInContext(context, decl->decl_specs, decltor);
    }
  } else if (node->type == kASTExprStmt) {
    const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
    if (!expr_stmt->expr) return;  // expr is opt
    AnalyzeNode(expr_stmt->expr, context);
  } else if (node->type == kASTExprBinOp) {
    ASTExprBinOp *bin_op = ToASTExprBinOp(node);
    AnalyzeNode(bin_op->left, context);
    AnalyzeNode(bin_op->right, context);
  } else if (node->type == kASTInteger) {
    printf("int\n");
  } else if (node->type == kASTString) {
    printf("char *\n");
  } else if (node->type == kASTIdent) {
    ASTIdent *ident = ToASTIdent(node);
    ASTNode *var = FindIdentInContext(context, ident);
    if (!var || (var->type != kASTLocalVar)) {
      // TODO: Add func ident check
      return;
    }
    ident->local_var = ToASTLocalVar(var);
  } else if (node->type == kASTForStmt) {
    ASTForStmt *stmt = ToASTForStmt(node);

    stmt->begin_label = AllocASTLabel();
    stmt->end_label = AllocASTLabel();

    ASTLabel *org_break_label = GetBreakLabelInContext(context);
    SetBreakLabelInContext(context, stmt->end_label);

    AnalyzeNode(stmt->init_expr, context);
    AnalyzeNode(stmt->cond_expr, context);
    AnalyzeNode(stmt->body_stmt, context);
    AnalyzeNode(stmt->updt_expr, context);

    SetBreakLabelInContext(context, org_break_label);
  } else if (node->type == kASTJumpStmt) {
    ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
    if (IsEqualToken(jump_stmt->kw->token, "return")) {
      if (jump_stmt->param) {
        AnalyzeNode(jump_stmt->param, context);
      }
      // TODO: Impl return type check
    } else if (IsEqualToken(jump_stmt->kw->token, "break")) {
      ASTLabel *break_label = GetBreakLabelInContext(context);
      if (!break_label) {
        Error("break-stmt should be used within iteration-stmt");
      }
      jump_stmt->param = ToASTNode(break_label);
    } else {
      Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
    }
  } else if (node->type == kASTIfStmt) {
    ASTIfStmt *if_stmt = ToASTIfStmt(node);
    AnalyzeNode(if_stmt->cond_expr, context);
    AnalyzeNode(if_stmt->true_stmt, context);
    if (if_stmt->false_stmt) AnalyzeNode(if_stmt->false_stmt, context);
  } else if (node->type == kASTExprFuncCall) {
    ASTExprFuncCall *expr_func_call = ToASTExprFuncCall(node);
    // TODO: support func pointer
    if (expr_func_call->func->type != kASTIdent) {
      Error("Calling non-labeled function is not implemented.");
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
  } else if (node->type == kASTExprUnaryPreOp) {
    ASTExprUnaryPreOp *op = ToASTExprUnaryPreOp(node);
    AnalyzeNode(op->expr, context);
  } else if (node->type == kASTExprUnaryPostOp) {
    ASTExprUnaryPostOp *op = ToASTExprUnaryPostOp(node);
    AnalyzeNode(op->expr, context);
  } else if (node->type == kASTCondStmt) {
    ASTCondStmt *cond_stmt = ToASTCondStmt(node);
    AnalyzeNode(cond_stmt->cond_expr, context);
    AnalyzeNode(cond_stmt->true_expr, context);
    AnalyzeNode(cond_stmt->false_expr, context);
  } else if (node->type == kASTWhileStmt) {
    ASTWhileStmt *stmt = ToASTWhileStmt(node);
    stmt->begin_label = AllocASTLabel();
    stmt->end_label = AllocASTLabel();

    ASTLabel *org_break_label = GetBreakLabelInContext(context);
    SetBreakLabelInContext(context, stmt->end_label);

    AnalyzeNode(stmt->cond_expr, context);
    AnalyzeNode(stmt->body_stmt, context);

    SetBreakLabelInContext(context, org_break_label);
  } else {
    PrintASTNode(node, 0);
    putchar('\n');
    Error("Analyzing AST%s is not implemented.", GetASTTypeName(node));
  }
}

void Analyze(ASTNode *root) {
  Context *context = AllocContext(NULL);
  AnalyzeNode(root, context);
}
