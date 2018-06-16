#include <stdarg.h>

#include "compilium.h"

struct AST_LIST {
  ASTType type;
  int capacity;
  int size;
  ASTNode* nodes[];
};

ASTNode* ToASTNode(void* node) { return (ASTNode*)node; }

#define GenToAST(Type) \
  AST##Type* ToAST##Type(ASTNode* node) { \
    if (!node || node->type != k##Type) return NULL; \
    return (AST##Type*)node; \
  }

GenToAST(Root);
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

#define GenAllocAST(Type) \
  AST##Type* AllocAST##Type() { \
    AST##Type* node = (AST##Type*)malloc(sizeof(AST##Type)); \
    node->type = k##Type; \
    return node; \
  }

GenAllocAST(Root);
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

void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
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

void PrintfWithPadding(int depth, const char* fmt, ...) {
  PrintASTNodePadding(depth);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

const char* ASTTypeName[kNumOfASTType] = {
    "Root",        "VarDef",     "FuncDecl", "FuncDef",
    "CompStmt",    "kExprBinOp", "kExprVal", "kExprStmt",
    "kReturnStmt", "kForStmt",   "kILOp",
};

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
  if (node->type == kRoot) {
    ASTRoot* root = ToASTRoot(node);
    PrintASTNodeWithName(depth + 1, "root_list=", ToASTNode(root->root_list));
  } else if (node->type == kVarDef) {
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
    PrintASTNodeWithName(depth + 1, "func_decl=", func_def->func_decl);
    PrintASTNodeWithName(depth + 1, "comp_stmt=", func_def->comp_stmt);
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
    PrintfWithPadding(depth + 1, " op=%s", InternalGetILOpTypeStr(il_op->op));
    PrintfWithPadding(depth + 1, " dst=%d", il_op->dst_reg);
    PrintfWithPadding(depth + 1, " left=%d", il_op->left_reg);
    PrintfWithPadding(depth + 1, " right=%d", il_op->right_reg);
  } else {
    Error("PrintASTNode not implemented for type %d", node->type);
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
