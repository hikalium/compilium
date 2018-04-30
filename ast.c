#include "compilium.h"

ASTNode *AllocateASTNode(ASTType type)
{
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = type;
  return node;
}

void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
}

void PrintASTNodeList(ASTNodeList *list, int depth);
void PrintASTNode(const ASTNode *node, int depth) {
  if(!node) {
    printf("(Null)");
    return;
  }
  if (node->type == kInclude) {
    printf("(Include:");
    PrintASTNodePadding(depth);
    PrintTokenList(node->data.directive_include.file_name_tokens);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kVarDef) {
    printf("(VarDef:");
    PrintASTNodePadding(depth);
    printf("type=");
    PrintTokenList(node->data.var_def.type_tokens);
    PrintASTNodePadding(depth);
    printf("name=%s", node->data.var_def.name->str);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDecl) {
    printf("(FuncDecl:");
    PrintASTNodePadding(depth);
    printf("type_and_name=");
    PrintASTNode(node->data.func_decl.type_and_name, depth + 1);
    PrintASTNodePadding(depth);
    printf("arg_list=");
    PrintASTNodeList(node->data.func_decl.arg_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDef) {
    printf("(FuncDef:");
    PrintASTNodePadding(depth);
    printf("func_decl=");
    PrintASTNode(node->data.func_def.func_decl, depth + 1);
    PrintASTNodePadding(depth);
    printf("comp_stmt=");
    PrintASTNode(node->data.func_def.comp_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kCompStatement) {
    printf("(CompStatement:");
    PrintASTNodePadding(depth);
    printf("(body=");
    PrintASTNodeList(node->data.comp_stmt.stmt_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExpressionStatement) {
    printf("(ExpressionStatement:");
    PrintASTNodePadding(depth);
    printf("expression=");
    PrintTokenList(node->data.expression_stmt.expression);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kReturnStatement) {
    printf("(ReturnStatement:");
    PrintASTNodePadding(depth);
    printf("expression_stmt=");
    PrintASTNode(node->data.return_stmt.expression_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kForStatement) {
    printf("(ForStatement:");
    PrintASTNodePadding(depth);
    printf("init_expression=");
    PrintTokenList(node->data.for_stmt.init_expression);
    PrintASTNodePadding(depth);
    printf("cond_expression=");
    PrintTokenList(node->data.for_stmt.cond_expression);
    PrintASTNodePadding(depth);
    printf("updt_expression=");
    PrintTokenList(node->data.for_stmt.updt_expression);
    PrintASTNodePadding(depth);
    printf("body_comp_stmt=");
    PrintASTNode(node->data.for_stmt.body_comp_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else {
    Error("PrintASTNode not implemented for type %d", node->type);
  }
}

ASTNodeList *AllocateASTNodeList() {
  ASTNodeList *list = malloc(sizeof(ASTNodeList));
  list->used = 0;
  return list;
}

void AppendASTNodeToList(ASTNodeList *list, ASTNode *node) {
  if (list->used >= AST_NODE_LIST_SIZE) {
    Error("No more space in ASTNodeList");
  }
  list->nodes[list->used++] = node;
}

void PrintASTNodeList(ASTNodeList *list, int depth) {
  putchar('[');
  PrintASTNodePadding(depth);
  for (int i = 0; i < list->used; i++) {
    PrintASTNode(list->nodes[i], depth + 1);
    PrintASTNodePadding(depth);
  }
  putchar(']');
}

