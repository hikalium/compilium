#include "compilium.h"

struct CONTEXT {
  const Context *parent;
  ASTDict *dict;
  ASTLabel *break_label;
};

Context *AllocContext(const Context *parent) {
  printf("AllocContext\n");
  Context *context = malloc(sizeof(Context));
  context->parent = parent;
  context->dict = AllocASTDict(8);
  return context;
}

ASTNode *FindIdentInContext(const Context *context, ASTIdent *ident) {
  if (!context) return NULL;
  ASTNode *result = FindASTNodeInDict(context->dict, ident->token->str);
  if (result) return result;
  return FindIdentInContext(context->parent, ident);
}

ASTLocalVar *AppendLocalVarInContext(Context *context, const Token *token) {
  int ofs = GetSizeOfASTDict(context->dict) + 1;
  printf("LocalVar[%d]: %s\n", ofs, token->str);
  ASTLocalVar *local_var = AllocASTLocalVar(ofs);
  local_var->name = token->str;
  AppendASTNodeToDict(context->dict, token->str, ToASTNode(local_var));
  return local_var;
}

int GetStackSizeForContext(const Context *context) {
  return GetSizeOfASTDict(context->dict) * 8;
}

int next_vreg_id = 1;
Register *AllocRegister() {
  Register *reg = malloc(sizeof(Register));
  reg->vreg_id = next_vreg_id++;
  reg->save_label_num = 0;
  reg->real_reg = 0;
  return reg;
}

const char *ILOpTypeName[kNumOfILOpFunc];

void InitILOpTypeName() {
  ILOpTypeName[kILOpAdd] = "Add";
  ILOpTypeName[kILOpSub] = "Sub";
  ILOpTypeName[kILOpMul] = "Mul";
  ILOpTypeName[kILOpDiv] = "Div";
  ILOpTypeName[kILOpMod] = "Mod";
  ILOpTypeName[kILOpAnd] = "And";
  ILOpTypeName[kILOpXor] = "Xor";
  ILOpTypeName[kILOpOr] = "Or";
  ILOpTypeName[kILOpAnd] = "Not";
  ILOpTypeName[kILOpNegate] = "Negate";
  ILOpTypeName[kILOpLogicalAnd] = "LogicalAnd";
  ILOpTypeName[kILOpLogicalOr] = "LogicalOr";
  ILOpTypeName[kILOpCmpG] = "CmpG";
  ILOpTypeName[kILOpCmpGE] = "CmpGE";
  ILOpTypeName[kILOpCmpL] = "CmpL";
  ILOpTypeName[kILOpCmpLE] = "CmpLE";
  ILOpTypeName[kILOpCmpE] = "CmpE";
  ILOpTypeName[kILOpCmpNE] = "CmpNE";
  ILOpTypeName[kILOpShiftLeft] = "ShiftLeft";
  ILOpTypeName[kILOpShiftRight] = "ShiftRight";
  ILOpTypeName[kILOpIncrement] = "Increment";
  ILOpTypeName[kILOpDecrement] = "Increment";
  ILOpTypeName[kILOpLoadImm] = "LoadImm";
  ILOpTypeName[kILOpLoadIdent] = "LoadIdent";
  ILOpTypeName[kILOpLoadArg] = "LoadArg";
  ILOpTypeName[kILOpFuncBegin] = "FuncBegin";
  ILOpTypeName[kILOpFuncEnd] = "FuncEnd";
  ILOpTypeName[kILOpReturn] = "Return";
  ILOpTypeName[kILOpCall] = "Call";
  ILOpTypeName[kILOpWriteLocalVar] = "WriteLocalVar";
  ILOpTypeName[kILOpReadLocalVar] = "ReadLocalVar";
  ILOpTypeName[kILOpLabel] = "Label";
  ILOpTypeName[kILOpJmp] = "Jmp";
  ILOpTypeName[kILOpJmpIfZero] = "JmpIfZero";
  ILOpTypeName[kILOpJmpIfNotZero] = "JmpIfZero";
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
void GenerateILFor(ASTList *il, Register *dst, ASTNode *node, Context *context);
void GenerateILForIdent(ASTList *il, Register *dst, ASTNode *node,
                        Context *context);

void GenerateILForCompStmt(ASTList *il, Register *dst, ASTNode *node,
                           Context *context) {
  ASTCompStmt *comp = ToASTCompStmt(node);
  ASTList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
    GenerateILFor(il, AllocRegister(), GetASTNodeAt(stmt_list, i), context);
  }
}

void GenerateILForFuncDef(ASTList *il, Register *dst, ASTNode *node,
                          Context *context) {
  EmitILOp(il, kILOpFuncBegin, NULL, NULL, NULL, node);
  ASTFuncDef *def = ToASTFuncDef(node);
  context = AllocContext(context);
  def->context = context;
  ASTDirectDecltor *args_decltor = def->decltor->direct_decltor;
  ASTList *param_decl_list = ToASTList(args_decltor->data);

  if (param_decl_list) {
    PrintASTNode(ToASTNode(param_decl_list), 0);
    for (int i = 0; i < GetSizeOfASTList(param_decl_list); i++) {
      ASTParamDecl *param_decl =
          ToASTParamDecl(GetASTNodeAt(param_decl_list, i));
      ASTDecltor *param_decltor = ToASTDecltor(param_decl->decltor);
      const Token *ident_token = GetIdentTokenFromDecltor(param_decltor);
      printf("param: %s\n", ident_token->str);
      ASTLocalVar *local_var = AppendLocalVarInContext(context, ident_token);
      Register *var_reg = AllocRegister();
      EmitILOp(il, kILOpLoadArg, var_reg, NULL, NULL, NULL);
      EmitILOp(il, kILOpWriteLocalVar, var_reg, NULL, var_reg,
               ToASTNode(local_var));
    }
  }
  GenerateILForCompStmt(il, AllocRegister(), ToASTNode(def->comp_stmt),
                        context);
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
                                    Register *rvalue, Context *context) {
  ASTIdent *left_ident = ToASTIdent(left);
  if (left_ident) {
    ASTNode *var = FindIdentInContext(context, left_ident);
    ASTLocalVar *local_var = ToASTLocalVar(var);
    if (local_var) {
      EmitILOp(il, kILOpWriteLocalVar, rvalue, NULL, rvalue,
               ToASTNode(local_var));
      return rvalue;
    }
    Error("local variable %s not defined here.", left_ident->token->str);
  }
  Error("Left operand of assignment should be an lvalue");
  return NULL;
}

Register *GenerateILForExprUnaryPreOp(ASTList *il, Register *dst, ASTNode *node,
                                      Context *context) {
  ASTExprUnaryPreOp *op = ToASTExprUnaryPreOp(node);
  Register *rvalue = AllocRegister();
  if (IsEqualToken(op->op, "+")) {
    GenerateILFor(il, dst, op->expr, context);
    return dst;
  } else if (IsEqualToken(op->op, "-")) {
    GenerateILFor(il, rvalue, op->expr, context);
    EmitILOp(il, kILOpNegate, dst, rvalue, NULL, node);
    return dst;
  } else if (IsEqualToken(op->op, "~")) {
    GenerateILFor(il, rvalue, op->expr, context);
    EmitILOp(il, kILOpNot, dst, rvalue, NULL, node);
    return dst;
  } else if (IsEqualToken(op->op, "!")) {
    GenerateILFor(il, rvalue, op->expr, context);
    EmitILOp(il, kILOpLogicalNot, dst, rvalue, NULL, node);
    return dst;
  } else if (IsEqualToken(op->op, "++")) {
    GenerateILForIdent(il, rvalue, op->expr, context);
    EmitILOp(il, kILOpIncrement, dst, rvalue, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, dst, context);
    return dst;
  } else if (IsEqualToken(op->op, "--")) {
    GenerateILForIdent(il, rvalue, op->expr, context);
    EmitILOp(il, kILOpDecrement, dst, rvalue, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, dst, context);
    return dst;
  }
  Error("Not impl");
  return NULL;
}

Register *GenerateILForExprUnaryPostOp(ASTList *il, Register *dst,
                                       ASTNode *node, Context *context) {
  ASTExprUnaryPostOp *op = ToASTExprUnaryPostOp(node);
  if (IsEqualToken(op->op, "++")) {
    Register *inc_result = AllocRegister();

    GenerateILForIdent(il, dst, op->expr, context);
    EmitILOp(il, kILOpIncrement, inc_result, dst, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, inc_result, context);
    return dst;
  } else if (IsEqualToken(op->op, "--")) {
    Register *dec_result = AllocRegister();

    GenerateILForIdent(il, dst, op->expr, context);
    EmitILOp(il, kILOpDecrement, dec_result, dst, NULL, NULL);
    GenerateILForAssignmentOp(il, op->expr, dec_result, context);
    return dst;
  }
  Error("Not impl");
  return NULL;
}

Register *GenerateILForExprBinOp(ASTList *il, Register *dst, ASTNode *node,
                                 Context *context) {
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
    GenerateILFor(il, left, bin_op->left, context);
    GenerateILFor(il, right, bin_op->right, context);
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
    GenerateILFor(il, left, bin_op->left, context);
    GenerateILFor(il, right, bin_op->right, context);
    EmitILOp(il, il_op_type, dst, left, right, node);
    GenerateILForAssignmentOp(il, bin_op->left, dst, context);
    return dst;
  }

  if (IsEqualToken(bin_op->op, "&&")) {
    ASTLabel *label = AllocASTLabel();

    GenerateILFor(il, left, bin_op->left, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, left, NULL, NULL);
    EmitILOp(il, kILOpJmpIfZero, NULL, left, NULL, ToASTNode(label));
    GenerateILFor(il, right, bin_op->right, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, right, NULL, NULL);
    EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(label));
    return dst;
  } else if (IsEqualToken(bin_op->op, "||")) {
    ASTLabel *label = AllocASTLabel();

    GenerateILFor(il, left, bin_op->left, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, left, NULL, NULL);
    EmitILOp(il, kILOpJmpIfNotZero, NULL, left, NULL, ToASTNode(label));
    GenerateILFor(il, right, bin_op->right, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, right, NULL, NULL);
    EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(label));
    return dst;
  } else if (IsEqualToken(bin_op->op, "=")) {
    GenerateILFor(il, dst, bin_op->right, context);
    GenerateILForAssignmentOp(il, bin_op->left, dst, context);
    return dst;
  } else if (IsEqualToken(bin_op->op, ",")) {
    GenerateILFor(il, AllocRegister(), bin_op->left, context);
    GenerateILFor(il, dst, bin_op->right, context);
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
        GenerateILFor(il, param_values[i], node, context);
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

Register *GenerateILForConstant(ASTList *il, Register *dst, ASTNode *node,
                                Context *context) {
  EmitILOp(il, kILOpLoadImm, dst, NULL, NULL, node);
  return dst;
}

void GenerateILForIdent(ASTList *il, Register *dst, ASTNode *node,
                        Context *context) {
  ASTNode *var = FindIdentInContext(context, ToASTIdent(node));
  if (!var || (var->type != kASTLocalVar)) {
    Error("Unknown identifier %s", ToASTIdent(node)->token->str);
  }
  ASTLocalVar *local_var = ToASTLocalVar(var);
  if (local_var) {
    EmitILOp(il, kILOpReadLocalVar, dst, NULL, NULL, ToASTNode(local_var));
    return;
  }
  Error("var is not a local_var");
}

void GenerateILForExprStmt(ASTList *il, Register *dst, ASTNode *node,
                           Context *context) {
  const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
  if (!expr_stmt) Error("expr_stmt is NULL");
  if (!expr_stmt->expr) return;  // expr is opt
  GenerateILFor(il, dst, expr_stmt->expr, context);
}

void GenerateILForJumpStmt(ASTList *il, Register *dst, ASTNode *node,
                           Context *context) {
  ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
  if (IsEqualToken(jump_stmt->kw->token, "return")) {
    Register *return_value = NULL;
    if (jump_stmt->param) {
      return_value = AllocRegister();
      GenerateILFor(il, return_value, jump_stmt->param, context);
    }
    EmitILOp(il, kILOpReturn, NULL, return_value, NULL, node);
    return;
  } else if (IsEqualToken(jump_stmt->kw->token, "break")) {
    if (!context->break_label) {
      Error("break-stmt should be used within iteration-stmt");
    }
    EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(context->break_label));
    return;
  }
  Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
}

void GenerateILForDecl(ASTList *il, Register *dst, ASTNode *node,
                       Context *context) {
  ASTDecl *decl = ToASTDecl(node);
  if (!decl) Error("node is not a Decl");
  PrintASTNode(ToASTNode(decl->decl_specs), 0);
  putchar('\n');
  for (int i = 0; i < GetSizeOfASTList(decl->init_decltors); i++) {
    ASTDecltor *decltor = ToASTDecltor(GetASTNodeAt(decl->init_decltors, i));
    AppendLocalVarInContext(context, GetIdentTokenFromDecltor(decltor));
  }
}

void GenerateILForIfStmt(ASTList *il, Register *dst, ASTNode *node,
                         Context *context) {
  ASTIfStmt *if_stmt = ToASTIfStmt(node);
  ASTLabel *end_label = AllocASTLabel();
  ASTLabel *false_label = if_stmt->false_stmt ? AllocASTLabel() : end_label;
  Register *cond_result = AllocRegister();

  GenerateILFor(il, cond_result, if_stmt->cond_expr, context);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL, ToASTNode(false_label));
  GenerateILFor(il, AllocRegister(), if_stmt->true_stmt, context);

  if (if_stmt->false_stmt) {
    EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(end_label));
    EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(false_label));
    GenerateILFor(il, AllocRegister(), if_stmt->false_stmt, context);
  }
  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(end_label));
}

void GenerateILForCondStmt(ASTList *il, Register *dst, ASTNode *node,
                           Context *context) {
  ASTCondStmt *cond_stmt = ToASTCondStmt(node);
  ASTLabel *false_label = AllocASTLabel();
  ASTLabel *end_label = AllocASTLabel();
  Register *cond_result = AllocRegister();

  GenerateILFor(il, cond_result, cond_stmt->cond_expr, context);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL, ToASTNode(false_label));

  GenerateILFor(il, dst, cond_stmt->true_expr, context);
  EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(end_label));

  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(false_label));
  GenerateILFor(il, dst, cond_stmt->false_expr, context);

  EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(end_label));
}

void GenerateILForWhileStmt(ASTList *il, Register *dst, ASTNode *node,
                            Context *context) {
  ASTWhileStmt *stmt = ToASTWhileStmt(node);
  ASTLabel *begin_label = AllocASTLabel();
  ASTLabel *end_label = AllocASTLabel();
  Register *cond_result = AllocRegister();

  ASTLabel *org_break_label = context->break_label;
  context->break_label = end_label;

  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(begin_label));
  GenerateILFor(il, cond_result, stmt->cond_expr, context);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL, ToASTNode(end_label));

  GenerateILFor(il, AllocRegister(), stmt->body_stmt, context);
  EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(begin_label));
  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(end_label));

  context->break_label = org_break_label;
}

void GenerateILForForStmt(ASTList *il, Register *dst, ASTNode *node,
                          Context *context) {
  ASTForStmt *stmt = ToASTForStmt(node);
  ASTLabel *begin_label = AllocASTLabel();
  ASTLabel *end_label = AllocASTLabel();
  Register *cond_result = AllocRegister();

  ASTLabel *org_break_label = context->break_label;
  context->break_label = end_label;

  GenerateILFor(il, AllocRegister(), stmt->init_expr, context);

  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(begin_label));
  GenerateILFor(il, cond_result, stmt->cond_expr, context);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_result, NULL, ToASTNode(end_label));

  GenerateILFor(il, AllocRegister(), stmt->body_stmt, context);
  GenerateILFor(il, AllocRegister(), stmt->updt_expr, context);
  EmitILOp(il, kILOpJmp, NULL, NULL, NULL, ToASTNode(begin_label));
  EmitILOp(il, kILOpLabel, NULL, NULL, NULL, ToASTNode(end_label));

  context->break_label = org_break_label;
}

void GenerateILFor(ASTList *il, Register *dst, ASTNode *node,
                   Context *context) {
  printf("GenerateIL: AST%s...\n", GetASTTypeName(node));
  if (node->type == kASTList) {
    // translation-unit
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kASTFuncDef) {
        GenerateILForFuncDef(il, NULL, child_node, context);
      }
    }
  } else if (node->type == kASTJumpStmt) {
    GenerateILForJumpStmt(il, dst, node, context);
  } else if (node->type == kASTExprUnaryPreOp) {
    GenerateILForExprUnaryPreOp(il, dst, node, context);
  } else if (node->type == kASTExprUnaryPostOp) {
    GenerateILForExprUnaryPostOp(il, dst, node, context);
  } else if (node->type == kASTExprBinOp) {
    GenerateILForExprBinOp(il, dst, node, context);
  } else if (node->type == kASTConstant) {
    GenerateILForConstant(il, dst, node, context);
  } else if (node->type == kASTExprStmt) {
    GenerateILForExprStmt(il, dst, node, context);
  } else if (node->type == kASTIdent) {
    GenerateILForIdent(il, dst, node, context);
  } else if (node->type == kASTCondStmt) {
    GenerateILForCondStmt(il, dst, node, context);
  } else if (node->type == kASTDecl) {
    GenerateILForDecl(il, dst, node, context);
  } else if (node->type == kASTWhileStmt) {
    GenerateILForWhileStmt(il, dst, node, context);
  } else if (node->type == kASTForStmt) {
    GenerateILForForStmt(il, dst, node, context);
  } else if (node->type == kASTIfStmt) {
    GenerateILForIfStmt(il, dst, node, context);
  } else if (node->type == kASTCompStmt) {
    GenerateILForCompStmt(il, dst, node, context);
  } else {
    PrintASTNode(node, 0);
    putchar('\n');
    Error("IL Generation for AST%s is not implemented.", GetASTTypeName(node));
  }
}

#define MAX_IL_NODES 2048
ASTList *GenerateIL(ASTNode *root) {
  ASTList *il = AllocASTList(MAX_IL_NODES);
  Context *context = AllocContext(NULL);
  GenerateILFor(il, NULL, root, context);
  return il;
}
