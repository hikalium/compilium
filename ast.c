#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compilium.h"

struct AST_LIST {
  ASTNodeType type;
  int capacity;
  int size;
  ASTNode* nodes[];
};

typedef struct {
  const char* key;
  ASTNode* value;
} ASTDictEntry;

struct AST_DICT {
  ASTNodeType type;
  int capacity;
  int size;
  ASTDictEntry entries[];
};

static const char* ASTNodeTypeName[kNumOfASTNodeType];

void InitASTNodeTypeName() {
  ASTNodeTypeName[kASTFuncDef] = "FuncDef";
  ASTNodeTypeName[kASTCompStmt] = "CompStmt";
  ASTNodeTypeName[kASTExprUnaryPreOp] = "ExprUnaryPreOp";
  ASTNodeTypeName[kASTExprUnaryPostOp] = "ExprUnaryPostOp";
  ASTNodeTypeName[kASTExprBinOp] = "ExprBinOp";
  ASTNodeTypeName[kASTExprFuncCall] = "ExprFuncCall";
  ASTNodeTypeName[kASTInteger] = "Integer";
  ASTNodeTypeName[kASTString] = "String";
  ASTNodeTypeName[kASTExprStmt] = "ExprStmt";
  ASTNodeTypeName[kASTJumpStmt] = "JumpStmt";
  ASTNodeTypeName[kASTCondStmt] = "CondStmt";
  ASTNodeTypeName[kASTIfStmt] = "IfStmt";
  ASTNodeTypeName[kASTWhileStmt] = "WhileStmt";
  ASTNodeTypeName[kASTForStmt] = "ForStmt";
  ASTNodeTypeName[kASTILOp] = "ILOp";
  ASTNodeTypeName[kASTList] = "List";
  ASTNodeTypeName[kASTKeyword] = "Keyword";
  ASTNodeTypeName[kASTDecltor] = "Decltor";
  ASTNodeTypeName[kASTDirectDecltor] = "DirectDecltor";
  ASTNodeTypeName[kASTIdent] = "Ident";
  ASTNodeTypeName[kASTDecl] = "Decl";
  ASTNodeTypeName[kASTParamDecl] = "ParamDecl";
  ASTNodeTypeName[kASTStructDecl] = "StructDecl";
  ASTNodeTypeName[kASTStructSpec] = "StructSpec";
  ASTNodeTypeName[kASTPointer] = "Pointer";
  ASTNodeTypeName[kASTDict] = "Dict";
  ASTNodeTypeName[kASTVar] = "Var";
  ASTNodeTypeName[kASTLabel] = "Label";
  ASTNodeTypeName[kASTType] = "Type";
}

const char* GetASTNodeTypeName(ASTNode* node) {
  if (!node || kNumOfASTNodeType <= node->type) return "?";
  return ASTNodeTypeName[node->type];
}

ASTNode* ToASTNode(void* node) { return (ASTNode*)node; }

GenToAST(FuncDef);
GenToAST(CompStmt);
GenToAST(ExprUnaryPreOp);
GenToAST(ExprUnaryPostOp);
GenToAST(ExprBinOp);
GenToAST(ExprFuncCall);
GenToAST(Integer);
GenToAST(String);
GenToAST(ExprStmt);
GenToAST(JumpStmt);
GenToAST(CondStmt);
GenToAST(IfStmt);
GenToAST(WhileStmt);
GenToAST(ForStmt);
GenToAST(ILOp);
GenToAST(List);
GenToAST(Keyword);
GenToAST(Decltor);
GenToAST(DirectDecltor);
GenToAST(Ident);
GenToAST(Decl);
GenToAST(ParamDecl);
GenToAST(StructDecl);
GenToAST(StructSpec);
GenToAST(Pointer);
GenToAST(Dict);
GenToAST(Var);
GenToAST(Label);

GenAllocAST(FuncDef);
GenAllocAST(CompStmt);
GenAllocAST(ExprUnaryPreOp);
GenAllocAST(ExprUnaryPostOp);
GenAllocAST(ExprBinOp);
GenAllocAST(ExprFuncCall);
GenAllocAST(Integer);
GenAllocAST(String);
GenAllocAST(ExprStmt);
GenAllocAST(JumpStmt);
GenAllocAST(CondStmt);
GenAllocAST(IfStmt);
GenAllocAST(WhileStmt);
GenAllocAST(ForStmt);
GenAllocAST(ILOp);

ASTList* AllocASTList(int capacity) {
  ASTList* list = calloc(1, sizeof(ASTList) + sizeof(ASTNode*) * capacity);
  list->type = kASTList;
  list->capacity = capacity;
  list->size = 0;
  return list;
}

GenAllocAST(Keyword);
GenAllocAST(Decltor);
GenAllocAST(DirectDecltor);
GenAllocAST(Ident);
GenAllocAST(Decl);
GenAllocAST(ParamDecl);
GenAllocAST(StructDecl);
GenAllocAST(StructSpec);
GenAllocAST(Pointer);

ASTDict* AllocASTDict(int capacity) {
  ASTDict* dict = calloc(1, sizeof(ASTDict) + sizeof(ASTDictEntry) * capacity);
  dict->type = kASTDict;
  dict->capacity = capacity;
  dict->size = 0;
  return dict;
}

GenAllocAST(Var);
GenAllocAST(Label);

ASTInteger* AllocAndInitASTInteger(int value) {
  ASTInteger* node = AllocASTInteger();
  node->value = value;
  return node;
}

ASTString* AllocAndInitASTString(const char* str) {
  ASTString* node = AllocASTString();
  node->str = str;
  return node;
}

ASTIdent* AllocAndInitASTIdent(const Token* token) {
  ASTIdent* node = AllocASTIdent();
  node->token = token;
  return node;
}

ASTKeyword* AllocAndInitASTKeyword(const Token* token) {
  ASTKeyword* node = AllocASTKeyword();
  node->token = token;
  return node;
}

ASTNode* AllocAndInitASTExprBinOp(const Token* op, ASTNode* left,
                                  ASTNode* right) {
  ASTExprBinOp* node = AllocASTExprBinOp();
  node->op = op;
  node->left = left;
  node->right = right;
  return ToASTNode(node);
}

ASTNode* AllocAndInitASTExprFuncCall(ASTNode* func, ASTNode* args) {
  ASTExprFuncCall* node = AllocASTExprFuncCall();
  node->func = func;
  node->args = args;
  return ToASTNode(node);
}

ASTVar* AllocAndInitASTVar(ASTList* decl_specs, ASTDecltor* decltor) {
  const Token* ident_token = GetIdentTokenFromDecltor(decltor);
  ASTVar* local_var = AllocASTVar();
  local_var->name = ident_token->str;
  local_var->var_type = AllocAndInitASTType(decl_specs, decltor);
  return local_var;
}

const Token* GetIdentTokenFromDirectDecltor(ASTDirectDecltor* direct_decltor) {
  if (!direct_decltor) return NULL;
  if (direct_decltor->direct_decltor)
    return GetIdentTokenFromDirectDecltor(direct_decltor->direct_decltor);
  ASTIdent* ident = ToASTIdent(direct_decltor->data);
  if (!ident) return NULL;
  return ident->token;
}

const Token* GetIdentTokenFromDecltor(ASTDecltor* decltor) {
  if (!decltor) return NULL;
  return GetIdentTokenFromDirectDecltor(decltor->direct_decltor);
}

const Token* GetFuncNameTokenFromFuncDef(ASTFuncDef* func_def) {
  if (!func_def) return NULL;
  return GetIdentTokenFromDecltor(func_def->decltor);
}

int IsTypedefDeclSpecs(ASTList* decl_specs) {
  ASTNode* node = GetASTNodeAt(decl_specs, 0);
  return node->type == kASTKeyword &&
         IsEqualToken(ToASTKeyword(node)->token, "typedef");
}

static void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
}

static void PrintfWithPadding(int depth, const char* fmt, ...) {
  PrintASTNodePadding(depth);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

static void PrintASTNodeWithName(int depth, const char* name, void* node) {
  PrintfWithPadding(depth, name);
  PrintASTNode(node, depth);
}

static void PrintTokenWithName(int depth, const char* name,
                               const Token* token) {
  PrintfWithPadding(depth, name);
  PrintToken(token);
}

void PrintASTNode(void* node, int depth) {
  ASTNode* n = ToASTNode(node);
  if (!node) {
    printf("(Null)");
    return;
  }
  if (n->type == kASTList) {
    ASTList* list = ToASTList(n);
    putchar('[');
    for (int i = 0; i < list->size; i++) {
      PrintASTNodePadding(depth + 1);
      PrintASTNode(list->nodes[i], depth + 1);
    }
    PrintASTNodePadding(depth);
    putchar(']');
    return;
  } else if (n->type == kASTDict) {
    ASTDict* dict = ToASTDict(n);
    putchar('{');
    for (int i = 0; i < dict->size; i++) {
      PrintASTNodePadding(depth + 1);
      printf("\"%s\" : ", dict->entries[i].key);
      PrintASTNode(dict->entries[i].value, depth + 1);
    }
    PrintASTNodePadding(depth);
    putchar('}');
    return;
  } else if (n->type < kNumOfASTNodeType) {
    printf("(%s:", ASTNodeTypeName[n->type]);
  } else {
    printf("(Unknown: %d)", n->type);
    return;
  }
  if (n->type == kASTFuncDef) {
    ASTFuncDef* func_def = ToASTFuncDef(n);
    PrintASTNodeWithName(depth + 1, "func_type=", func_def->func_type);
    PrintASTNodeWithName(depth + 1, "decl_specs=", func_def->decl_specs);
    PrintASTNodeWithName(depth + 1, "decltor=", func_def->decltor);
    PrintASTNodeWithName(depth + 1, "comp_stmt=", func_def->comp_stmt);
  } else if (n->type == kASTCompStmt) {
    ASTCompStmt* comp_stmt = ToASTCompStmt(n);
    PrintASTNodeWithName(depth + 1, "body=", comp_stmt->stmt_list);
  } else if (n->type == kASTExprUnaryPreOp) {
    ASTExprUnaryPreOp* expr_unary_pre_op = ToASTExprUnaryPreOp(n);
    PrintTokenWithName(depth + 1, "op=", expr_unary_pre_op->op);
    PrintASTNodeWithName(depth + 1, "expr_type=", expr_unary_pre_op->expr_type);
    PrintASTNodeWithName(depth + 1, "expr=", expr_unary_pre_op->expr);
  } else if (n->type == kASTExprUnaryPostOp) {
    ASTExprUnaryPostOp* expr_unary_post_op = ToASTExprUnaryPostOp(n);
    PrintTokenWithName(depth + 1, "op=", expr_unary_post_op->op);
    PrintASTNodeWithName(depth + 1,
                         "expr_type=", expr_unary_post_op->expr_type);
    PrintASTNodeWithName(depth + 1, "expr=", expr_unary_post_op->expr);
  } else if (n->type == kASTExprBinOp) {
    ASTExprBinOp* expr_bin_op = ToASTExprBinOp(n);
    PrintTokenWithName(depth + 1, "op=", expr_bin_op->op);
    PrintASTNodeWithName(depth + 1, "expr_type=", expr_bin_op->expr_type);
    PrintASTNodeWithName(depth + 1, "left=", expr_bin_op->left);
    PrintASTNodeWithName(depth + 1, "right=", expr_bin_op->right);
  } else if (n->type == kASTExprFuncCall) {
    ASTExprFuncCall* expr_func_call = ToASTExprFuncCall(n);
    PrintASTNodeWithName(depth + 1, "func=", expr_func_call->func);
    PrintASTNodeWithName(depth + 1, "args=", expr_func_call->args);
  } else if (n->type == kASTInteger) {
    ASTInteger* constant = ToASTInteger(n);
    PrintfWithPadding(depth + 1, "value=%d", constant->value);
  } else if (n->type == kASTString) {
    ASTString* constant = ToASTString(n);
    PrintfWithPadding(depth + 1, "str=", constant->str);
  } else if (n->type == kASTExprStmt) {
    ASTExprStmt* expr_stmt = ToASTExprStmt(n);
    PrintASTNodeWithName(depth + 1, "expression=", expr_stmt->expr);
  } else if (n->type == kASTJumpStmt) {
    ASTJumpStmt* jump_stmt = ToASTJumpStmt(n);
    PrintASTNodeWithName(depth + 1, "kw=", jump_stmt->kw);
    PrintASTNodeWithName(depth + 1, "param=", jump_stmt->param);
  } else if (n->type == kASTCondStmt) {
    ASTCondStmt* cond_stmt = ToASTCondStmt(n);
    PrintASTNodeWithName(depth + 1, "cond_expr=", cond_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "true_expr=", cond_stmt->true_expr);
    PrintASTNodeWithName(depth + 1, "false_expr=", cond_stmt->false_expr);
  } else if (n->type == kASTIfStmt) {
    ASTIfStmt* if_stmt = ToASTIfStmt(n);
    PrintASTNodeWithName(depth + 1, "cond_expr=", if_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "true_stmt=", if_stmt->true_stmt);
    PrintASTNodeWithName(depth + 1, "false_stmt=", if_stmt->false_stmt);
  } else if (n->type == kASTWhileStmt) {
    ASTWhileStmt* stmt = ToASTWhileStmt(n);
    PrintASTNodeWithName(depth + 1, "cond_expr=", stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "body_stmt=", stmt->body_stmt);
  } else if (n->type == kASTForStmt) {
    ASTForStmt* for_stmt = ToASTForStmt(n);
    PrintASTNodeWithName(depth + 1, "init_expr=", for_stmt->init_expr);
    PrintASTNodeWithName(depth + 1, "cond_expr=", for_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "updt_expr=", for_stmt->updt_expr);
    PrintASTNodeWithName(depth + 1, "body_stmt=", for_stmt->body_stmt);
  } else if (n->type == kASTILOp) {
    ASTILOp* il_op = ToASTILOp(n);
    PrintfWithPadding(depth + 1, "op=%s", GetILOpTypeName(il_op->op));
    PrintfWithPadding(depth + 1, "dst=%d",
                      il_op->dst ? il_op->dst->vreg_id : 0);
    PrintfWithPadding(depth + 1, "left=%d",
                      il_op->left ? il_op->left->vreg_id : 0);
    PrintfWithPadding(depth + 1, "right=%d",
                      il_op->right ? il_op->right->vreg_id : 0);
    PrintASTNodeWithName(depth + 1, "ast_n=", il_op->ast_node);
  } else if (n->type == kASTKeyword) {
    ASTKeyword* kw = ToASTKeyword(n);
    PrintTokenWithName(depth + 1, "token=", kw->token);
  } else if (n->type == kASTDecltor) {
    ASTDecltor* decltor = ToASTDecltor(n);
    PrintASTNodeWithName(depth + 1, "pointer=", decltor->pointer);
    PrintASTNodeWithName(depth + 1, "direct_decltor=", decltor->direct_decltor);
  } else if (n->type == kASTDirectDecltor) {
    ASTDirectDecltor* direct_decltor = ToASTDirectDecltor(n);
    PrintTokenWithName(depth + 1,
                       "bracket_token=", direct_decltor->bracket_token);
    PrintASTNodeWithName(depth + 1, "data=", direct_decltor->data);
    PrintASTNodeWithName(depth + 1,
                         "direct_decltor=", direct_decltor->direct_decltor);
  } else if (n->type == kASTIdent) {
    ASTIdent* ident = ToASTIdent(n);
    PrintTokenWithName(depth + 1, "token=", ident->token);
    PrintASTNodeWithName(depth + 1, "var_type=", ident->var_type);
    PrintASTNodeWithName(depth + 1, "local_var=", ident->local_var);
  } else if (n->type == kASTDecl) {
    ASTDecl* decl = ToASTDecl(n);
    PrintASTNodeWithName(depth + 1, "decl_specs=", decl->decl_specs);
    PrintASTNodeWithName(depth + 1, "init_decltors=", decl->init_decltors);
  } else if (n->type == kASTParamDecl) {
    ASTParamDecl* param_decl = ToASTParamDecl(n);
    PrintASTNodeWithName(depth + 1, "decl_specs=", param_decl->decl_specs);
    PrintASTNodeWithName(depth + 1, "decltor=", param_decl->decltor);
  } else if (n->type == kASTStructDecl) {
    ASTStructDecl* struct_decl = ToASTStructDecl(n);
    PrintASTNodeWithName(depth + 1,
                         "spec_qual_list=", struct_decl->spec_qual_list);
    PrintASTNodeWithName(
        depth + 1, "struct_decltor_list=", struct_decl->struct_decltor_list);
  } else if (n->type == kASTStructSpec) {
    ASTStructSpec* struct_spec = ToASTStructSpec(n);
    PrintTokenWithName(depth + 1, "ident=", struct_spec->ident);
    PrintASTNodeWithName(depth + 1,
                         "struct_decl_list=", struct_spec->struct_decl_list);
  } else if (n->type == kASTPointer) {
    ASTPointer* pointer = ToASTPointer(n);
    PrintASTNodeWithName(depth + 1, "pointer=", pointer->pointer);
  } else if (n->type == kASTLabel) {
    ASTLabel* label = ToASTLabel(n);
    PrintfWithPadding(depth + 1, "label_number=%d", label->label_number);
  } else if (n->type == kASTVar) {
    ASTVar* var = ToASTVar(n);
    PrintASTNodeWithName(depth + 1, "type=", var->var_type);
    PrintfWithPadding(depth + 1, "name=%s", var->name);
    PrintfWithPadding(depth + 1, "ofs=%d", var->ofs);
  } else if (n->type == kASTType) {
    PrintASTType(ToASTType(n));
    putchar(')');
    return;
  } else {
    Error("PrintASTNode not implemented for type %d (%s)", n->type,
          GetASTNodeTypeName(n));
  }
  PrintfWithPadding(depth, ")");
}

void DebugPrintASTNode(void* node) {
  PrintASTNode(node, 0);
  putchar('\n');
}

// ASTList

void PushASTNodeToList(ASTList* list, ASTNode* node) {
  if (list->size >= list->capacity) {
    Error("No more space in ASTList");
  }
  list->nodes[list->size++] = node;
}

ASTNode* PopASTNodeFromList(ASTList* list) {
  if (list->size <= 0) {
    Error("Trying to pop empty ASTList");
  }
  return list->nodes[--list->size];
}

ASTNode* GetASTNodeAt(const ASTList* list, int index) {
  if (index < 0 || list->size <= index) {
    Error("ASTList: Trying to read index out of bound");
  }
  return list->nodes[index];
}

void SetASTNodeAt(ASTList* list, int index, ASTNode* node) {
  if (index < 0 || list->size <= index) {
    Error("ASTList: Trying to write index out of bound");
  }
  list->nodes[index] = node;
}

int GetSizeOfASTList(const ASTList* list) { return list->size; }

ASTNode* GetLastASTNode(const ASTList* list) {
  return GetASTNodeAt(list, GetSizeOfASTList(list) - 1);
}

// ASTDict

void AppendASTNodeToDict(ASTDict* dict, const char* key, ASTNode* node) {
  if (dict->size >= dict->capacity) {
    Error("No more space in ASTDict");
  }
  dict->entries[dict->size].key = key;
  dict->entries[dict->size].value = node;
  dict->size++;
}

ASTNode* FindASTNodeInDict(ASTDict* dict, const char* key) {
  for (int i = 0; i < dict->size; i++) {
    if (strcmp(key, dict->entries[i].key) == 0) return dict->entries[i].value;
  }
  return NULL;
}

ASTNode* GetASTNodeInDictAt(const ASTDict* dict, int index) {
  if (index < 0 || dict->size <= index) {
    Error("ASTDict: Trying to read index out of bound");
  }
  return dict->entries[index].value;
}

int GetSizeOfASTDict(const ASTDict* dict) { return dict->size; }
