#include "compilium.h"

#define REG_NULL 0

struct CONTEXT {
  const Context *parent;
  ASTDict *dict;
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

ASTILOp *EmitILOp(ASTList *il, ILOpType type, Register *dst, Register *left,
                  Register *right, ASTNode *node) {
  ASTILOp *op = AllocAndInitASTILOp(type, dst, left, right, node);
  PushASTNodeToList(il, ToASTNode(op));
  return op;
}

// generators
ASTILOp *GenerateILFor(ASTList *il, ASTNode *node, Context *context);

void GenerateILForCompStmt(ASTList *il, ASTNode *node, Context *context) {
  ASTCompStmt *comp = ToASTCompStmt(node);
  ASTList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
    GenerateILFor(il, GetASTNodeAt(stmt_list, i), context);
  }
}

void GenerateILForFuncDef(ASTList *il, ASTNode *node, Context *context) {
  EmitILOp(il, kILOpFuncBegin, REG_NULL, REG_NULL, REG_NULL, node);
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
      ASTILOp *il_op =
          EmitILOp(il, kILOpLoadArg, AllocRegister(), NULL, NULL, NULL);
      EmitILOp(il, kILOpWriteLocalVar, il_op->dst, NULL, il_op->dst,
               ToASTNode(local_var));
    }
  }
  GenerateILForCompStmt(il, ToASTNode(def->comp_stmt), context);
  EmitILOp(il, kILOpFuncEnd, REG_NULL, REG_NULL, REG_NULL, node);
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

ASTILOp *GenerateILForExprBinOp(ASTList *il, ASTNode *node, Context *context) {
  ASTExprBinOp *bin_op = ToASTExprBinOp(node);
  ILOpType il_op_type = kILOpNop;
  for (int i = 0; bin_op_list[i].str; i++) {
    if (IsEqualToken(bin_op->op, bin_op_list[i].str)) {
      il_op_type = bin_op_list[i].il_op_type;
      break;
    }
  }
  if (il_op_type != kILOpNop) {
    ASTILOp *left = GenerateILFor(il, bin_op->left, context);
    ASTILOp *right = GenerateILFor(il, bin_op->right, context);
    return EmitILOp(il, il_op_type, AllocRegister(), left->dst, right->dst,
                    node);
  } else if (IsEqualToken(bin_op->op, "&&")) {
    Register *dst = AllocRegister();
    ASTLabel *label = AllocASTLabel();
    ASTILOp *il_left = GenerateILFor(il, bin_op->left, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, il_left->dst, NULL, NULL);
    EmitILOp(il, kILOpJmpIfZero, NULL, il_left->dst, NULL, ToASTNode(label));
    ASTILOp *il_right = GenerateILFor(il, bin_op->right, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, il_right->dst, REG_NULL, NULL);
    return EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(label));
  } else if (IsEqualToken(bin_op->op, "||")) {
    Register *dst = AllocRegister();
    ASTLabel *label = AllocASTLabel();
    ASTILOp *il_left = GenerateILFor(il, bin_op->left, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, il_left->dst, NULL, NULL);
    EmitILOp(il, kILOpJmpIfNotZero, NULL, il_left->dst, NULL, ToASTNode(label));
    ASTILOp *il_right = GenerateILFor(il, bin_op->right, context);
    EmitILOp(il, kILOpSetLogicalValue, dst, il_right->dst, REG_NULL, NULL);
    return EmitILOp(il, kILOpLabel, dst, NULL, NULL, ToASTNode(label));
  } else if (IsEqualToken(bin_op->op, "=")) {
    ASTIdent *left_ident = ToASTIdent(bin_op->left);
    if (left_ident) {
      ASTNode *var = FindIdentInContext(context, left_ident);
      ASTLocalVar *local_var = ToASTLocalVar(var);
      if (local_var) {
        ASTILOp *il_op = GenerateILFor(il, bin_op->right, context);
        return EmitILOp(il, kILOpWriteLocalVar, il_op->dst, NULL, il_op->dst,
                        ToASTNode(local_var));
      }
      Error("local variable %s not defined here.", left_ident->token->str);
    }
    Error("Left operand of assignment should be an lvalue");
  } else if (IsEqualToken(bin_op->op, ",")) {
    GenerateILFor(il, bin_op->left, context);
    return GenerateILFor(il, bin_op->right, context);
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
                          ToASTNode(GenerateILFor(il, node, context)));
      }
    }
    return EmitILOp(il, kILOpCall, AllocRegister(), NULL, NULL,
                    ToASTNode(call_params));
  }
  Error("Not implemented GenerateILForExprBinOp (op: %s)", bin_op->op->str);
  return NULL;
}

ASTILOp *GenerateILForConstant(ASTList *il, ASTNode *node, Context *context) {
  return EmitILOp(il, kILOpLoadImm, AllocRegister(), NULL, NULL, node);
}

ASTILOp *GenerateILForIdent(ASTList *il, ASTNode *node, Context *context) {
  ASTNode *var = FindIdentInContext(context, ToASTIdent(node));
  if (!var || (var->type != kASTLocalVar)) {
    Error("Unknown identifier %s", ToASTIdent(node)->token->str);
  }
  ASTLocalVar *local_var = ToASTLocalVar(var);
  if (local_var) {
    return EmitILOp(il, kILOpReadLocalVar, AllocRegister(), NULL, NULL,
                    ToASTNode(local_var));
  }
  Error("var is not a local_var");
  return NULL;
}

ASTILOp *GenerateILForExprStmt(ASTList *il, ASTNode *node, Context *context) {
  const ASTExprStmt *expr_stmt = ToASTExprStmt(node);
  return GenerateILFor(il, expr_stmt->expr, context);
}

ASTILOp *GenerateILForJumpStmt(ASTList *il, ASTNode *node, Context *context) {
  ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
  if (IsEqualToken(jump_stmt->kw->token, "return")) {
    ASTILOp *expr = GenerateILForExprStmt(il, jump_stmt->param, context);
    return EmitILOp(il, kILOpReturn, NULL, expr->dst, REG_NULL, node);
  }
  Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
  return NULL;
}

void GenerateILForDecl(ASTList *il, ASTNode *node, Context *context) {
  ASTDecl *decl = ToASTDecl(node);
  if (!decl) Error("node is not a Decl");
  PrintASTNode(ToASTNode(decl->decl_specs), 0);
  putchar('\n');
  for (int i = 0; i < GetSizeOfASTList(decl->init_decltors); i++) {
    ASTDecltor *decltor = ToASTDecltor(GetASTNodeAt(decl->init_decltors, i));
    AppendLocalVarInContext(context, GetIdentTokenFromDecltor(decltor));
  }
}

void GenerateILForIfStmt(ASTList *il, ASTNode *node, Context *context) {
  ASTIfStmt *if_stmt = ToASTIfStmt(node);
  ASTLabel *label = AllocASTLabel();

  ASTILOp *cond_expr_il = GenerateILFor(il, if_stmt->cond_expr, context);
  EmitILOp(il, kILOpJmpIfZero, NULL, cond_expr_il->dst, NULL, ToASTNode(label));
  GenerateILFor(il, if_stmt->body_stmt, context);
  EmitILOp(il, kILOpLabel, REG_NULL, REG_NULL, REG_NULL, ToASTNode(label));
}

ASTILOp *GenerateILFor(ASTList *il, ASTNode *node, Context *context) {
  printf("GenerateIL: AST%s...\n", GetASTTypeName(node));
  if (node->type == kASTList) {
    // translation-unit
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kASTFuncDef) {
        GenerateILForFuncDef(il, child_node, context);
      }
    }
    return NULL;
  } else if (node->type == kASTJumpStmt) {
    return GenerateILForJumpStmt(il, node, context);
  } else if (node->type == kASTExprBinOp) {
    return GenerateILForExprBinOp(il, node, context);
  } else if (node->type == kASTConstant) {
    return GenerateILForConstant(il, node, context);
  } else if (node->type == kASTExprStmt) {
    return GenerateILForExprStmt(il, node, context);
  } else if (node->type == kASTIdent) {
    return GenerateILForIdent(il, node, context);
  } else if (node->type == kASTDecl) {
    GenerateILForDecl(il, node, context);
    return NULL;
  } else if (node->type == kASTIfStmt) {
    GenerateILForIfStmt(il, node, context);
    return NULL;
  }
  PrintASTNode(node, 0);
  putchar('\n');
  Error("IL Generation for AST%s is not implemented.", GetASTTypeName(node));
  return NULL;
}

#define MAX_IL_NODES 2048
ASTList *GenerateIL(ASTNode *root) {
  ASTList *il = AllocASTList(MAX_IL_NODES);
  Context *context = AllocContext(NULL);
  GenerateILFor(il, root, context);
  return il;
}
