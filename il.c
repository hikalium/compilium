#include <stdio.h>
#include <stdlib.h>

#include "compilium.h"

int next_vreg_id = 1;
Register *AllocRegister() {
  Register *reg = malloc(sizeof(Register));
  reg->vreg_id = next_vreg_id++;
  reg->spill_index = 0;
  reg->real_reg = 0;
  return reg;
}

const char *ILOpTypeName[kNumOfILOpFunc];

void InitILOpTypeName() {
  ILOpTypeName[kILOpNop] = "Nop";
  ILOpTypeName[kILOpAdd] = "Add";
  ILOpTypeName[kILOpSub] = "Sub";
  ILOpTypeName[kILOpMul] = "Mul";
  ILOpTypeName[kILOpDiv] = "Div";
  ILOpTypeName[kILOpMod] = "Mod";
  ILOpTypeName[kILOpAnd] = "And";
  ILOpTypeName[kILOpXor] = "Xor";
  ILOpTypeName[kILOpOr] = "Or";
  ILOpTypeName[kILOpNot] = "Not";
  ILOpTypeName[kILOpNegate] = "Negate";
  ILOpTypeName[kILOpLogicalAnd] = "LogicalAnd";
  ILOpTypeName[kILOpLogicalOr] = "LogicalOr";
  ILOpTypeName[kILOpLogicalNot] = "LogicalNot";
  ILOpTypeName[kILOpShiftLeft] = "ShiftLeft";
  ILOpTypeName[kILOpShiftRight] = "ShiftRight";
  ILOpTypeName[kILOpIncrement] = "Increment";
  ILOpTypeName[kILOpDecrement] = "Decrement";
  ILOpTypeName[kILOpCmpG] = "CmpG";
  ILOpTypeName[kILOpCmpGE] = "CmpGE";
  ILOpTypeName[kILOpCmpL] = "CmpL";
  ILOpTypeName[kILOpCmpLE] = "CmpLE";
  ILOpTypeName[kILOpCmpE] = "CmpE";
  ILOpTypeName[kILOpCmpNE] = "CmpNE";
  ILOpTypeName[kILOpLoad8] = "Load8";
  ILOpTypeName[kILOpLoad64] = "Load64";
  ILOpTypeName[kILOpStore8] = "Store8";
  ILOpTypeName[kILOpStore64] = "Store64";
  ILOpTypeName[kILOpLoadImm] = "LoadImm";
  ILOpTypeName[kILOpLoadIdent] = "LoadIdent";
  ILOpTypeName[kILOpLoadArg] = "LoadArg";
  ILOpTypeName[kILOpFuncBegin] = "FuncBegin";
  ILOpTypeName[kILOpFuncEnd] = "FuncEnd";
  ILOpTypeName[kILOpReturn] = "Return";
  ILOpTypeName[kILOpCall] = "Call";
  ILOpTypeName[kILOpCallParam] = "CallParam";
  ILOpTypeName[kILOpLoadLocalVarAddr] = "LoadLocalVarAddr";
  ILOpTypeName[kILOpLabel] = "Label";
  ILOpTypeName[kILOpJmp] = "Jmp";
  ILOpTypeName[kILOpJmpIfZero] = "JmpIfZero";
  ILOpTypeName[kILOpJmpIfNotZero] = "JmpIfNotZero";
  ILOpTypeName[kILOpSetLogicalValue] = "SetLogicalValue";
}

const char *GetILOpTypeName(ILOpType type) {
  if (kNumOfILOpFunc <= type) return "?";
  return ILOpTypeName[type];
}

ASTILOp *AllocAndInitASTILOp(ILOpType op, Register *dst, Register *left,
                             Register *right, ASTNode *ast_node) {
  ASTILOp *node = AllocASTILOp();
  node->op = op;
  node->dst = dst;
  node->left = left;
  node->right = right;
  node->ast_node = ast_node;
  return node;
}

void EmitILOp(ASTList *il, ILOpType type, Register *dst, Register *left,
              Register *right, ASTNode *node) {
  ASTILOp *op = AllocAndInitASTILOp(type, dst, left, right, node);
  PushASTNodeToList(il, ToASTNode(op));
}

// generators
void GenerateILFor(ASTList *il, Register *dst, ASTNode *node);
void GenerateILForLocalVar(ASTList *il, Register *dst, ASTNode *node);

void GenerateILForCompStmt(ASTList *il, Register *dst, ASTNode *node) {
  ASTCompStmt *comp = ToASTCompStmt(node);
  ASTList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
    GenerateILFor(il, AllocRegister(), GetASTNodeAt(stmt_list, i));
  }
}

void GenerateILForFuncDef(ASTList *il, Register *dst, ASTNode *node) {
  EmitILOp(il, kILOpFuncBegin, NULL, NULL, NULL, node);
  ASTFuncDef *def = ToASTFuncDef(node);
  ASTDirectDecltor *args_decltor = def->decltor->direct_decltor;
  ASTList *param_decl_list = ToASTList(args_decltor->data);

  if (param_decl_list) {
    PrintASTNode(ToASTNode(param_decl_list), 0);
    // TODO: Simplify this
    Register *var_regs[8];
    ASTLocalVar *local_vars[8];
    for (int i = 0; i < GetSizeOfASTList(param_decl_list); i++) {
      local_vars[i] = ToASTLocalVar(GetASTNodeAt(param_decl_list, i));
      var_regs[i] = AllocRegister();
      EmitILOp(il, kILOpLoadArg, var_regs[i], NULL, NULL,
               ToASTNode(local_vars[i]));
    }
    for (int i = 0; i < GetSizeOfASTList(param_decl_list); i++) {
      Register *var_addr = AllocRegister();
      EmitILOp(il, kILOpLoadLocalVarAddr, var_addr, NULL, NULL,
               ToASTNode(local_vars[i]));
      EmitILOp(il, kILOpStore64, var_addr, var_regs[i], NULL, NULL);
    }
  }
  GenerateILForCompStmt(il, AllocRegister(), ToASTNode(def->comp_stmt));
  EmitILOp(il, kILOpFuncEnd, NULL, NULL, NULL, node);
}

typedef struct {
  const char *str;
  ILOpType il_op_type;
} PairOfStrAndILOpType;

PairOfStrAndILOpType bin_op_list[] = {
    {"+", kILOpAdd},         {"-", kILOpSub},    {"*", kILOpMul},
    {"/", kILOpDiv},         {"%", kILOpMod},    {"<<", kILOpShiftLeft},
    {">>", kILOpShiftRight}, {">", kILOpCmpG},   {">=", kILOpCmpGE},
    {"<", kILOpCmpL},        {"<=", kILOpCmpLE}, {"==", kILOpCmpE},
    {"!=", kILOpCmpNE},      {"&", kILOpAnd},    {"^", kILOpXor},
    {"|", kILOpOr},          {NULL, kILOpNop},
};

PairOfStrAndILOpType comp_assign_op_list[] = {
    {"*=", kILOpMul},         {"/=", kILOpDiv}, {"%=", kILOpMod},
    {"+=", kILOpAdd},         {"-=", kILOpSub}, {"<<=", kILOpShiftLeft},
    {">>=", kILOpShiftRight}, {"&=", kILOpAnd}, {"^=", kILOpXor},
    {"|=", kILOpOr},          {NULL, kILOpNop},
};

Register *GenerateILForAssignmentOp(ASTList *il, ASTNode *left,
                                    Register *rvalue) {
  ASTIdent *left_ident = ToASTIdent(left);
  if (left_ident->local_var) {
    Register *var_addr = AllocRegister();
    EmitILOp(il, kILOpLoadLocalVarAddr, var_addr, NULL, NULL,
             ToASTNode(left_ident->local_var));
    EmitILOp(il, kILOpStore64, var_addr, rvalue, NULL, NULL);
    return rvalue;
  }
  Error("Left operand of assignment should be an lvalue");
  return NULL;
}

Register *GenerateILForExprUnaryPreOp(ASTList *il, Register *dst,
                                      ASTNode *node) {
  ASTExprUnaryPreOp *op = ToASTExprUnaryPreOp(node);
  Register *rvalue = AllocRegister();
  if (IsEqualToken(op->op, "+")) {
    GenerateILFor(il, dst, op->expr);
    return dst;
  } else if (IsEqualToken(op->op, "-")) {
    GenerateILFor(il, rvalue, op->expr);
    EmitILOp(il, kILOpNegate, dst, rvalue, NULL, node);
    return dst;
  } else if (IsEqualToken(op->op, "~")) {
    GenerateILFor(il, rvalue, op->expr);
    EmitILOp(il, kILOpNot, dst, rvalue, NULL, node);
    return dst;
  } else if (IsEqualToken(op->op, "!")) {
    GenerateILFor(il, rvalue, op->expr);
    EmitILOp(il, kILOpLogicalNot, dst, rvalue, NULL, node);
    return dst;
  } else if (IsEqualToken(op->op, "++")) {
    // TODO: Support pointer
    GenerateILFor(il, rvalue, op->expr);
    EmitILOp(il, kILOpIncrement, dst, rvalue, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, dst);
    return dst;
  } else if (IsEqualToken(op->op, "--")) {
    // TODO: Support pointer
    GenerateILFor(il, rvalue, op->expr);
    EmitILOp(il, kILOpDecrement, dst, rvalue, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, dst);
    return dst;
  } else if (IsEqualToken(op->op, "&")) {
    ASTIdent *left_ident = ToASTIdent(op->expr);
    EmitILOp(il, kILOpLoadLocalVarAddr, dst, NULL, dst,
             ToASTNode(left_ident->local_var));
    return dst;
  } else if (IsEqualToken(op->op, "*")) {
    GenerateILFor(il, rvalue, op->expr);
    ASTIdent *left_ident = ToASTIdent(op->expr);
    ASTLocalVar *local_var = left_ident->local_var;
    if (local_var) {
      int size = GetSizeOfType(GetDereferencedTypeOf(local_var->var_type));
      printf("size after deref = %d\n", size);
      if (size == 8) {
        EmitILOp(il, kILOpLoad64, dst, rvalue, NULL, node);
        return dst;
      } else if (size == 1) {
        EmitILOp(il, kILOpLoad8, dst, rvalue, NULL, node);
        return dst;
      }
      Error("Deref of size %d is not implemented", size);
    }
  }
  Error("GenerateILForExprUnaryPreOp: Not impl %s", op->op->str);
  return NULL;
}

Register *GenerateILForExprUnaryPostOp(ASTList *il, Register *dst,
                                       ASTNode *node) {
  ASTExprUnaryPostOp *op = ToASTExprUnaryPostOp(node);
  if (IsEqualToken(op->op, "++")) {
    Register *inc_result = AllocRegister();

    GenerateILFor(il, dst, op->expr);
    EmitILOp(il, kILOpIncrement, inc_result, dst, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, inc_result);
    return dst;
  } else if (IsEqualToken(op->op, "--")) {
    Register *dec_result = AllocRegister();

    GenerateILFor(il, dst, op->expr);
    EmitILOp(il, kILOpDecrement, dec_result, dst, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, dec_result);
    return dst;
  }
  Error("Not impl");
  return NULL;
}

Register *GenerateILForExprBinOp(ASTList *il, Register *dst, ASTNode *node) {
  ASTExprBinOp *bin_op = ToASTExprBinOp(node);
  ILOpType il_op_type = kILOpNop;
  Register *left = AllocRegister();
  Register *right = AllocRegister();

  for (int i = 0; bin_op_list[i].str; i++) {
    if (IsEqualToken(bin_op->op, bin_op_list[i].str)) {
      il_op_type = bin_op_list[i].il_op_type;
      break;
    }
  }
  if (il_op_type != kILOpNop) {
    GenerateILFor(il, left, bin_op->left);
    GenerateILFor(il, right, bin_op->right);
    EmitILOp(il, il_op_type, dst, left, right, node);
    return dst;
  }

  for (int i = 0; comp_assign_op_list[i].str; i++) {
    if (IsEqualToken(bin_op->op, comp_assign_op_list[i].str)) {
      il_op_type = comp_assign_op_list[i].il_op_type;
      break;
    }
  }
  if (il_op_type != kILOpNop) {
    GenerateILFor(il, left, bin_op->left);
    GenerateILFor(il, right, bin_op->right);
    EmitILOp(il, il_op_type, dst, left, right, node);
    GenerateILForAssignmentOp(il, bin_op->left, dst);
    return dst;
  }

  if (IsEqualToken(bin_op->op, "&&")) {
    ASTLabel *label = AllocASTLabel();

    GenerateILFor(il, left, bin_op->left);
    EmitILOp(il, kILOpSetLogicalValue, dst, left, NULL, NULL);
    EmitILOp(il, kILOpJmpIfZero, NULL, left, NULL, ToASTNode(label));
    GenerateILFor(il, right, bin_op->right);
    EmitILOp(il, kILOpSetLogicalValue, dst, right, NULL, NULL);
    EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(label));
    return dst;
  } else if (IsEqualToken(bin_op->op, "||")) {
    ASTLabel *label = AllocASTLabel();

    GenerateILFor(il, left, bin_op->left);
    EmitILOp(il, kILOpSetLogicalValue, dst, left, NULL, NULL);
    EmitILOp(il, kILOpJmpIfNotZero, NULL, left, NULL, ToASTNode(label));
    GenerateILFor(il, right, bin_op->right);
    EmitILOp(il, kILOpSetLogicalValue, dst, right, NULL, NULL);
    EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(label));
    return dst;
  } else if (IsEqualToken(bin_op->op, "=")) {
    GenerateILFor(il, dst, bin_op->right);
    GenerateILForAssignmentOp(il, bin_op->left, dst);
    return dst;
  } else if (IsEqualToken(bin_op->op, ",")) {
    GenerateILFor(il, AllocRegister(), bin_op->left);
    GenerateILFor(il, dst, bin_op->right);
    return dst;
  } else if (IsEqualToken(bin_op->op, "(")) {
    if (bin_op->left->type != kASTIdent) {
      Error("Calling non-labeled function is not implemented.");
    }
    if (bin_op->right) {
      ASTList *arg_list = ToASTList(bin_op->right);
      if (!arg_list) Error("arg_list is not an ASTList");
      Register *param_values[8];
      if (GetSizeOfASTList(arg_list) > 8) Error("Too many params");
      for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
        ASTNode *node = GetASTNodeAt(arg_list, i);
        param_values[i] = AllocRegister();
        GenerateILFor(il, param_values[i], node);
      }
      for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
        EmitILOp(il, kILOpCallParam, NULL, param_values[i], NULL, NULL);
      }
    }
    EmitILOp(il, kILOpCall, dst, NULL, NULL, ToASTNode(bin_op->left));
    return dst;
  }
  Error("Not implemented GenerateILForExprBinOp (op: %s)", bin_op->op->str);
  return NULL;
}

Register *GenerateILForExprFuncCall(ASTList *il, Register *dst, ASTNode *node) {
  ASTExprFuncCall *expr_func_call = ToASTExprFuncCall(node);
  if (expr_func_call->func->type != kASTIdent) {
    Error("Calling non-labeled function is not implemented.");
  }
  if (expr_func_call->args) {
    ASTList *arg_list = ToASTList(expr_func_call->args);
    if (!arg_list) Error("arg_list is not an ASTList");
    Register *param_values[8];
    if (GetSizeOfASTList(arg_list) > 8) Error("Too many params");
    for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
      ASTNode *node = GetASTNodeAt(arg_list, i);
      param_values[i] = AllocRegister();
      GenerateILFor(il, param_values[i], node);
    }
    for (int i = 0; i < GetSizeOfASTList(arg_list); i++) {
      EmitILOp(il, kILOpCallParam, NULL, param_values[i], NULL, NULL);
    }
  }
  EmitILOp(il, kILOpCall, dst, NULL, NULL, ToASTNode(expr_func_call->func));
  return dst;
}

void GenerateILForLocalVar(ASTList *il, Register *dst, ASTNode *node) {
  Register *var_addr = AllocRegister();
  EmitILOp(il, kILOpLoadLocalVarAddr, var_addr, NULL, NULL, node);
  EmitILOp(il, kILOpLoad64, dst, var_addr, NULL, NULL);
}

void GenerateILForExprStmt(ASTList *il, Register *dst, ASTNode *node) {
  const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
  if (!expr_stmt) Error("expr_stmt is NULL");
  if (!expr_stmt->expr) return;  // expr is opt
  GenerateILFor(il, dst, expr_stmt->expr);
}

void GenerateILForJumpStmt(ASTList *il, Register *dst, ASTNode *node) {
  ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
  if (IsEqualToken(jump_stmt->kw->token, "return")) {
    Register *return_value = NULL;
    if (jump_stmt->param) {
      return_value = AllocRegister();
      GenerateILFor(il, return_value, jump_stmt->param);
    }
    EmitILOp(il, kILOpReturn, NULL, return_value, NULL, node);
    return;
  } else if (IsEqualToken(jump_stmt->kw->token, "break")) {
    EmitILOp(il, kILOpJmp, NULL, NULL, NULL, jump_stmt->param);
    return;
  }
  Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
}

void GenerateILForIfStmt(ASTList *il, Register *dst, ASTNode *node) {
  ASTIfStmt *if_stmt = ToASTIfStmt(node);
  ASTLabel *end_label = AllocASTLabel();
  ASTLabel *false_label = if_stmt->false_stmt ? AllocASTLabel() : end_label;
  Register *cond_result = AllocRegister();

  GenerateILFor(il, cond_result, if_stmt->cond_expr);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL, ToASTNode(false_label));
  GenerateILFor(il, AllocRegister(), if_stmt->true_stmt);

  if (if_stmt->false_stmt) {
    EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(end_label));
    EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(false_label));
    GenerateILFor(il, AllocRegister(), if_stmt->false_stmt);
  }
  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(end_label));
}

void GenerateILForCondStmt(ASTList *il, Register *dst, ASTNode *node) {
  ASTCondStmt *cond_stmt = ToASTCondStmt(node);
  ASTLabel *false_label = AllocASTLabel();
  ASTLabel *end_label = AllocASTLabel();
  Register *cond_result = AllocRegister();

  GenerateILFor(il, cond_result, cond_stmt->cond_expr);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL, ToASTNode(false_label));

  GenerateILFor(il, dst, cond_stmt->true_expr);
  EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(end_label));

  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(false_label));
  GenerateILFor(il, dst, cond_stmt->false_expr);

  EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(end_label));
}

void GenerateILForWhileStmt(ASTList *il, Register *dst, ASTNode *node) {
  ASTWhileStmt *stmt = ToASTWhileStmt(node);
  Register *cond_result = AllocRegister();

  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(stmt->begin_label));
  GenerateILFor(il, cond_result, stmt->cond_expr);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL,
           ToASTNode(stmt->end_label));

  GenerateILFor(il, AllocRegister(), stmt->body_stmt);
  EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(stmt->begin_label));
  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(stmt->end_label));
}

void GenerateILForForStmt(ASTList *il, Register *dst, ASTNode *node) {
  ASTForStmt *stmt = ToASTForStmt(node);
  Register *cond_result = AllocRegister();

  GenerateILFor(il, AllocRegister(), stmt->init_expr);

  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(stmt->begin_label));
  GenerateILFor(il, cond_result, stmt->cond_expr);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL,
           ToASTNode(stmt->end_label));

  GenerateILFor(il, AllocRegister(), stmt->body_stmt);
  GenerateILFor(il, AllocRegister(), stmt->updt_expr);
  EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(stmt->begin_label));
  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(stmt->end_label));
}

void GenerateILFor(ASTList *il, Register *dst, ASTNode *node) {
  printf("GenerateIL: AST%s...\n", GetASTNodeTypeName(node));
  if (node->type == kASTList) {
    // translation-unit
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kASTFuncDef) {
        GenerateILForFuncDef(il, NULL, child_node);
      }
    }
  } else if (node->type == kASTJumpStmt) {
    GenerateILForJumpStmt(il, dst, node);
  } else if (node->type == kASTExprUnaryPreOp) {
    GenerateILForExprUnaryPreOp(il, dst, node);
  } else if (node->type == kASTExprUnaryPostOp) {
    GenerateILForExprUnaryPostOp(il, dst, node);
  } else if (node->type == kASTExprBinOp) {
    GenerateILForExprBinOp(il, dst, node);
  } else if (node->type == kASTExprFuncCall) {
    GenerateILForExprFuncCall(il, dst, node);
  } else if (node->type == kASTInteger) {
    EmitILOp(il, kILOpLoadImm, dst, NULL, NULL, node);
  } else if (node->type == kASTString) {
    EmitILOp(il, kILOpLoadImm, dst, NULL, NULL, node);
  } else if (node->type == kASTExprStmt) {
    GenerateILForExprStmt(il, dst, node);
  } else if (node->type == kASTLocalVar) {
    GenerateILForLocalVar(il, dst, node);
  } else if (node->type == kASTCondStmt) {
    GenerateILForCondStmt(il, dst, node);
  } else if (node->type == kASTDecl) {
    // do nothing
  } else if (node->type == kASTWhileStmt) {
    GenerateILForWhileStmt(il, dst, node);
  } else if (node->type == kASTForStmt) {
    GenerateILForForStmt(il, dst, node);
  } else if (node->type == kASTIfStmt) {
    GenerateILForIfStmt(il, dst, node);
  } else if (node->type == kASTCompStmt) {
    GenerateILForCompStmt(il, dst, node);
  } else if (node->type == kASTIdent) {
    ASTIdent *ident = ToASTIdent(node);
    GenerateILForLocalVar(il, dst, ToASTNode(ident->local_var));
  } else {
    PrintASTNode(node, 0);
    putchar('\n');
    Error("IL Generation for AST%s is not implemented.",
          GetASTNodeTypeName(node));
  }
}

#define MAX_IL_NODES 2048
ASTList *GenerateIL(ASTNode *root) {
  ASTList *il = AllocASTList(MAX_IL_NODES);
  GenerateILFor(il, NULL, root);
  return il;
}
