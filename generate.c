#include "compilium.h"

int GenerateIL(ASTNodeList *il, const ASTNode *node);

// https://wiki.osdev.org/System_V_ABI
// registers should be preserved: rbx, rsp, rbp, r12, r13, r14, and r15
// scratch registers: rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11
// return value: rax

#define NUM_OF_SCRATCH_REGS 9
const char *ScratchRegNames[] = {"rax", "rdi", "rsi", "rdx", "rcx",
                                 "r8",  "r9",  "r10", "r11"};

int GetLabelNumber() {
  static int num = 0;
  return num++;
}

#define REG_NULL 0

int GetRegNumber() {
  static int num = 0;
  return ++num;
}

void GenerateILForCompStmt(ASTNodeList *il, const ASTNode *node) {
  const ASTDataCompStmt *comp = GetDataAsCompStmt(node);
  const ASTNodeList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTNodeList(stmt_list); i++) {
    GenerateIL(il, GetASTNodeAt(stmt_list, i));
  }
}

void GenerateILForFuncDef(ASTNodeList *il, const ASTNode *node) {
  const ASTDataFuncDef *def = GetDataAsFuncDef(node);
  PushASTNodeToList(il, AllocateASTNodeAsILOp(kILOpFuncBegin, REG_NULL,
                                              REG_NULL, REG_NULL, node));
  GenerateILForCompStmt(il, def->comp_stmt);
  PushASTNodeToList(il, AllocateASTNodeAsILOp(kILOpFuncEnd, REG_NULL, REG_NULL,
                                              REG_NULL, node));
}

int GenerateILForExprBinOp(ASTNodeList *il, const ASTNode *node) {
  const ASTDataExprBinOp *bin_op = GetDataAsExprBinOp(node);
  int il_left = GenerateIL(il, bin_op->left);
  int il_right = GenerateIL(il, bin_op->right);
  int dst = GetRegNumber();

  ASTNode *il_op;
  switch (bin_op->op_type) {
    case kOpAdd:
      il_op = AllocateASTNodeAsILOp(kILOpAdd, dst, il_left, il_right, node);
      PushASTNodeToList(il, il_op);
      break;
    case kOpSub:
      il_op = AllocateASTNodeAsILOp(kILOpSub, dst, il_left, il_right, node);
      PushASTNodeToList(il, il_op);
      break;
    case kOpMul:
      il_op = AllocateASTNodeAsILOp(kILOpMul, dst, il_left, il_right, node);
      PushASTNodeToList(il, il_op);
      break;
    default:
      Error("Not implemented GenerateILForExprBinOp (op_type: %d)",
            bin_op->op_type);
  }
  return dst;
}

int GenerateILForExprVal(ASTNodeList *il, const ASTNode *node) {
  int dst = GetRegNumber();
  ASTNode *il_op =
      AllocateASTNodeAsILOp(kILOpLoadImm, dst, REG_NULL, REG_NULL, node);
  PushASTNodeToList(il, il_op);
  return dst;
}

int GenerateILForExprStmt(ASTNodeList *il, const ASTNode *node) {
  // https://wiki.osdev.org/System_V_ABI
  const ASTDataExprStmt *expr_stmt = GetDataAsExprStmt(node);
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

int GenerateILForReturnStmt(ASTNodeList *il, const ASTNode *node) {
  const ASTDataReturnStmt *ret = GetDataAsReturnStmt(node);
  int expr_reg = GenerateILForExprStmt(il, ret->expr_stmt);

  ASTNode *il_op =
      AllocateASTNodeAsILOp(kILOpReturn, REG_NULL, expr_reg, REG_NULL, node);
  PushASTNodeToList(il, il_op);

  return REG_NULL;
}

int GenerateIL(ASTNodeList *il, const ASTNode *node) {
  if (node->type == kRoot) {
    ASTNodeList *list = GetDataAsRoot(node)->root_list;
    for (int i = 0; i < GetSizeOfASTNodeList(list); i++) {
      const ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kFuncDef) {
        GenerateILForFuncDef(il, child_node);
      }
    }
    return -1;
  } else if (node->type == kReturnStmt) {
    return GenerateILForReturnStmt(il, node);
  } else if (node->type == kExprBinOp) {
    return GenerateILForExprBinOp(il, node);
  } else if (node->type == kExprVal) {
    return GenerateILForExprVal(il, node);
  } else if (node->type == kExprStmt) {
    return GenerateILForExprStmt(il, node);
  } else {
    Error("Generation for AST Type %d is not implemented.", node->type);
  }
  return -1;
}

void GenerateCode(FILE *fp, ASTNodeList *il) {
  fputs(".intel_syntax noprefix\n", fp);
  // generate func symbol
  for (int i = 0; i < GetSizeOfASTNodeList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    const ASTDataILOp *op;
    if ((op = GetDataAsILOpOfType(node, kILOpFuncBegin))) {
      const ASTDataFuncDef *def = GetDataAsFuncDef(op->ast_node);
      const ASTDataFuncDecl *decl = GetDataAsFuncDecl(def->func_decl);
      const ASTDataVarDef *defv = GetDataAsVarDef(decl->type_and_name);
      fprintf(fp, ".global _%s\n", defv->name->str);
      printf("found func: %s\n", defv->name->str);
    }
  }
  // generate code
  for (int i = 0; i < GetSizeOfASTNodeList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    const ASTDataILOp *op;
    if ((op = GetDataAsILOpOfType(node, kILOpFuncBegin))) {
      const ASTDataFuncDef *def = GetDataAsFuncDef(op->ast_node);
      const ASTDataFuncDecl *decl = GetDataAsFuncDecl(def->func_decl);
      const ASTDataVarDef *defv = GetDataAsVarDef(decl->type_and_name);
      fprintf(fp, "_%s:\n", defv->name->str);
      fprintf(fp, "push    rbp\n");
      fprintf(fp, "mov     rbp, rsp\n");
    } else if ((op = GetDataAsILOpOfType(node, kILOpFuncEnd))) {
      fprintf(fp, "mov     dword ptr [rbp - 4], 0\n");
      fprintf(fp, "pop     rbp\n");
      fprintf(fp, "ret\n");
    } else if ((op = GetDataAsILOpOfType(node, kILOpLoadImm))) {
      const char *dst_name = ScratchRegNames[op->dst_reg];
      //
      char *p;
      const ASTDataExprVal *val = GetDataAsExprVal(op->ast_node);
      const char *s = val->token->str;
      int n = strtol(s, &p, 0);
      if (!(s[0] != 0 && *p == 0)) {
        Error("%s is not valid as integer.", s);
      }
      fprintf(fp, "mov %s, %d\n", dst_name, n);
    } else if ((op = GetDataAsILOpOfType(node, kILOpAdd))) {
      const char *dst = ScratchRegNames[op->dst_reg];
      const char *left = ScratchRegNames[op->left_reg];
      const char *right = ScratchRegNames[op->right_reg];
      //
      fprintf(fp, "add %s, %s\n", left, right);
      fprintf(fp, "mov %s, %s\n", dst, left);
    } else if ((op = GetDataAsILOpOfType(node, kILOpSub))) {
      const char *dst = ScratchRegNames[op->dst_reg];
      const char *left = ScratchRegNames[op->left_reg];
      const char *right = ScratchRegNames[op->right_reg];
      //
      fprintf(fp, "sub %s, %s\n", left, right);
      fprintf(fp, "mov %s, %s\n", dst, left);
    } else if ((op = GetDataAsILOpOfType(node, kILOpMul))) {
      const char *dst = ScratchRegNames[op->dst_reg];
      const char *left = ScratchRegNames[op->left_reg];
      const char *right = ScratchRegNames[op->right_reg];
      //
      // eDX: eAX <- eAX * r/m
      fprintf(fp, "mov rax, %s\n", left);
      fprintf(fp, "push rdx\n");
      fprintf(fp, "imul %s\n", right);
      fprintf(fp, "pop rdx\n");
      fprintf(fp, "mov %s, rax\n", dst);
    } else if ((op = GetDataAsILOpOfType(node, kILOpReturn))) {
      const char *left = ScratchRegNames[op->left_reg];
      //
      fprintf(fp, "mov rax, %s\n", left);
    } else {
      PrintASTNode(node, 0);
      putchar('\n');
      Error("Not implemented code generation for the node shown above");
    }
  }
}

#define MAX_IL_NODES 2048
void Generate(FILE *fp, const ASTNode *root) {
  ASTNodeList *intermediate_code = AllocateASTNodeList(MAX_IL_NODES);

  GenerateIL(intermediate_code, root);
  PrintASTNodeList(intermediate_code, 0);
  putchar('\n');

  GenerateCode(fp, intermediate_code);
}
