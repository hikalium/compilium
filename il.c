#include "compilium.h"

#define REG_NULL 0

const char *ILOpTypeName[kNumOfILOpFunc];

void InitILOpTypeName() {
  ILOpTypeName[kILOpAdd] = "Add";
  ILOpTypeName[kILOpSub] = "Sub";
  ILOpTypeName[kILOpMul] = "Mul";
  ILOpTypeName[kILOpLoadImm] = "LoadImm";
  ILOpTypeName[kILOpLoadIdent] = "LoadIdent";
  ILOpTypeName[kILOpFuncBegin] = "FuncBegin";
  ILOpTypeName[kILOpFuncEnd] = "FuncEnd";
  ILOpTypeName[kILOpReturn] = "Return";
  ILOpTypeName[kILOpCall] = "Call";
}

const char *GetILOpTypeName(ILOpType type) {
  if (kNumOfILOpFunc <= type) return "?";
  return ILOpTypeName[type];
}

int GetRegNumber() {
  static int num = 0;
  return ++num;
}

void GenerateILForCompStmt(ASTList *il, ASTNode *node) {
  ASTCompStmt *comp = ToASTCompStmt(node);
  ASTList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
    GenerateIL(il, GetASTNodeAt(stmt_list, i));
  }
}

void GenerateILForFuncDef(ASTList *il, ASTNode *node) {
  ASTFuncDef *def = ToASTFuncDef(node);
  PushASTNodeToList(il, ToASTNode(AllocAndInitASTILOp(kILOpFuncBegin, REG_NULL,
                                              REG_NULL, REG_NULL, node)));
  GenerateILForCompStmt(il, ToASTNode(def->comp_stmt));
  PushASTNodeToList(il, ToASTNode(AllocAndInitASTILOp(kILOpFuncEnd, REG_NULL, REG_NULL,
                                              REG_NULL, node)));
}

ASTILOp *GenerateILForExprBinOp(ASTList *il, ASTNode *node) {
  int dst = REG_NULL;
  ASTExprBinOp *bin_op = ToASTExprBinOp(node);
  ILOpType il_op_type = kILOpNop;
  if (IsEqualToken(bin_op->op, "+")) {
    il_op_type = kILOpAdd;
  } else if (IsEqualToken(bin_op->op, "-")) {
    il_op_type = kILOpSub;
  } else if (IsEqualToken(bin_op->op, "*")) {
    il_op_type = kILOpMul;
  }
  if (il_op_type != kILOpNop) {
    dst = GetRegNumber();
    int il_left = GenerateIL(il, bin_op->left)->dst_reg;
    int il_right = GenerateIL(il, bin_op->right)->dst_reg;
    ASTILOp *il_op =
        AllocAndInitASTILOp(il_op_type, dst, il_left, il_right, node);
    PushASTNodeToList(il, ToASTNode(il_op));
    return il_op;
  } else if (IsEqualToken(bin_op->op, ",")) {
    GenerateIL(il, bin_op->left);
    return GenerateIL(il, bin_op->right);
  } else if (IsEqualToken(bin_op->op, "(")) {
    // func_call
    // call_params = [func_addr: ILOp, arg1: ILOp, arg2: ILOp, ...]
    ASTList *call_params = AllocASTList(8);

    // func_addr
    if(bin_op->left->type == kASTIdent){
      PushASTNodeToList(call_params, bin_op->left);
    } else{
      Error("Calling non-labeled function is not implemented.");
    }

    // args
    if (bin_op->right) {
      ASTList *arg_list = ToASTList(bin_op->right);
      if (!arg_list) Error("arg_list is not an ASTList");
      for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
        ASTNode *node = GetASTNodeAt(arg_list, i);
        PushASTNodeToList(call_params, ToASTNode(GenerateIL(il, node)));
      }
    }
    ASTILOp *il_op_call =
        AllocAndInitASTILOp(kILOpCall, GetRegNumber(), REG_NULL, REG_NULL, ToASTNode(call_params));
    PushASTNodeToList(il, ToASTNode(il_op_call));
    return il_op_call;
  }
  Error("Not implemented GenerateILForExprBinOp (op: %s)", bin_op->op->str);
  return NULL;
}

ASTILOp *GenerateILForConstant(ASTList *il, ASTNode *node) {
  int dst = GetRegNumber();
  ASTILOp *il_op =
      AllocAndInitASTILOp(kILOpLoadImm, dst, REG_NULL, REG_NULL, node);
  PushASTNodeToList(il, ToASTNode(il_op));
  return il_op;
}

ASTILOp *GenerateILForIdent(ASTList *il, ASTNode *node) {
  int dst = GetRegNumber();
  ASTILOp *il_op =
      AllocAndInitASTILOp(kILOpLoadIdent, dst, REG_NULL, REG_NULL, node);
  PushASTNodeToList(il, ToASTNode(il_op));
  return il_op;
}

ASTILOp *GenerateILForExprStmt(ASTList *il, ASTNode *node) {
  // https://wiki.osdev.org/System_V_ABI
  const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
  return GenerateIL(il, expr_stmt->expr);
  /*
  const TokenList *token_list = expr_stmt->expr;
  if (token_list->used == 1 && token_list->tokens[0]->type == kInteger) {
    char *p;
    const char *s = token_list->tokens[0]->str;
    int var = strtol(s, &p, 0);
    if (!(s[0] != 0 && *p == 0)) {
      Error("%s is not valid as integer.", s);
    }
    fprintf(fp, "mov     rax, %d\n", var);
  } else if (token_list->used == 4 &&
             IsEqualToken(token_list->tokens[0], "puts") &&
             IsEqualToken(token_list->tokens[1], "(") &&
             token_list->tokens[2]->type == kStringLiteral &&
             IsEqualToken(token_list->tokens[3], ")")) {
    int label_for_skip = GetLabelNumber();
    int label_str = GetLabelNumber();
    fprintf(fp, "jmp L%d\n", label_for_skip);
    fprintf(fp, "L%d:\n", label_str);
    fprintf(fp, ".asciz  \"%s\"\n", token_list->tokens[2]->str);
    fprintf(fp, "L%d:\n", label_for_skip);
    fprintf(fp, "sub     rsp, 16\n");
    fprintf(fp, "lea     rdi, [rip + L%d]\n", label_str);
    fprintf(fp, "call    _%s\n", token_list->tokens[0]->str);
    fprintf(fp, "add     rsp, 16\n");
    //Error("GenerateILForExprStmt: puts case ");


  } else {
    Error("GenerateILForExprStmt: Not implemented ");
  }
  */
}

ASTILOp *GenerateILForJumpStmt(ASTList *il, ASTNode *node) {
  ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
  if (IsEqualToken(jump_stmt->kw->token, "return")) {
    int expr_reg = GenerateILForExprStmt(il, jump_stmt->param)->dst_reg;

    ASTILOp *il_op =
        AllocAndInitASTILOp(kILOpReturn, REG_NULL, expr_reg, REG_NULL, node);
    PushASTNodeToList(il, ToASTNode(il_op));
    return il_op;
  }
  Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
  return NULL;
}

ASTILOp *GenerateIL(ASTList *il, ASTNode *node) {
  printf("GenerateIL: AST%s...\n", GetASTTypeName(node));
  if (node->type == kASTList) {
    // translation-unit
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kASTFuncDef) {
        GenerateILForFuncDef(il, child_node);
      }
    }
    return NULL;
  } else if (node->type == kASTJumpStmt) {
    return GenerateILForJumpStmt(il, node);
  } else if (node->type == kASTExprBinOp) {
    return GenerateILForExprBinOp(il, node);
  } else if (node->type == kASTConstant) {
    return GenerateILForConstant(il, node);
  } else if (node->type == kASTExprStmt) {
    return GenerateILForExprStmt(il, node);
  } else if (node->type == kASTIdent) {
    return GenerateILForIdent(il, node);
  }
  PrintASTNode(node, 0);
  Error("Generation for AST%s is not implemented.", GetASTTypeName(node));
  return NULL;
}
