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
  } else if (node->type == kASTConstant) {
    ASTConstant *val = ToASTConstant(node);
    switch (val->token->type) {
      case kInteger: {
        printf("int\n");
      } break;
      case kStringLiteral: {
        printf("char *\n");
      } break;
      default:
        Error("Not implemented Imm token type %d", val->token->type);
    }
  } else if (node->type == kASTIdent) {
    ASTNode *var = FindIdentInContext(context, ToASTIdent(node));
    if (!var || (var->type != kASTLocalVar)) {
      Error("Unknown identifier %s", ToASTIdent(node)->token->str);
    }
    ASTLocalVar *local_var = ToASTLocalVar(var);
    if (!local_var) {
      Error("var is not a local_var");
    }
  } else if (node->type == kASTForStmt) {
    // ASTForStmt *stmt = ToASTForStmt(node);
    // ASTLabel *begin_label = AllocASTLabel();
    // ASTLabel *end_label = AllocASTLabel();
    // Register *cond_result = AllocRegister();

    ASTLabel *org_break_label = GetBreakLabelInContext(context);
    // SetBreakLabelInContext(context, end_label);

    // AnalyzeNode(stmt->init_expr, context);
    // AnalyzeNode(stmt->cond_expr, context);
    // AnalyzeNode(stmt->body_stmt, context);
    // AnalyzeNode(stmt->updt_expr, context);

    SetBreakLabelInContext(context, org_break_label);
  } else {
    PrintASTNode(node, 0);
    putchar('\n');
    Warning("Analyzing AST%s is not implemented.", GetASTTypeName(node));
  }
}

void Analyze(ASTNode *root) {
  Context *context = AllocContext(NULL);
  AnalyzeNode(root, context);
}
