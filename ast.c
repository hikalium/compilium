#include "compilium.h"

ASTNode *ToASTNode(void *node) { return (ASTNode *)node; }

#define GenToAST(Type)                               \
  AST##Type *ToAST##Type(ASTNode *node) {            \
    if (!node || node->type != k##Type) return NULL; \
    return (AST##Type *)node;                        \
  }

GenToAST(Root) GenToAST(VarDef) GenToAST(FuncDecl) GenToAST(FuncDef)
    GenToAST(CompStmt) GenToAST(ExprBinOp) GenToAST(ExprVal) GenToAST(ExprStmt)
        GenToAST(ReturnStmt) GenToAST(ForStmt) GenToAST(ILOp)
#define GenAllocAST(Type)                                     \
  AST##Type *AllocAST##Type() {                               \
    AST##Type *node = (AST##Type *)malloc(sizeof(AST##Type)); \
    node->type = k##Type;                                     \
    return node;                                              \
  }

            GenAllocAST(Root) GenAllocAST(VarDef) GenAllocAST(FuncDecl)
                GenAllocAST(FuncDef) GenAllocAST(CompStmt)
                    GenAllocAST(ExprBinOp) GenAllocAST(ExprVal)
                        GenAllocAST(ExprStmt) GenAllocAST(ReturnStmt)
                            GenAllocAST(ForStmt) GenAllocAST(ILOp)

                                ASTNode *AllocateASTNodeAsExprVal(
                                    const Token *token) {
  ASTExprVal *node = AllocASTExprVal();
  node->token = token;
  return ToASTNode(node);
}

ASTNode *AllocateASTNodeAsExprBinOp(ASTExprBinOpType op_type) {
  ASTExprBinOp *node = AllocASTExprBinOp();
  node->op_type = op_type;
  node->left = NULL;
  node->right = NULL;
  return ToASTNode(node);
}

void SetOperandOfExprBinOp(ASTExprBinOp *node, ASTNode *left, ASTNode *right) {
  node->left = left;
  node->right = right;
}

ASTNode *AllocateASTNodeAsILOp(ILOpType op, int dst_reg, int left_reg,
                               int right_reg, ASTNode *ast_node) {
  ASTILOp *node = AllocASTILOp();
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

const char *ILOpTypeStr[kNumOfILOpFunc];

const char *InternalGetILOpTypeStr(ILOpType type) {
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

void PrintASTNodeList(ASTNodeList *list, int depth);
void PrintASTNode(ASTNode *node, int depth) {
  if (!node) {
    printf("(Null)");
    return;
  }
  if (node->type == kRoot) {
    ASTRoot *root = ToASTRoot(node);
    printf("(Root:");
    PrintASTNodePadding(depth);
    printf("root_list=");
    PrintASTNodeList(root->root_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kVarDef) {
    ASTVarDef *var_def = ToASTVarDef(node);
    printf("(VarDef:");
    PrintASTNodePadding(depth);
    printf("type=");
    PrintTokenList(var_def->type_tokens);
    PrintASTNodePadding(depth);
    printf("name=%s", var_def->name->str);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDecl) {
    ASTFuncDecl *func_decl = ToASTFuncDecl(node);
    printf("(FuncDecl:");
    PrintASTNodePadding(depth);
    printf("type_and_name=");
    PrintASTNode(func_decl->type_and_name, depth + 1);
    PrintASTNodePadding(depth);
    printf("arg_list=");
    PrintASTNodeList(func_decl->arg_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDef) {
    ASTFuncDef *func_def = ToASTFuncDef(node);
    printf("(FuncDef:");
    //
    PrintASTNodePadding(depth);
    printf("func_decl=");
    PrintASTNode(func_def->func_decl, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf("comp_stmt=");
    PrintASTNode(func_def->comp_stmt, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kCompStmt) {
    ASTCompStmt *comp_stmt = ToASTCompStmt(node);
    printf("(CompStatement:");
    //
    PrintASTNodePadding(depth);
    printf("(body=");
    PrintASTNodeList(comp_stmt->stmt_list, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExprBinOp) {
    ASTExprBinOp *expr_bin_op = ToASTExprBinOp(node);
    printf("(ExprBinOp:");
    //
    PrintASTNodePadding(depth);
    printf("op_type=%d", expr_bin_op->op_type);
    //
    PrintASTNodePadding(depth);
    printf("left=");
    PrintASTNode(expr_bin_op->left, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf("right=");
    PrintASTNode(expr_bin_op->right, depth + 1);
    //
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExprVal) {
    ASTExprVal *expr_val = ToASTExprVal(node);
    printf("(ExprVal:");
    PrintASTNodePadding(depth);
    printf("token=");
    PrintToken(expr_val->token);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExprStmt) {
    ASTExprStmt *expr_stmt = ToASTExprStmt(node);
    printf("(ExprStmt:");
    PrintASTNodePadding(depth);
    printf("expression=");
    PrintASTNode(expr_stmt->expr, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kReturnStmt) {
    ASTReturnStmt *return_stmt = ToASTReturnStmt(node);
    printf("(ReturnStmt:");
    PrintASTNodePadding(depth);
    printf("expr_stmt=");
    PrintASTNode(return_stmt->expr_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kForStmt) {
    ASTForStmt *for_stmt = ToASTForStmt(node);
    printf("(ForStatement:");
    PrintASTNodePadding(depth);
    printf("init_expr=");
    PrintASTNode(for_stmt->init_expr, depth + 1);
    PrintASTNodePadding(depth);
    printf("cond_expr=");
    PrintASTNode(for_stmt->cond_expr, depth + 1);
    PrintASTNodePadding(depth);
    printf("updt_expr=");
    PrintASTNode(for_stmt->updt_expr, depth + 1);
    PrintASTNodePadding(depth);
    printf("body_comp_stmt=");
    PrintASTNode(for_stmt->body_comp_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kILOp) {
    ASTILOp *il_op = ToASTILOp(node);
    printf("(ILOp:");
    printf(" op=%s", InternalGetILOpTypeStr(il_op->op));
    printf(" dst=%d", il_op->dst_reg);
    printf(" left=%d", il_op->left_reg);
    printf(" right=%d", il_op->right_reg);
    printf(")");

  } else {
    Error("PrintASTNode not implemented for type %d", node->type);
  }
}

struct AST_NODE_LIST {
  int capacity;
  int size;
  ASTNode *nodes[];
};

ASTNodeList *AllocateASTNodeList(int capacity) {
  ASTNodeList *list =
      malloc(sizeof(ASTNodeList) + sizeof(ASTNode *) * capacity);
  list->capacity = capacity;
  list->size = 0;
  return list;
}

void PushASTNodeToList(ASTNodeList *list, ASTNode *node) {
  if (list->size >= list->capacity) {
    Error("No more space in ASTNodeList");
  }
  list->nodes[list->size++] = node;
}

ASTNode *PopASTNodeFromList(ASTNodeList *list) {
  if (list->size <= 0) {
    Error("Trying to pop empty ASTNodeList");
  }
  return list->nodes[--list->size];
}

ASTNode *GetASTNodeAt(const ASTNodeList *list, int index) {
  if (index < 0 || list->size <= index) {
    Error("ASTNodeList: Trying to read index out of bound");
  }
  return list->nodes[index];
}

int GetSizeOfASTNodeList(const ASTNodeList *list) { return list->size; }

ASTNode *GetLastASTNode(const ASTNodeList *list) {
  return GetASTNodeAt(list, GetSizeOfASTNodeList(list) - 1);
}

void PrintASTNodeList(ASTNodeList *list, int depth) {
  putchar('[');
  PrintASTNodePadding(depth);
  for (int i = 0; i < list->size; i++) {
    PrintASTNode(list->nodes[i], depth + 1);
    PrintASTNodePadding(depth);
  }
  putchar(']');
}
