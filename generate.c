#include "compilium.h"

int GenerateIL(ASTList *il, ASTNode *node);

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

void GenerateILForCompStmt(ASTList *il, ASTNode *node) {
  ASTCompStmt *comp = ToASTCompStmt(node);
  ASTList *stmt_list = comp->stmt_list;
  for (int i = 0; i < GetSizeOfASTList(stmt_list); i++) {
    GenerateIL(il, GetASTNodeAt(stmt_list, i));
  }
}

void GenerateILForFuncDef(ASTList *il, ASTNode *node) {
  ASTFuncDef *def = ToASTFuncDef(node);
  PushASTNodeToList(il, AllocateASTNodeAsILOp(kILOpFuncBegin, REG_NULL,
                                              REG_NULL, REG_NULL, node));
  GenerateILForCompStmt(il, ToASTNode(def->comp_stmt));
  PushASTNodeToList(il, AllocateASTNodeAsILOp(kILOpFuncEnd, REG_NULL, REG_NULL,
                                              REG_NULL, node));
}

int GenerateILForExprBinOp(ASTList *il, ASTNode *node) {
  ASTExprBinOp *bin_op = ToASTExprBinOp(node);
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

int GenerateILForExprVal(ASTList *il, ASTNode *node) {
  int dst = GetRegNumber();
  ASTNode *il_op =
      AllocateASTNodeAsILOp(kILOpLoadImm, dst, REG_NULL, REG_NULL, node);
  PushASTNodeToList(il, il_op);
  return dst;
}

int GenerateILForExprStmt(ASTList *il, ASTNode *node) {
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

int GenerateILForJumpStmt(ASTList *il, ASTNode *node) {
  ASTJumpStmt *jump_stmt = ToASTJumpStmt(node);
  if(IsEqualToken(jump_stmt->kw->token, "return")){
    int expr_reg = GenerateILForExprStmt(il, jump_stmt->param);

    ASTNode *il_op =
       AllocateASTNodeAsILOp(kILOpReturn, REG_NULL, expr_reg, REG_NULL, node);
    PushASTNodeToList(il, il_op);
  } else{
    Error("Not implemented JumpStmt (%s)", jump_stmt->kw->token->str);
  }

  return REG_NULL;
}

int GenerateIL(ASTList *il, ASTNode *node) {
  if (node->type == kList) {
    // translation-unit
    ASTList *list = ToASTList(node);
    for (int i = 0; i < GetSizeOfASTList(list); i++) {
      ASTNode *child_node = GetASTNodeAt(list, i);
      if (child_node->type == kFuncDef) {
        GenerateILForFuncDef(il, child_node);
      }
    }
    return -1;
  } else if (node->type == kJumpStmt) {
    return GenerateILForJumpStmt(il, node);
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

void GenerateCode(FILE *fp, ASTList *il) {
  fputs(".intel_syntax noprefix\n", fp);
  // generate func symbol
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    if (op->op == kILOpFuncBegin) {
      const char *func_name =
          GetFuncNameStrFromFuncDef(ToASTFuncDef(op->ast_node));
      if (!func_name) {
        Error("func_name is null");
      }
      fprintf(fp, ".global _%s\n", func_name);
      printf("found func: %s\n", func_name);
    }
  }
  // generate code
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    if (!op) {
      Error("op is null!");
    }
    switch (op->op) {
      case kILOpFuncBegin: {
        const char *func_name =
            GetFuncNameStrFromFuncDef(ToASTFuncDef(op->ast_node));
        if (!func_name) {
          Error("func_name is null");
        }
        fprintf(fp, "_%s:\n", func_name);
        fprintf(fp, "push    rbp\n");
        fprintf(fp, "mov     rbp, rsp\n");
      } break;
      case kILOpFuncEnd:
        fprintf(fp, "mov     dword ptr [rbp - 4], 0\n");
        fprintf(fp, "pop     rbp\n");
        fprintf(fp, "ret\n");
        break;
      case kILOpLoadImm: {
        const char *dst_name = ScratchRegNames[op->dst_reg];
        //
        char *p;
        ASTExprVal *val = ToASTExprVal(op->ast_node);
        const char *s = val->token->str;
        int n = strtol(s, &p, 0);
        if (!(s[0] != 0 && *p == 0)) {
          Error("%s is not valid as integer.", s);
        }
        fprintf(fp, "mov %s, %d\n", dst_name, n);
      } break;
      case kILOpAdd:
      case kILOpSub:
      case kILOpMul: {
        const char *dst = ScratchRegNames[op->dst_reg];
        const char *left = ScratchRegNames[op->left_reg];
        const char *right = ScratchRegNames[op->right_reg];
        //
        if (op->op == kILOpAdd) {
          fprintf(fp, "add %s, %s\n", left, right);
          fprintf(fp, "mov %s, %s\n", dst, left);
        } else if (op->op == kILOpSub) {
          fprintf(fp, "sub %s, %s\n", left, right);
          fprintf(fp, "mov %s, %s\n", dst, left);
        } else if (op->op == kILOpMul) {
          // eDX: eAX <- eAX * r/m
          fprintf(fp, "mov rax, %s\n", left);
          fprintf(fp, "push rdx\n");
          fprintf(fp, "imul %s\n", right);
          fprintf(fp, "pop rdx\n");
          fprintf(fp, "mov %s, rax\n", dst);
        }
      } break;
      case kILOpReturn: {
        const char *left = ScratchRegNames[op->left_reg];
        //
        fprintf(fp, "mov rax, %s\n", left);
      } break;
      default:
        PrintASTNode(node, 0);
        putchar('\n');
        Error("Not implemented code generation for the node shown above");
    }
  }
}

#define MAX_IL_NODES 2048
void Generate(FILE *fp, ASTNode *root) {
  ASTList *intermediate_code = AllocASTList(MAX_IL_NODES);

  GenerateIL(intermediate_code, root);
  PrintASTNode(ToASTNode(intermediate_code), 0);
  putchar('\n');

  GenerateCode(fp, intermediate_code);
}
