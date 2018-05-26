#include "compilium.h"

void GenerateSymbolForFuncDef(FILE *fp, const ASTNode *node) {
  const ASTDataFuncDef *def = GetDataAsFuncDef(node);
  const ASTDataFuncDecl *decl = GetDataAsFuncDecl(def->func_decl);
  const ASTDataVarDef *defv = GetDataAsVarDef(decl->type_and_name);
  fprintf(fp, ".global _%s\n", defv->name->str);
}

void GenerateCodeForCompStmt(FILE *fp, const ASTNode *node) {
  const ASTDataCompStmt *comp = GetDataAsCompStmt(node);
  const ASTNodeList *stmt_list = comp->stmt_list;
  for (int i = 0; i < stmt_list->used; i++) {
    Generate(fp, stmt_list->nodes[i]);
  }
}

void GenerateCodeForFuncDef(FILE *fp, const ASTNode *node) {
  const ASTDataFuncDef *def = GetDataAsFuncDef(node);
  const ASTDataFuncDecl *decl = GetDataAsFuncDecl(def->func_decl);
  const ASTDataVarDef *defv = GetDataAsVarDef(decl->type_and_name);
  //
  fprintf(fp, "_%s:\n", defv->name->str);
  fprintf(fp, "push    rbp\n");
  fprintf(fp, "mov     rbp, rsp\n");
  // body. rax is return value.
  GenerateCodeForCompStmt(fp, def->comp_stmt);
  //
  fprintf(fp, "mov     dword ptr [rbp - 4], 0\n");
  fprintf(fp, "pop     rbp\n");
  fprintf(fp, "ret\n");
}

int GetLabelNumber() {
  static int num = 0;
  return num++;
}

void GenerateCodeForExprBinOp(FILE *fp, const ASTNode *node) {
  Error("Not implemented GenerateCodeForExprBinOp");
}

void GenerateCodeForExprVal(FILE *fp, const ASTNode *node) {
  // https://wiki.osdev.org/System_V_ABI
  const ASTDataExprVal *expr_val = GetDataAsExprVal(node);
  char *p;
  const char *s = expr_val->token->str;
  int var = strtol(s, &p, 0);
  if (!(s[0] != 0 && *p == 0)) {
    Error("%s is not valid as integer.", s);
  }
  fprintf(fp, "mov     rax, %d\n", var);
}

void GenerateCodeForExprStmt(FILE *fp, const ASTNode *node) {
  // https://wiki.osdev.org/System_V_ABI
  const ASTDataExprStmt *expr_stmt = GetDataAsExprStmt(node);
  Generate(fp, expr_stmt->expr);
  /*
  const TokenList *token_list = expr_stmt->expr;
  if (token_list->used == 1 && token_list->tokens[0]->type == kInteger) {
    char *p;
    const char *s = token_list->tokens[0]->str;
    int var = strtol(s, &p, 0);
    if (!(s[0] != 0 && *p == 0)) {
      Error("%s is not valid as integer.", s);
    }
    fprintf(fp, "mov     rax, %d\n", var);
  } else if (token_list->used == 4 &&
             IsEqualToken(token_list->tokens[0], "puts") &&
             IsEqualToken(token_list->tokens[1], "(") &&
             token_list->tokens[2]->type == kStringLiteral &&
             IsEqualToken(token_list->tokens[3], ")")) {
    int label_for_skip = GetLabelNumber();
    int label_str = GetLabelNumber();
    fprintf(fp, "jmp L%d\n", label_for_skip);
    fprintf(fp, "L%d:\n", label_str);
    fprintf(fp, ".asciz  \"%s\"\n", token_list->tokens[2]->str);
    fprintf(fp, "L%d:\n", label_for_skip);
    fprintf(fp, "sub     rsp, 16\n");
    fprintf(fp, "lea     rdi, [rip + L%d]\n", label_str);
    fprintf(fp, "call    _%s\n", token_list->tokens[0]->str);
    fprintf(fp, "add     rsp, 16\n");
    //Error("GenerateCodeForExprStmt: puts case ");


  } else {
    Error("GenerateCodeForExprStmt: Not implemented ");
  }
  */
}

void Generate(FILE *fp, const ASTNode *node) {
  if (node->type == kRoot) {
    fputs(".intel_syntax noprefix\n", fp);
    ASTNodeList *list = GetDataAsRoot(node)->root_list;
    // Generate symbols
    for (int i = 0; i < list->used; i++) {
      const ASTNode *child_node = list->nodes[i];
      if (child_node->type == kFuncDef) {
        GenerateSymbolForFuncDef(fp, child_node);
      }
    }
    // Generate function bodies
    for (int i = 0; i < list->used; i++) {
      const ASTNode *child_node = list->nodes[i];
      if (child_node->type == kFuncDef) {
        GenerateCodeForFuncDef(fp, child_node);
      }
    }
  } else if (node->type == kReturnStmt) {
    const ASTDataReturnStmt *ret = GetDataAsReturnStmt(node);
    GenerateCodeForExprStmt(fp, ret->expr_stmt);
  } else if (node->type == kExprBinOp) {
    GenerateCodeForExprBinOp(fp, node);
  } else if (node->type == kExprVal) {
    GenerateCodeForExprVal(fp, node);
  } else if (node->type == kExprStmt) {
    GenerateCodeForExprStmt(fp, node);
  } else {
    Error("Generation for AST Type %d is not implemented.", node->type);
  }
}
