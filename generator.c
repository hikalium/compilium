#include "compilium.h"

static void GenerateForNodeRValue(struct Node *node);

static struct Node *str_list;

static int GetLabelNumber() {
  static int label_number;
  return ++label_number;
}

static void EmitConvertToBool(int dst, int src) {
  // This code also sets zero flag as boolean value
  printf("cmp %s, 0\n", reg_names_64[src]);
  printf("setnz %s\n", reg_names_8[src]);
  printf("movzx %s, %s\n", reg_names_64[dst], reg_names_8[src]);
}

static void EmitCompareIntegers(int dst, int left, int right, const char *cc) {
  printf("cmp %s, %s\n", reg_names_64[left], reg_names_64[right]);
  printf("set%s %s\n", cc, reg_names_8[dst]);
  printf("movzx %s, %s\n", reg_names_64[dst], reg_names_8[dst]);
}

static void GenerateForNode(struct Node *node) {
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      GenerateForNode(GetNodeAt(node, i));
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    GenerateForNodeRValue(node->func_expr);
    printf("push %s\n", reg_names_64[node->func_expr->reg]);
    int i;
    assert(GetSizeOfList(node->arg_expr_list) <= NUM_OF_PARAM_REGISTERS);
    for (i = 0; i < GetSizeOfList(node->arg_expr_list); i++) {
      struct Node *n = GetNodeAt(node->arg_expr_list, i);
      GenerateForNodeRValue(n);
      printf("push %s\n", reg_names_64[n->reg]);
    }
    for (i--; i >= 0; i--) {
      printf("pop %s\n", param_reg_names_64[i]);
    }
    printf("pop rax\n");
    printf("call rax\n");
    printf("mov %s, rax\n", reg_names_64[node->reg]);
    return;
  } else if (node->type == kASTFuncDef) {
    const char *func_name = CreateTokenStr(node->func_name_token);
    printf(".global %s%s\n", symbol_prefix, func_name);
    printf("%s%s:\n", symbol_prefix, func_name);
    printf("push rbp\n");
    printf("mov rbp, rsp\n");
    GenerateForNode(node->func_body);
    printf("mov rsp, rbp\n");
    printf("pop rbp\n");
    printf("ret\n");
    return;
  }
  assert(node && node->op);
  if (node->type == kASTExpr) {
    if (node->op->type == kTokenDecimalNumber ||
        node->op->type == kTokenOctalNumber) {
      printf("mov %s, %ld\n", reg_names_64[node->reg],
             strtol(node->op->begin, NULL, 0));
      return;
    } else if (node->op->type == kTokenCharLiteral) {
      if (node->op->length == (1 + 1 + 1)) {
        printf("mov %s, %d\n", reg_names_64[node->reg], node->op->begin[1]);
        return;
      }
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      GenerateForNode(node->right);
      return;
    } else if (node->op->type == kTokenIdent) {
      if (node->expr_type->type == kTypeFunction) {
        const char *label_name = CreateTokenStr(node->op);
        printf(".global %s%s\n", symbol_prefix, label_name);
        printf("mov %s, [rip + %s%s@GOTPCREL]\n", reg_names_64[node->reg],
               symbol_prefix, label_name);
        return;
      }
      printf("lea %s, [rbp - %d]\n", reg_names_64[node->reg],
             node->byte_offset);
      return;
    } else if (node->op->type == kTokenStringLiteral) {
      int str_label = GetLabelNumber();
      printf("lea %s, [rip + L%d]\n", reg_names_64[node->reg], str_label);
      node->label_number = str_label;
      PushToList(str_list, node);
      return;
    } else if (node->cond) {
      GenerateForNodeRValue(node->cond);
      int false_label = GetLabelNumber();
      int end_label = GetLabelNumber();
      EmitConvertToBool(node->cond->reg, node->cond->reg);
      printf("jz L%d\n", false_label);
      GenerateForNodeRValue(node->left);
      printf("mov %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->left->reg]);
      printf("jmp L%d\n", end_label);
      printf("L%d:\n", false_label);
      GenerateForNodeRValue(node->right);
      printf("mov %s, %s\n", reg_names_64[node->reg],
             reg_names_64[node->right->reg]);
      printf("L%d:\n", end_label);
      return;
    } else if (!node->left && node->right) {
      if (node->op->type == kTokenKwSizeof) {
        printf("mov %s, %d\n", reg_names_64[node->reg],
               GetSizeOfType(node->right->expr_type));
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "&")) {
        GenerateForNode(node->right);
        return;
      }
      GenerateForNodeRValue(node->right);
      if (IsEqualTokenWithCStr(node->op, "+")) {
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "-")) {
        printf("neg %s\n", reg_names_64[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "~")) {
        printf("not %s\n", reg_names_64[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "!")) {
        EmitConvertToBool(node->reg, node->reg);
        printf("setz %s\n", reg_names_8[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "*")) {
        return;
      }
      ErrorWithToken(node->op,
                     "GenerateForNode: Not implemented unary prefix op");
    } else if (node->left && node->right) {
      if (IsEqualTokenWithCStr(node->op, "&&")) {
        GenerateForNodeRValue(node->left);
        int skip_label = GetLabelNumber();
        EmitConvertToBool(node->reg, node->left->reg);
        printf("jz L%d\n", skip_label);
        GenerateForNodeRValue(node->right);
        EmitConvertToBool(node->reg, node->right->reg);
        printf("L%d:\n", skip_label);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "||")) {
        GenerateForNodeRValue(node->left);
        int skip_label = GetLabelNumber();
        EmitConvertToBool(node->reg, node->left->reg);
        printf("jnz L%d\n", skip_label);
        GenerateForNodeRValue(node->right);
        EmitConvertToBool(node->reg, node->right->reg);
        printf("L%d:\n", skip_label);
        return;
      } else if (IsEqualTokenWithCStr(node->op, ",")) {
        GenerateForNode(node->left);
        GenerateForNodeRValue(node->right);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "=")) {
        GenerateForNode(node->left);
        GenerateForNodeRValue(node->right);
        int size = GetSizeOfType(node->right->expr_type);
        if (size == 8) {
          printf("mov [%s], %s\n", reg_names_64[node->left->reg],
                 reg_names_64[node->right->reg]);
          return;
        } else if (size == 4) {
          printf("mov [%s], %s\n", reg_names_64[node->left->reg],
                 reg_names_32[node->right->reg]);
          return;
        } else if (size == 1) {
          printf("mov [%s], %s\n", reg_names_64[node->left->reg],
                 reg_names_8[node->right->reg]);
          return;
        }
        ErrorWithToken(node->op, "Assigning %d bytes is not implemented.",
                       size);
      }
      GenerateForNodeRValue(node->left);
      GenerateForNodeRValue(node->right);
      if (IsEqualTokenWithCStr(node->op, "+")) {
        printf("add %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "-")) {
        printf("sub %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "*")) {
        // rdx:rax <- rax * r/m
        printf("xor rdx, rdx\n");
        printf("mov rax, %s\n", reg_names_64[node->reg]);
        printf("imul %s\n", reg_names_64[node->right->reg]);
        printf("mov %s, rax\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "/")) {
        // rax <- rdx:rax / r/m
        printf("xor rdx, rdx\n");
        printf("mov rax, %s\n", reg_names_64[node->reg]);
        printf("idiv %s\n", reg_names_64[node->right->reg]);
        printf("mov %s, rax\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "%")) {
        // rdx <- rdx:rax / r/m
        printf("xor rdx, rdx\n");
        printf("mov rax, %s\n", reg_names_64[node->reg]);
        printf("idiv %s\n", reg_names_64[node->right->reg]);
        printf("mov %s, rdx\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "<<")) {
        // r/m <<= CL
        printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
        printf("sal %s, cl\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, ">>")) {
        // r/m >>= CL
        printf("mov rcx, %s\n", reg_names_64[node->right->reg]);
        printf("sar %s, cl\n", reg_names_64[node->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "<")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "l");
        return;
      } else if (IsEqualTokenWithCStr(node->op, ">")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "g");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "<=")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "le");
        return;
      } else if (IsEqualTokenWithCStr(node->op, ">=")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "ge");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "==")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "e");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "!=")) {
        EmitCompareIntegers(node->reg, node->left->reg, node->right->reg, "ne");
        return;
      } else if (IsEqualTokenWithCStr(node->op, "&")) {
        printf("and %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "^")) {
        printf("xor %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      } else if (IsEqualTokenWithCStr(node->op, "|")) {
        printf("or %s, %s\n", reg_names_64[node->reg],
               reg_names_64[node->right->reg]);
        return;
      }
    }
  }
  if (node->type == kASTExprStmt) {
    if (node->left) GenerateForNode(node->left);
    return;
  } else if (node->type == kASTList) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      GenerateForNode(GetNodeAt(node, i));
    }
    return;
  } else if (node->type == kASTDecl) {
    return;
  } else if (node->type == kASTJumpStmt) {
    if (node->op->type == kTokenKwReturn) {
      if (node->right) {
        GenerateForNodeRValue(node->right);
        printf("mov rax, %s\n", reg_names_64[node->right->reg]);
      }
      printf("mov rsp, rbp\n");
      printf("pop rbp\n");
      printf("ret\n");
      return;
    }
    ErrorWithToken(node->op, "GenerateForNode: Not implemented jump stmt");
  } else if (node->type == kASTSelectionStmt) {
    if (node->op->type == kTokenKwIf) {
      GenerateForNodeRValue(node->cond);
      int false_label = GetLabelNumber();
      int end_label = GetLabelNumber();
      EmitConvertToBool(node->cond->reg, node->cond->reg);
      printf("jz L%d\n", false_label);
      GenerateForNodeRValue(node->left);
      printf("jmp L%d\n", end_label);
      printf("L%d:\n", false_label);
      if (node->right) {
        GenerateForNodeRValue(node->right);
      }
      printf("L%d:\n", end_label);
      return;
    }
    ErrorWithToken(node->op, "GenerateForNode: Not implemented jump stmt");
  }
  ErrorWithToken(node->op, "GenerateForNode: Not implemented");
}

static void GenerateForNodeRValue(struct Node *node) {
  GenerateForNode(node);
  if (!node->expr_type || node->expr_type->type != kTypeLValue) return;
  int size = GetSizeOfType(GetRValueType(node->expr_type));
  if (size == 8) {
    printf("mov %s, [%s]\n", reg_names_64[node->reg], reg_names_64[node->reg]);
    return;
  } else if (size == 4) {
    printf("movsxd %s, dword ptr[%s]\n", reg_names_64[node->reg],
           reg_names_64[node->reg]);
    return;
  } else if (size == 1) {
    printf("movsx %s, byte ptr[%s]\n", reg_names_64[node->reg],
           reg_names_64[node->reg]);
    return;
  }
  ErrorWithToken(node->op, "Dereferencing %d bytes is not implemented.", size);
}

void Generate(struct Node *ast) {
  str_list = AllocList();
  printf(".intel_syntax noprefix\n");
  printf(".text\n");
  GenerateForNode(ast);

  printf(".data\n");
  for (int i = 0; i < GetSizeOfList(str_list); i++) {
    struct Node *n = GetNodeAt(str_list, i);
    printf("L%d: ", n->label_number);
    printf(".asciz ");
    PrintTokenStrToFile(n->op, stdout);
    putchar('\n');
  }
}
