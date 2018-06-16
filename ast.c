#include <stdarg.h>

#include "compilium.h"

struct AST_LIST {
  ASTType type;
  int capacity;
  int size;
  ASTNode* nodes[];
};

const char* ASTTypeName[kNumOfASTType];

void InitASTTypeName() {
  ASTTypeName[kVarDef] = "VarDef";
  ASTTypeName[kFuncDecl] = "FuncDecl";
  ASTTypeName[kFuncDef] = "FuncDef";
  ASTTypeName[kCompStmt] = "CompStmt";
  ASTTypeName[kExprBinOp] = "ExprBinOp";
  ASTTypeName[kExprStmt] = "ExprStmt";
  ASTTypeName[kReturnStmt] = "ReturnStmt";
  ASTTypeName[kForStmt] = "ForStmt";
  ASTTypeName[kILOp] = "ILOp";
  ASTTypeName[kList] = "List";
  ASTTypeName[kKeyword] = "Keyword";
  ASTTypeName[kDecltor] = "Decltor";
  ASTTypeName[kDirectDecltor] = "DirectDecltor";
  ASTTypeName[kIdent] = "Ident";
}

const char* ILOpTypeStr[kNumOfILOpFunc];

const char* InternalGetILOpTypeStr(ILOpType type) {
  if (!ILOpTypeStr[0]) {
    ILOpTypeStr[kILOpAdd] = "Add";
    ILOpTypeStr[kILOpSub] = "Sub";
    ILOpTypeStr[kILOpMul] = "Mul";
    ILOpTypeStr[kILOpLoadImm] = "LoadImm";
    ILOpTypeStr[kILOpFuncBegin] = "FuncBegin";
    ILOpTypeStr[kILOpFuncEnd] = "FuncEnd";
    ILOpTypeStr[kILOpReturn] = "Return";
  }
  if (kNumOfILOpFunc <= type) return "?";
  return ILOpTypeStr[type];
}

ASTNode* ToASTNode(void* node) { return (ASTNode*)node; }

#define GenToAST(Type) \
  AST##Type* ToAST##Type(ASTNode* node) { \
    if (!node || node->type != k##Type) return NULL; \
    return (AST##Type*)node; \
  }

GenToAST(VarDef);
GenToAST(FuncDecl);
GenToAST(FuncDef);
GenToAST(CompStmt);
GenToAST(ExprBinOp);
GenToAST(ExprVal);
GenToAST(ExprStmt);
GenToAST(ReturnStmt);
GenToAST(ForStmt);
GenToAST(ILOp);
GenToAST(List);
GenToAST(Keyword);
GenToAST(Decltor);
GenToAST(DirectDecltor);
GenToAST(Ident);

#define GenAllocAST(Type) \
  AST##Type* AllocAST##Type() { \
    AST##Type* node = (AST##Type*)malloc(sizeof(AST##Type)); \
    node->type = k##Type; \
    return node; \
  }

GenAllocAST(VarDef);
GenAllocAST(FuncDecl);
GenAllocAST(FuncDef);
GenAllocAST(CompStmt);
GenAllocAST(ExprBinOp);
GenAllocAST(ExprVal);
GenAllocAST(ExprStmt);
GenAllocAST(ReturnStmt);
GenAllocAST(ForStmt);
GenAllocAST(ILOp);
GenAllocAST(Keyword);
GenAllocAST(Decltor);
GenAllocAST(DirectDecltor);
GenAllocAST(Ident);

ASTList* AllocASTList(int capacity) {
  ASTList* list = malloc(sizeof(ASTList) + sizeof(ASTNode*) * capacity);
  list->type = kList;
  list->capacity = capacity;
  list->size = 0;
  return list;
}

ASTNode* AllocateASTNodeAsExprVal(const Token* token) {
  ASTExprVal* node = AllocASTExprVal();
  node->token = token;
  return ToASTNode(node);
}

ASTNode* AllocateASTNodeAsExprBinOp(ASTExprBinOpType op_type) {
  ASTExprBinOp* node = AllocASTExprBinOp();
  node->op_type = op_type;
  node->left = NULL;
  node->right = NULL;
  return ToASTNode(node);
}

void SetOperandOfExprBinOp(ASTExprBinOp* node, ASTNode* left, ASTNode* right) {
  node->left = left;
  node->right = right;
}

ASTNode* AllocateASTNodeAsILOp(ILOpType op, int dst_reg, int left_reg,
                               int right_reg, ASTNode* ast_node) {
  ASTILOp* node = AllocASTILOp();
  node->op = op;
  node->dst_reg = dst_reg;
  node->left_reg = left_reg;
  node->right_reg = right_reg;
  node->ast_node = ast_node;
  return ToASTNode(node);
}

const char* GetIdentStrFromDirectDecltor(ASTDirectDecltor* direct_decltor) {
  if (!direct_decltor) return NULL;
  if (direct_decltor->direct_decltor)
    return GetIdentStrFromDirectDecltor(direct_decltor->direct_decltor);
  ASTIdent* ident = ToASTIdent(direct_decltor->data);
  if (!ident) return NULL;
  return ident->token->str;
}

const char* GetIdentStrFromDecltor(ASTDecltor* decltor) {
  if (!decltor) return NULL;
  return GetIdentStrFromDirectDecltor(decltor->direct_decltor);
}

const char* GetFuncNameStrFromFuncDef(ASTFuncDef* func_def) {
  if (!func_def) return NULL;
  return GetIdentStrFromDecltor(func_def->decltor);
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
  if (node->type == kList) {
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
  if (node->type == kVarDef) {
    ASTVarDef* var_def = ToASTVarDef(node);
    PrintTokenListWithName(depth + 1, "type=", var_def->type_tokens);
    PrintfWithPadding(depth + 1, "name=%s", var_def->name->str);
  } else if (node->type == kFuncDecl) {
    ASTFuncDecl* func_decl = ToASTFuncDecl(node);
    PrintASTNodeWithName(depth + 1, "type_and_name=", func_decl->type_and_name);
    PrintASTNodeWithName(depth + 1,
                         "arg_list=", ToASTNode(func_decl->arg_list));
  } else if (node->type == kFuncDef) {
    ASTFuncDef* func_def = ToASTFuncDef(node);
    PrintASTNodeWithName(depth + 1,
                         "decl_specs=", ToASTNode(func_def->decl_specs));
    PrintASTNodeWithName(depth + 1, "decltor=", ToASTNode(func_def->decltor));
    PrintASTNodeWithName(depth + 1,
                         "comp_stmt=", ToASTNode(func_def->comp_stmt));
  } else if (node->type == kCompStmt) {
    ASTCompStmt* comp_stmt = ToASTCompStmt(node);
    PrintASTNodeWithName(depth + 1, "body=", ToASTNode(comp_stmt->stmt_list));
  } else if (node->type == kExprBinOp) {
    ASTExprBinOp* expr_bin_op = ToASTExprBinOp(node);
    PrintfWithPadding(depth + 1, "op_type=%d", expr_bin_op->op_type);
    PrintASTNodeWithName(depth + 1, "left=", expr_bin_op->left);
    PrintASTNodeWithName(depth + 1, "right=", expr_bin_op->right);
  } else if (node->type == kExprVal) {
    ASTExprVal* expr_val = ToASTExprVal(node);
    PrintTokenWithName(depth + 1, "token=", expr_val->token);
  } else if (node->type == kExprStmt) {
    ASTExprStmt* expr_stmt = ToASTExprStmt(node);
    PrintASTNodeWithName(depth + 1, "expression=", expr_stmt->expr);
  } else if (node->type == kReturnStmt) {
    ASTReturnStmt* return_stmt = ToASTReturnStmt(node);
    PrintASTNodeWithName(depth + 1, "expr_stmt=", return_stmt->expr_stmt);
  } else if (node->type == kForStmt) {
    ASTForStmt* for_stmt = ToASTForStmt(node);
    PrintASTNodeWithName(depth + 1, "init_expr=", for_stmt->init_expr);
    PrintASTNodeWithName(depth + 1, "cond_expr=", for_stmt->cond_expr);
    PrintASTNodeWithName(depth + 1, "updt_expr=", for_stmt->updt_expr);
    PrintASTNodeWithName(depth + 1,
                         "body_comp_stmt=", for_stmt->body_comp_stmt);
  } else if (node->type == kILOp) {
    ASTILOp* il_op = ToASTILOp(node);
    PrintfWithPadding(depth + 1, "op=%s", InternalGetILOpTypeStr(il_op->op));
    PrintfWithPadding(depth + 1, "dst=%d", il_op->dst_reg);
    PrintfWithPadding(depth + 1, "left=%d", il_op->left_reg);
    PrintfWithPadding(depth + 1, "right=%d", il_op->right_reg);
    PrintASTNodeWithName(depth + 1, "ast_node=", il_op->ast_node);
  } else if (node->type == kKeyword) {
    ASTKeyword* kw = ToASTKeyword(node);
    PrintTokenWithName(depth + 1, "token=", kw->token);
  } else if (node->type == kDecltor) {
    ASTDecltor* decltor = ToASTDecltor(node);
    PrintASTNodeWithName(depth + 1,
                         "direct_decltor=", ToASTNode(decltor->direct_decltor));
  } else if (node->type == kDirectDecltor) {
    ASTDirectDecltor* direct_decltor = ToASTDirectDecltor(node);
    PrintASTNodeWithName(depth + 1, "direct_decltor=",
                         ToASTNode(direct_decltor->direct_decltor));
    PrintASTNodeWithName(depth + 1, "data=", direct_decltor->data);
  } else if (node->type == kIdent) {
    ASTIdent* ident = ToASTIdent(node);
    PrintTokenWithName(depth + 1, "token=", ident->token);
  } else {
    Error("PrintASTNode not implemented for type %d (%s)", node->type,
          ASTTypeName[node->type] ? ASTTypeName[node->type] : "?");
  }
  PrintfWithPadding(depth, ")");
}

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

int GetSizeOfASTList(const ASTList* list) { return list->size; }

ASTNode* GetLastASTNode(const ASTList* list) {
  return GetASTNodeAt(list, GetSizeOfASTList(list) - 1);
}
