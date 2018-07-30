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

int GetByteSizeOfDeclSpecs(ASTList *decl_specs) {
  if (GetSizeOfASTList(decl_specs) == 1) {
    ASTKeyword *kw = ToASTKeyword(GetASTNodeAt(decl_specs, 0));
    if (!kw) Error("decl_specs should have a ASTkeyword.");
    if (IsEqualToken(kw->token, "char")) {
      return 1;
    } else if (IsEqualToken(kw->token, "int")) {
      return 8;
    }
    Error("Not implemented decl_spec %s", kw->token->str);
  }
  Error("Not supported decl_specs consist of multiple tokens");
  return -1;
}

int GetByteSizeOfDecl(ASTList *decl_specs, ASTDecltor *decltor) {
  if (decltor->pointer) return 8;
  return GetByteSizeOfDeclSpecs(decl_specs);
}

int GetByteSizeOfDeclAfterDeref(ASTList *decl_specs, ASTDecltor *decltor) {
  if (!decltor->pointer) Error("Cannot deref value which is not a pointer");
  if (decltor->pointer->pointer) return 8;
  return GetByteSizeOfDeclSpecs(decl_specs);
}

int GetStackSizeForContext(const Context *context) {
  int size = 0;
  for (int i = 0; i < GetSizeOfASTDict(context->dict); i++) {
    ASTLocalVar *local_var =
        ToASTLocalVar(GetASTNodeInDictAt(context->dict, i));
    if (!local_var) Error("GetStackSizeForContext: local_var is NULL");
    size += local_var->size;
  }
  return size;
}

ASTLocalVar *AppendLocalVarInContext(Context *context, ASTList *decl_specs,
                                     ASTDecltor *decltor) {
  const Token *ident_token = GetIdentTokenFromDecltor(decltor);
  int size = GetByteSizeOfDecl(decl_specs, decltor);
  int ofs_in_stack = GetStackSizeForContext(context) + size;
  printf("LocalVar[\"%s\"] ofs_in_stack = %d, size = %d\n", ident_token->str,
         ofs_in_stack, size);
  ASTLocalVar *local_var = AllocASTLocalVar(ofs_in_stack);
  local_var->size = size;
  local_var->name = ident_token->str;
  local_var->decl_specs = decl_specs;
  local_var->decltor = decltor;
  AppendASTNodeToDict(context->dict, ident_token->str, ToASTNode(local_var));
  return local_var;
}

void SetBreakLabelInContext(Context *context, ASTLabel *label) {
  context->break_label = label;
}

ASTLabel *GetBreakLabelInContext(Context *context) {
  return context->break_label;
}
