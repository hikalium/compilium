#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compilium.h"

struct AST_LIST {
  ASTType type;
  int capacity;
  int size;
  ASTNode* nodes[];
};

typedef struct {
  const char* key;
  ASTNode* value;
} ASTDictEntry;

struct AST_DICT {
  ASTType type;
  int capacity;
  int size;
  ASTDictEntry entries[];
};

const char* ASTTypeName[kNumOfASTType];

void InitASTTypeName() {
  ASTTypeName[kASTFuncDecl] = "FuncDecl";
  ASTTypeName[kASTFuncDef] = "FuncDef";
  ASTTypeName[kASTCompStmt] = "CompStmt";
  ASTTypeName[kASTExprUnaryPreOp] = "ExprUnaryPreOp";
  ASTTypeName[kASTExprUnaryPostOp] = "ExprUnaryPostOp";
  ASTTypeName[kASTExprBinOp] = "ExprBinOp";
  ASTTypeName[kASTExprFuncCall] = "ExprFuncCall";
  ASTTypeName[kASTInteger] = "Integer";
  ASTTypeName[kASTString] = "String";
  ASTTypeName[kASTExprStmt] = "ExprStmt";
  ASTTypeName[kASTJumpStmt] = "JumpStmt";
  ASTTypeName[kASTCondStmt] = "CondStmt";
  ASTTypeName[kASTIfStmt] = "IfStmt";
  ASTTypeName[kASTWhileStmt] = "WhileStmt";
  ASTTypeName[kASTForStmt] = "ForStmt";
  ASTTypeName[kASTILOp] = "ILOp";
  ASTTypeName[kASTList] = "List";
  ASTTypeName[kASTKeyword] = "Keyword";
  ASTTypeName[kASTDecltor] = "Decltor";
  ASTTypeName[kASTDirectDecltor] = "DirectDecltor";
  ASTTypeName[kASTIdent] = "Ident";
  ASTTypeName[kASTDecl] = "Decl";
  ASTTypeName[kASTParamDecl] = "ParamDecl";
  ASTTypeName[kASTDict] = "Dict";
  ASTTypeName[kASTLocalVar] = "LocalVar";
  ASTTypeName[kASTLabel] = "Label";
}

const char* GetASTTypeName(ASTNode* node) {
  if (!node || kNumOfASTType <= node->type) return "?";
  return ASTTypeName[node->type];
}

ASTNode* ToASTNode(void* node) { return (ASTNode*)node; }

#define GenToAST(Type) \
  AST##Type* ToAST##Type(ASTNode* node) { \
    if (!node || node->type != kAST##Type) return NULL; \
    return (AST##Type*)node; \
  }

GenToAST(FuncDecl);
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
GenToAST(Pointer);
GenToAST(Dict);
GenToAST(LocalVar);
GenToAST(Label);

#define GenAllocAST(Type) \
  AST##Type* AllocAST##Type() { \
    AST##Type* node = (AST##Type*)calloc(1, sizeof(AST##Type)); \
    node->type = kAST##Type; \
    return node; \
  }

GenAllocAST(FuncDecl);
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
GenAllocAST(Pointer);

ASTDict* AllocASTDict(int capacity) {
  ASTDict* dict = calloc(1, sizeof(ASTDict) + sizeof(ASTDictEntry*) * capacity);
  dict->type = kASTDict;
  dict->capacity = capacity;
  dict->size = 0;
  return dict;
}

ASTLocalVar* AllocASTLocalVar(int ofs_in_stack) {
  ASTLocalVar* var = calloc(1, sizeof(ASTLocalVar));
  var->type = kASTLocalVar;
  var->ofs_in_stack = ofs_in_stack;
  return var;
}

ASTLabel* AllocASTLabel() {
  ASTLabel* label = calloc(1, sizeof(ASTLabel));
  label->type = kASTLabel;
  label->label_number = 0;
  return label;
}

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

void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
}

void PrintfWithPadding(int depth, const char* fmt, ...) {
  PrintASTNodePadding(depth);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

void PrintASTNodeWithName(int depth, const char* name, ASTNode* node) {
  PrintfWithPadding(depth, name);
  PrintASTNode(node, depth);
}
void PrintTokenWithName(int depth, const char* name, const Token* token) {
  PrintfWithPadding(depth + 1, name);
  PrintToken(token);
}
void PrintTokenListWithName(int depth, const char* name, TokenList* list) {
  PrintfWithPadding(depth + 1, name);
  PrintTokenList(list);
}
void PrintASTNode(ASTNode* node, int depth) {
  if (!node) {
    printf("(Null)");
    return;
  }
  if (node->type == kASTList) {
    ASTList* list = ToASTList(node);
    putchar('[');
    for (int i = 0; i < list->size; i++) {
      PrintASTNodePadding(depth + 1);
      PrintASTNode(list->nodes[i], depth + 1);
    }
    PrintASTNodePadding(depth);
    putchar(']');
    return;
  } else if (node->type < kNumOfASTType) {
    printf("(%s:", ASTTypeName[node->type]);
  } else {
    printf("(Unknown: %d)", node->type);
    return;
  }
  if (node->type == kASTFuncDecl) {
    ASTFuncDecl* func_decl = ToASTFuncDecl(node);
    PrintASTNodeWithName(depth + 1, "type_and_name=", func_decl->type_and_name);
    PrintASTNodeWithName(depth + 1,
                         "arg_list=", ToASTNode(func_decl->arg_list));
  } else if (node->type == kASTFuncDef) {
    ASTFuncDef* func_def = ToASTFuncDef(node);
    PrintASTNodeWithName(depth + 1,
                         "decl_specs=", ToASTNode(func_def->decl_specs));
    PrintASTNodeWithName(depth + 1, "decltor=", ToASTNode(func_def->decltor));
    PrintASTNodeWithName(depth + 1,
                         "comp_stmt=", ToASTNode(func_def->comp_stmt));
  } else if (node->type == kASTCompStmt) {
    ASTCompStmt* comp_stmt = ToASTCompStmt(node);
    PrintASTNodeWithName(depth + 1, "body=", ToASTNode(comp_stmt->stmt_list));
  } else if (node->type == kASTExprUnaryPreOp) {
    ASTExprUnaryPreOp* expr_unary_pre_op = ToASTExprUnaryPreOp(node);
    PrintTokenWithName(depth + 1, "op=", expr_unary_pre_op->op);
    PrintASTNodeWithName(depth + 1, "expr=", expr_unary_pre_op->expr);
  } else if (node->type == kASTExprUnaryPostOp) {
    ASTExprUnaryPostOp* expr_unary_post_op = ToASTExprUnaryPostOp(node);
    PrintTokenWithName(depth + 1, "op=", expr_unary_post_op->op);
    PrintASTNodeWithName(depth + 1, "expr=", expr_unary_post_op->expr);
  } else if (node->type == kASTExprBinOp) {
    ASTExprBinOp* expr_bin_op = ToASTExprBinOp(node);
    PrintTokenWithName(depth + 1, "op=", expr_bin_op->op);
    PrintASTNodeWithName(depth + 1, "left=", expr_bin_op->left);
    PrintASTNodeWithName(depth + 1, "right=", expr_bin_op->right);
  } else if (node->type == kASTExprFuncCall) {
    ASTExprFuncCall* expr_func_call = ToASTExprFuncCall(node);
    PrintASTNodeWithName(depth + 1, "func=", expr_func_call->func);
    PrintASTNodeWithName(depth + 1, "args=", expr_func_call->args);
  } else if (node->type == kASTInteger) {
    ASTInteger* constant = ToASTInteger(node);
    PrintfWithPadding(depth + 1, "value=%d", constant->value);
  } else if (node->type == kASTString) {
    ASTString* constant = ToASTString(node);
    PrintfWithPadding(depth + 1, "str=", constant->str);
  } else if (node->type == kASTExprStmt) {
    ASTExprStmt* expr_stmt = ToASTExprStmt(node);
    PrintASTNodeWithName(depth + 1, "expression=", expr_stmt->expr);
  } else if (node->type == kASTJumpStmt) {
    ASTJumpStmt* jump_stmt = ToASTJumpStmt(node);
    PrintASTNodeWithName(depth + 1, "kw=", ToASTNode(jump_stmt->kw));
    PrintASTNodeWithName(depth + 1, "param=", jump_stmt->param);
  } else if (node->type == kASTCondStmt) {
    ASTCondStmt* cond_stmt = ToASTCondStmt(node);
    PrintASTNodeWithName(depth + 1, "cond_expr=", cond_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "true_expr=", cond_stmt->true_expr);
    PrintASTNodeWithName(depth + 1, "false_expr=", cond_stmt->false_expr);
  } else if (node->type == kASTIfStmt) {
    ASTIfStmt* if_stmt = ToASTIfStmt(node);
    PrintASTNodeWithName(depth + 1, "cond_expr=", if_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "true_stmt=", if_stmt->true_stmt);
    PrintASTNodeWithName(depth + 1, "false_stmt=", if_stmt->false_stmt);
  } else if (node->type == kASTWhileStmt) {
    ASTWhileStmt* stmt = ToASTWhileStmt(node);
    PrintASTNodeWithName(depth + 1, "cond_expr=", stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "body_stmt=", stmt->body_stmt);
  } else if (node->type == kASTForStmt) {
    ASTForStmt* for_stmt = ToASTForStmt(node);
    PrintASTNodeWithName(depth + 1, "init_expr=", for_stmt->init_expr);
    PrintASTNodeWithName(depth + 1, "cond_expr=", for_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "updt_expr=", for_stmt->updt_expr);
    PrintASTNodeWithName(depth + 1, "body_stmt=", for_stmt->body_stmt);
  } else if (node->type == kASTILOp) {
    ASTILOp* il_op = ToASTILOp(node);
    PrintfWithPadding(depth + 1, "op=%s", GetILOpTypeName(il_op->op));
    PrintfWithPadding(depth + 1, "dst=%d",
                      il_op->dst ? il_op->dst->vreg_id : 0);
    PrintfWithPadding(depth + 1, "left=%d",
                      il_op->left ? il_op->left->vreg_id : 0);
    PrintfWithPadding(depth + 1, "right=%d",
                      il_op->right ? il_op->right->vreg_id : 0);
    PrintASTNodeWithName(depth + 1, "ast_node=", il_op->ast_node);
  } else if (node->type == kASTKeyword) {
    ASTKeyword* kw = ToASTKeyword(node);
    PrintTokenWithName(depth + 1, "token=", kw->token);
  } else if (node->type == kASTDecltor) {
    ASTDecltor* decltor = ToASTDecltor(node);
    PrintASTNodeWithName(depth + 1,
                         "direct_decltor=", ToASTNode(decltor->direct_decltor));
  } else if (node->type == kASTDirectDecltor) {
    ASTDirectDecltor* direct_decltor = ToASTDirectDecltor(node);
    PrintASTNodeWithName(depth + 1, "direct_decltor=",
                         ToASTNode(direct_decltor->direct_decltor));
    PrintASTNodeWithName(depth + 1, "data=", direct_decltor->data);
  } else if (node->type == kASTIdent) {
    ASTIdent* ident = ToASTIdent(node);
    PrintTokenWithName(depth + 1, "token=", ident->token);
    PrintASTNodeWithName(depth + 1, "local_var=", ToASTNode(ident->local_var));
  } else if (node->type == kASTDecl) {
    ASTDecl* decl = ToASTDecl(node);
    PrintASTNodeWithName(depth + 1, "decl_specs=", ToASTNode(decl->decl_specs));
    PrintASTNodeWithName(depth + 1,
                         "init_decltors=", ToASTNode(decl->init_decltors));
  } else if (node->type == kASTParamDecl) {
    ASTParamDecl* param_decl = ToASTParamDecl(node);
    PrintASTNodeWithName(depth + 1,
                         "decl_specs=", ToASTNode(param_decl->decl_specs));
    PrintASTNodeWithName(depth + 1, "decltor=", ToASTNode(param_decl->decltor));
  } else if (node->type == kASTLabel) {
    ASTLabel* label = ToASTLabel(node);
    PrintfWithPadding(depth + 1, "label_number=%d", label->label_number);
  } else if (node->type == kASTLocalVar) {
    ASTLocalVar* var = ToASTLocalVar(node);
    PrintfWithPadding(depth + 1, "size=%d", var->size);
    PrintfWithPadding(depth + 1, "name=%s", var->name);
  } else {
    Error("PrintASTNode not implemented for type %d (%s)", node->type,
          GetASTTypeName(node));
  }
  PrintfWithPadding(depth, ")");
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
