#include <stdio.h>
#include <stdlib.h>

#include "compilium.h"

struct CONTEXT {
  const Context *parent;
  ASTDict *dict;
  ASTLabel *break_label;
};

Context *AllocContext(const Context *parent) {
  printf("AllocContext\n");
  Context *context = malloc(sizeof(Context));
  context->parent = parent;
  context->dict = AllocASTDict(8);
  return context;
}

ASTNode *FindIdentInContext(const Context *context, ASTIdent *ident) {
  if (!context) return NULL;
  ASTNode *result = FindASTNodeInDict(context->dict, ident->token->str);
  if (result) return result;
  return FindIdentInContext(context->parent, ident);
}

int GetStackSizeForContext(const Context *context) {
  int size = 0;
  for (int i = 0; i < GetSizeOfASTDict(context->dict); i++) {
    ASTLocalVar *local_var =
        ToASTLocalVar(GetASTNodeInDictAt(context->dict, i));
    if (!local_var) Error("GetStackSizeForContext: local_var is NULL");
    size += GetSizeOfType(local_var->var_type);
  }
  return size;
}

ASTLocalVar *AppendLocalVarInContext(Context *context, ASTList *decl_specs,
                                     ASTDecltor *decltor) {
  const Token *ident_token = GetIdentTokenFromDecltor(decltor);
  ASTLocalVar *local_var = AllocASTLocalVar();
  local_var->name = ident_token->str;
  local_var->var_type = AllocAndInitASTType(decl_specs, decltor);
  local_var->ofs_in_stack =
      GetStackSizeForContext(context) + GetSizeOfType(local_var->var_type);
  AppendASTNodeToDict(context->dict, ident_token->str, ToASTNode(local_var));
  PrintASTNode(ToASTNode(local_var), 0);
  putchar('\n');
  return local_var;
}

void SetBreakLabelInContext(Context *context, ASTLabel *label) {
  context->break_label = label;
}

ASTLabel *GetBreakLabelInContext(Context *context) {
  return context->break_label;
}
