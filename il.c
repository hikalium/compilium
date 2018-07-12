#include "compilium.h"

#define REG_NULL 0

const char *ILOpTypeName[kNumOfILOpFunc];

void InitILOpTypeName() {
  ILOpTypeName[kILOpAdd] = "Add";
  ILOpTypeName[kILOpSub] = "Sub";
  ILOpTypeName[kILOpMul] = "Mul";
  ILOpTypeName[kILOpDiv] = "Div";
  ILOpTypeName[kILOpMod] = "Mod";
  ILOpTypeName[kILOpLoadImm] = "LoadImm";
  ILOpTypeName[kILOpLoadIdent] = "LoadIdent";
  ILOpTypeName[kILOpFuncBegin] = "FuncBegin";
  ILOpTypeName[kILOpFuncEnd] = "FuncEnd";
  ILOpTypeName[kILOpReturn] = "Return";
  ILOpTypeName[kILOpCall] = "Call";
  ILOpTypeName[kILOpWriteLocalVar] = "WriteLocalVar";
  ILOpTypeName[kILOpReadLocalVar] = "ReadLocalVar";
}

const char *GetILOpTypeName(ILOpType type) {
  if (kNumOfILOpFunc <= type) return "?";
  return ILOpTypeName[type];
}

int GetRegNumber() {
  static int num = 2;
  return num++;
}

void GenerateILForCompStmt(ASTList *il, ASTNode *node, ASTDict *stack_vars) {
  ASTCompStmt *comp = ToASTCompStmt(node);
  ASTList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
    GenerateIL(il, GetASTNodeAt(stmt_list, i), stack_vars);
  }
}

void GenerateILForFuncDef(ASTList *il, ASTNode *node) {
  ASTFuncDef *def = ToASTFuncDef(node);
  PushASTNodeToList(
      il, ToASTNode(AllocAndInitASTILOp(kILOpFuncBegin, REG_NULL, REG_NULL,
                                        REG_NULL, node)));
  ASTDict *stack_vars = AllocASTDict(8);
  GenerateILForCompStmt(il, ToASTNode(def->comp_stmt), stack_vars);
  PushASTNodeToList(il, ToASTNode(AllocAndInitASTILOp(
                            kILOpFuncEnd, REG_NULL, REG_NULL, REG_NULL, node)));
}

ASTILOp *GenerateILForExprBinOp(ASTList *il, ASTNode *node,
                                ASTDict *stack_vars) {
  int dst = REG_NULL;
  ASTExprBinOp *bin_op = ToASTExprBinOp(node);
  ILOpType il_op_type = kILOpNop;
  if (IsEqualToken(bin_op->op, "+")) {
    il_op_type = kILOpAdd;
  } else if (IsEqualToken(bin_op->op, "-")) {
    il_op_type = kILOpSub;
  } else if (IsEqualToken(bin_op->op, "*")) {
    il_op_type = kILOpMul;
  } else if (IsEqualToken(bin_op->op, "/")) {
    il_op_type = kILOpDiv;
  } else if (IsEqualToken(bin_op->op, "%")) {
    il_op_type = kILOpMod;
  }
  if (il_op_type != kILOpNop) {
    dst = GetRegNumber();
    int il_left = GenerateIL(il, bin_op->left, stack_vars)->dst_reg;
    int il_right = GenerateIL(il, bin_op->right, stack_vars)->dst_reg;
    ASTILOp *il_op =
        AllocAndInitASTILOp(il_op_type, dst, il_left, il_right, node);
    PushASTNodeToList(il, ToASTNode(il_op));
    return il_op;
  } else if (IsEqualToken(bin_op->op, "=")) {
    ASTIdent *left_ident = ToASTIdent(bin_op->left);
    if (left_ident) {
      ASTLocalVar *local_var =
          ToASTLocalVar(FindASTNodeInDict(stack_vars, left_ident->token->str));
      if (local_var) {
        int il_right = GenerateIL(il, bin_op->right, stack_vars)->dst_reg;
        ASTILOp *il_op =
            AllocAndInitASTILOp(kILOpWriteLocalVar, il_right, REG_NULL,
                                il_right, ToASTNode(local_var));
        PushASTNodeToList(il, ToASTNode(il_op));
        return il_op;
      }
      Error("local variable %s not defined here.", left_ident->token->str);
    }
    Error("Left operand of assignment should be an lvalue");
  } else if (IsEqualToken(bin_op->op, ",")) {
    GenerateIL(il, bin_op->left, stack_vars);
    return GenerateIL(il, bin_op->right, stack_vars);
  } else if (IsEqualToken(bin_op->op, "(")) {
    // func_call
    // call_params = [func_addr: ILOp, arg1: ILOp, arg2: ILOp, ...]
    ASTList *call_params = AllocASTList(8);

    // func_addr
    if (bin_op->left->type == kASTIdent) {
      PushASTNodeToList(call_params, bin_op->left);
    } else {
      Error("Calling non-labeled function is not implemented.");
    }

    // args
    if (bin_op->right) {
      ASTList *arg_list = ToASTList(bin_op->right);
      if (!arg_list) Error("arg_list is not an ASTList");
      for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
        ASTNode *node = GetASTNodeAt(arg_list, i);
        PushASTNodeToList(call_params,
                          ToASTNode(GenerateIL(il, node, stack_vars)));
      }
    }
    ASTILOp *il_op_call = AllocAndInitASTILOp(
        kILOpCall, GetRegNumber(), REG_NULL, REG_NULL, ToASTNode(call_params));
    PushASTNodeToList(il, ToASTNode(il_op_call));
    return il_op_call;
  }
  Error("Not implemented GenerateILForExprBinOp (op: %s)", bin_op->op->str);
  return NULL;
}

ASTILOp *GenerateILForConstant(ASTList *il, ASTNode *node,
                               ASTDict *stack_vars) {
  int dst = GetRegNumber();
  ASTILOp *il_op =
      AllocAndInitASTILOp(kILOpLoadImm, dst, REG_NULL, REG_NULL, node);
  PushASTNodeToList(il, ToASTNode(il_op));
  return il_op;
}

ASTILOp *GenerateILForIdent(ASTList *il, ASTNode *node, ASTDict *stack_vars) {
  int dst = GetRegNumber();
  ASTLocalVar *local_var = ToASTLocalVar(
      FindASTNodeInDict(stack_vars, ToASTIdent(node)->token->str));
  ASTILOp *il_op =
      local_var
          ? AllocAndInitASTILOp(kILOpReadLocalVar, dst, REG_NULL, REG_NULL,
                                ToASTNode(local_var))
          : AllocAndInitASTILOp(kILOpLoadIdent, dst, REG_NULL, REG_NULL, node);
  PushASTNodeToList(il, ToASTNode(il_op));
  return il_op;
}

ASTILOp *GenerateILForExprStmt(ASTList *il, ASTNode *node,
                               ASTDict *stack_vars) {
  // https://wiki.osdev.org/System_V_ABI
  const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
  return GenerateIL(il, expr_stmt->expr, stack_vars);
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

ASTILOp *GenerateILForJumpStmt(ASTList *il, ASTNode *node,
                               ASTDict *stack_vars) {
  ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
  if (IsEqualToken(jump_stmt->kw->token, "return")) {
    int expr_reg =
        GenerateILForExprStmt(il, jump_stmt->param, stack_vars)->dst_reg;

    ASTILOp *il_op =
        AllocAndInitASTILOp(kILOpReturn, REG_NULL, expr_reg, REG_NULL, node);
    PushASTNodeToList(il, ToASTNode(il_op));
    return il_op;
  }
  Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
  return NULL;
}

void GenerateILForDecl(ASTList *il, ASTNode *node, ASTDict *stack_vars) {
  ASTDecl *decl = ToASTDecl(node);
  if (!decl) Error("node is not a Decl");
  PrintASTNode(ToASTNode(decl->decl_specs), 0);
  putchar('\n');
  for (int i = 0; i < GetSizeOfASTList(decl->init_decltors); i++) {
    ASTDecltor *decltor = ToASTDecltor(GetASTNodeAt(decl->init_decltors, i));
    const char *ident_str = GetIdentStrFromDecltor(decltor);
    printf("ident: %s\n", ident_str);
    AppendASTNodeToDict(
        stack_vars, ident_str,
        ToASTNode(AllocASTLocalVar(GetSizeOfASTDict(stack_vars) + 1)));
  }
}

ASTILOp *GenerateIL(ASTList *il, ASTNode *node, ASTDict *stack_vars) {
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
    return GenerateILForJumpStmt(il, node, stack_vars);
  } else if (node->type == kASTExprBinOp) {
    return GenerateILForExprBinOp(il, node, stack_vars);
  } else if (node->type == kASTConstant) {
    return GenerateILForConstant(il, node, stack_vars);
  } else if (node->type == kASTExprStmt) {
    return GenerateILForExprStmt(il, node, stack_vars);
  } else if (node->type == kASTIdent) {
    return GenerateILForIdent(il, node, stack_vars);
  } else if (node->type == kASTDecl) {
    GenerateILForDecl(il, node, stack_vars);
    return NULL;
  }
  PrintASTNode(node, 0);
  putchar('\n');
  Error("IL Generation for AST%s is not implemented.", GetASTTypeName(node));
  return NULL;
}
