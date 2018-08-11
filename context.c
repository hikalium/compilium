#include <stdio.h>
#include <stdlib.h>

#include "compilium.h"

struct CONTEXT {
  const Context *parent;
  ASTDict *dict;
  ASTLabel *break_label;
};

Context *AllocContext(const Context *parent) {
  Context *context = malloc(sizeof(Context));
  context->parent = parent;
  context->dict = AllocASTDict(8);
  return context;
}

ASTNode *FindInContext(const Context *context, const char *key) {
  if (!context) return NULL;
  ASTNode *result = FindASTNodeInDict(context->dict, key);
  if (result) return result;
  return FindInContext(context->parent, key);
}

ASTNode *FindIdentInContext(const Context *context, ASTIdent *ident) {
  return FindInContext(context, ident->token->str);
}

int GetSizeOfContext(const Context *context) {
  int size = 0;
  for (int i = 0; i < GetSizeOfASTDict(context->dict); i++) {
    ASTVar *local_var = ToASTVar(GetASTNodeInDictAt(context->dict, i));
    if (!local_var) Error("GetStackSizeForContext: local_var is NULL");
    size += GetSizeOfType(local_var->var_type);
  }
  int max_align_size = GetAlignOfContext(context);
  size = (size + max_align_size - 1) / max_align_size * max_align_size;
  return size;
}

static int imax(int a, int b) { return a > b ? a : b; }

int GetAlignOfContext(const Context *context) {
  int max_align_size = 1;
  for (int i = 0; i < GetSizeOfASTDict(context->dict); i++) {
    ASTVar *local_var = ToASTVar(GetASTNodeInDictAt(context->dict, i));
    if (!local_var) Error("GetStackSizeForContext: local_var is NULL");
    max_align_size = imax(max_align_size, GetAlignOfType(local_var->var_type));
  }
  return max_align_size;
}

ASTVar *AppendLocalVarToContext(Context *context, ASTList *decl_specs,
                                ASTDecltor *decltor, Context *struct_names) {
  ASTVar *local_var = AllocAndInitASTVar(decl_specs, decltor, struct_names);
  local_var->ofs =
      GetSizeOfContext(context) + GetSizeOfType(local_var->var_type);
  AppendASTNodeToDict(context->dict, local_var->name, ToASTNode(local_var));
  return local_var;
}

ASTVar *AppendStructMemberToContext(Context *context, ASTList *decl_specs,
                                    ASTDecltor *decltor,
                                    Context *struct_names) {
  ASTVar *local_var = AllocAndInitASTVar(decl_specs, decltor, struct_names);
  local_var->ofs = GetSizeOfContext(context);
  AppendASTNodeToDict(context->dict, local_var->name, ToASTNode(local_var));
  return local_var;
}

void AppendTypeToContext(Context *context, const char *name, ASTType *type) {
  AppendASTNodeToDict(context->dict, name, ToASTNode(type));
}

void SetBreakLabelInContext(Context *context, ASTLabel *label) {
  context->break_label = label;
}

ASTLabel *GetBreakLabelInContext(Context *context) {
  return context->break_label;
}

void PrintContext(const Context *context) { DebugPrintASTNode(context->dict); }
