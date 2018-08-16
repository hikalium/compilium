#include "compilium.h"

struct CONTEXT {
  const Context *parent;
  ASTDict *dict;
  ASTLabel *break_label;
  ASTLabel *continue_label;
};

Context *struct_names;
Context *identifiers;

void InitGlobalContext() {
  struct_names = AllocContext(NULL);
  identifiers = AllocContext(NULL);
}

Context *AllocContext(const Context *parent) {
  Context *context = malloc(sizeof(Context));
  context->parent = parent;
  context->dict = AllocASTDict(1024);
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

static int GetNextBaseForLocalVarContext(const Context *context) {
  if (GetSizeOfASTDict(context->dict) == 0) return 0;
  ASTVar *var = ToASTVar(
      GetASTNodeInDictAt(context->dict, GetSizeOfASTDict(context->dict) - 1));
  assert(var);
  return var->ofs;
}

static int GetNextOfsForStructContext(const Context *context) {
  if (GetSizeOfASTDict(context->dict) == 0) return 0;
  ASTVar *var = ToASTVar(
      GetASTNodeInDictAt(context->dict, GetSizeOfASTDict(context->dict) - 1));
  assert(var);
  return var->ofs + GetSizeOfType(var->var_type);
}

int GetSizeOfContext(const Context *context) {
  int size = 0;
  if (GetSizeOfASTDict(context->dict)) {
    ASTVar *var = ToASTVar(
        GetASTNodeInDictAt(context->dict, GetSizeOfASTDict(context->dict) - 1));
    assert(var);
    size = var->ofs + GetSizeOfType(var->var_type);
  }
  int max_align_size = GetAlignOfContext(context);
  size = (size + max_align_size - 1) / max_align_size * max_align_size;
  return size;
}

static int imax(int a, int b) { return a > b ? a : b; }

int GetAlignOfContext(const Context *context) {
  int max_align_size = 1;
  for (int i = 0; i < GetSizeOfASTDict(context->dict); i++) {
    ASTVar *var = ToASTVar(GetASTNodeInDictAt(context->dict, i));
    assert(var);
    max_align_size = imax(max_align_size, GetAlignOfType(var->var_type));
  }
  return max_align_size;
}

ASTVar *AppendLocalVarToContext(Context *context, ASTList *decl_specs,
                                ASTDecltor *decltor) {
  ASTVar *local_var = AllocAndInitASTVar(decl_specs, decltor);
  int base = GetNextBaseForLocalVarContext(context);
  int align = GetAlignOfType(local_var->var_type);
  int ofs = base + GetSizeOfType(local_var->var_type);
  local_var->ofs = (ofs + align - 1) / align * align;
  AppendASTNodeToDict(context->dict, local_var->name, ToASTNode(local_var));
  return local_var;
}

ASTVar *AppendStructMemberToContext(Context *context, ASTList *decl_specs,
                                    ASTDecltor *decltor) {
  ASTVar *var = AllocAndInitASTVar(decl_specs, decltor);
  int ofs = GetNextOfsForStructContext(context);
  int align = GetAlignOfType(var->var_type);
  var->ofs = (ofs + align - 1) / align * align;
  AppendASTNodeToDict(context->dict, var->name, ToASTNode(var));
  return var;
}

void AppendTypeToContext(Context *context, const char *name, ASTType *type) {
  AppendToContext(context, name, ToASTNode(type));
}

void AppendToContext(Context *context, const char *name, ASTNode *node) {
  AppendASTNodeToDict(context->dict, name, node);
  PrintContext(context);
}

void SetBreakLabelInContext(Context *context, ASTLabel *label) {
  context->break_label = label;
}

ASTLabel *GetBreakLabelInContext(Context *context) {
  return context->break_label;
}

void SetContinueLabelInContext(Context *context, ASTLabel *label) {
  context->continue_label = label;
}

ASTLabel *GetContinueLabelInContext(Context *context) {
  return context->continue_label;
}

void PrintContext(const Context *context) { DebugPrintASTNode(context->dict); }
