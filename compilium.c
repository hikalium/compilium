#include "compilium.h"

const ASTDataRoot *GetDataAsRoot(const ASTNode *node)
{
  if(node->type != kRoot) return NULL;
  return &node->data.root;
}

const ASTDataVarDef *GetDataAsVarDef(const ASTNode *node)
{
  if(node->type != kVarDef) return NULL;
  return &node->data.var_def;
}

const ASTDataFuncDecl *GetDataAsFuncDecl(const ASTNode *node)
{
  if(node->type != kFuncDecl) return NULL;
  return &node->data.func_decl;
}

const ASTDataFuncDef *GetDataAsFuncDef(const ASTNode *node)
{
  if(node->type != kFuncDef) return NULL;
  return &node->data.func_def;
}

const ASTDataCompStmt *GetDataAsCompStmt(const ASTNode *node)
{
  if(node->type != kCompStmt) return NULL;
  return &node->data.comp_stmt;
}

const ASTDataExprStmt *GetDataAsExprStmt(const ASTNode *node)
{
  if(node->type != kExprStmt) return NULL;
  return &node->data.expr_stmt;
}

const ASTDataReturnStmt *GetDataAsReturnStmt(const ASTNode *node)
{
  if(node->type != kReturnStmt) return NULL;
  return &node->data.return_stmt;
}

void GenerateSymbolForFuncDef(FILE *fp, const ASTNode *node){
  const ASTDataFuncDef *def = GetDataAsFuncDef(node);
  const ASTDataFuncDecl *decl = GetDataAsFuncDecl(def->func_decl);
  const ASTDataVarDef *defv = GetDataAsVarDef(decl->type_and_name);
  fprintf(fp, ".global _%s\n", defv->name->str);
}

void GenerateCodeForCompStmt(FILE *fp, const ASTNode *node){
  const ASTDataCompStmt *comp = GetDataAsCompStmt(node);
  const ASTNodeList *stmt_list = comp->stmt_list;
  for(int i = 0; i < stmt_list->used; i++){
    Generate(fp, stmt_list->nodes[i]);
  }
}

void GenerateCodeForFuncDef(FILE *fp, const ASTNode *node){
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

void GenerateCodeForExprStmt(FILE *fp, const ASTNode *node){
  const ASTDataExprStmt *expr_stmt = GetDataAsExprStmt(node);
  const TokenList *token_list = expr_stmt->expr;
  if(token_list->used == 1 && token_list->tokens[0]->type == kInteger){
    char *p;
    const char *s = token_list->tokens[0]->str;
    int var = strtol(s, &p, 0);
    if(!(s[0] != 0 && *p == 0)){
      Error("%s is not valid as integer.", s);
    }
    fprintf(fp, "mov     rax, %d\n", var);
  } else{
    Error("GenerateCodeForExprStmt: Not implemented ");
  }
}

void Generate(FILE *fp, const ASTNode *node)
{
  if(node->type == kRoot){
    fputs(".intel_syntax noprefix\n", fp);
    ASTNodeList *list = GetDataAsRoot(node)->root_list;
    // Generate symbols
    for(int i = 0; i < list->used; i++){
      const ASTNode *child_node = list->nodes[i];
      if(child_node->type == kFuncDef){
        GenerateSymbolForFuncDef(fp, child_node);
      }
    }
    // Generate function bodies
    for(int i = 0; i < list->used; i++){
      const ASTNode *child_node = list->nodes[i];
      if(child_node->type == kFuncDef){
        GenerateCodeForFuncDef(fp, child_node);
      }
    }
  } else if(node->type == kReturnStmt){
    const ASTDataReturnStmt *ret = GetDataAsReturnStmt(node);
    GenerateCodeForExprStmt(fp, ret->expr_stmt);
  } else{
    Error("Generation for AST Type %d is not implemented.", node->type);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    Error("Usage: %s <src_c_file> <dst_S_file>", argv[0]);
  }

  char *input = ReadFile(argv[1]);
  Tokenize(input);
  free(input);

  ASTNode *ast = Parse();

  PrintASTNode(ast, 1); putchar('\n');

  FILE *dst_fp = fopen(argv[2], "wb");
  if(!dst_fp){
    Error("Failed to open %s", argv[2]);
  }
  Generate(dst_fp, ast);
  fclose(dst_fp);

  return 0;
}
