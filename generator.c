#include "compilium.h"

static void GenerateForNodeRValue(struct Node *node);

static struct Node *str_list;
static int label_to_break;
static int label_to_continue;

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

static void EmitMoveToMemory(struct Node *op, int dst, int src, int size) {
  if (size == 8) {
    printf("mov [%s], %s\n", reg_names_64[dst], reg_names_64[src]);
    return;
  }
  if (size == 4) {
    printf("mov [%s], %s # 4 byte store\n", reg_names_64[dst],
           reg_names_32[src]);
    return;
  }
  if (size == 1) {
    printf("mov [%s], %s\n", reg_names_64[dst], reg_names_8[src]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitMoveFromMemory(struct Node *op, int dst, int src, int size) {
  if (size == 8) {
    printf("mov %s, [%s]\n", reg_names_64[dst], reg_names_64[src]);
    return;
  }
  if (size == 4) {
    printf("movsxd %s, dword ptr [%s]\n", reg_names_64[dst], reg_names_64[src]);
    return;
  }
  if (size == 1) {
    printf("movsxb %s, byte ptr [%s]\n", reg_names_64[dst], reg_names_64[src]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitAddToMemory(struct Node *op, int dst, int src, int size) {
  if (size == 8) {
    printf("add qword ptr [%s], %s\n", reg_names_64[dst], reg_names_64[src]);
    return;
  }
  if (size == 4) {
    printf("add dword ptr [%s], %s\n", reg_names_64[dst], reg_names_32[src]);
    return;
  }
  if (size == 1) {
    printf("add byte ptr [%s], %s\n", reg_names_64[dst], reg_names_8[src]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitSubFromMemory(struct Node *op, int dst, int src, int size) {
  if (size == 8) {
    printf("sub qword ptr [%s], %s\n", reg_names_64[dst], reg_names_64[src]);
    return;
  }
  if (size == 4) {
    printf("sub dword ptr [%s], %s\n", reg_names_64[dst], reg_names_32[src]);
    return;
  }
  if (size == 1) {
    printf("sub byte ptr [%s], %s\n", reg_names_64[dst], reg_names_8[src]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitDecMemory(struct Node *op, int dst, int size) {
  if (size == 8) {
    printf("dec qword ptr [%s]\n", reg_names_64[dst]);
    return;
  }
  if (size == 4) {
    printf("dec dword ptr [%s]\n", reg_names_64[dst]);
    return;
  }
  if (size == 1) {
    printf("dec byte ptr [%s]\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitIncMemory(struct Node *op, int dst, int size) {
  if (size == 8) {
    printf("inc qword ptr [%s]\n", reg_names_64[dst]);
    return;
  }
  if (size == 4) {
    printf("inc dword ptr [%s]\n", reg_names_64[dst]);
    return;
  }
  if (size == 1) {
    printf("inc byte ptr [%s]\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitMulToMemory(struct Node *op, int dst, int src, int size) {
  if (size == 4) {
    // rdx:rax <- rax * r/m
    printf("xor rdx, rdx\n");
    printf("mov rax, %s\n", reg_names_64[dst]);
    printf("mov eax, [rax]\n");
    printf("imul %s\n", reg_names_64[src]);
    printf("mov [%s], eax\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitDivToMemory(struct Node *op, int dst, int src, int size) {
  if (size == 4) {
    // rax <- rdx:rax / r/m
    printf("xor rdx, rdx\n");
    printf("mov eax, [%s]\n", reg_names_64[dst]);
    printf("idiv %s\n", reg_names_64[src]);
    printf("mov [%s], eax\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitModToMemory(struct Node *op, int dst, int src, int size) {
  if (size == 4) {
    // rdx <- rdx:rax % r/m
    printf("xor rdx, rdx\n");
    printf("mov eax, [%s]\n", reg_names_64[dst]);
    printf("idiv %s\n", reg_names_64[src]);
    printf("mov [%s], edx\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitLShiftMemory(struct Node *op, int dst, int src, int size) {
  if (size == 4) {
    printf("mov ecx, %s\n", reg_names_32[src]);
    printf("shl dword ptr [%s], cl\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

static void EmitRShiftMemory(struct Node *op, int dst, int src, int size) {
  if (size == 4) {
    printf("mov ecx, %s\n", reg_names_32[src]);
    printf("shr dword ptr [%s], cl\n", reg_names_64[dst]);
    return;
  }
  ErrorWithToken(op, "Assigning %d bytes is not implemented.", size);
}

const char *GetParamRegName(struct Node *type, int idx) {
  assert(0 <= idx && idx < NUM_OF_PARAM_REGISTERS);
  int size = GetSizeOfType(type);
  if (size == 8) return param_reg_names_64[idx];
  if (size == 4) return param_reg_names_32[idx];
  if (size == 1) return param_reg_names_8[idx];
  ErrorWithToken(GetIdentifierTokenFromTypeAttr(type),
                 "Assigning %d bytes is not implemented.", size);
}

static void GenerateForNode(struct Node *node) {
  if (node->type == kASTList && !node->op) {
    for (int i = 0; i < GetSizeOfList(node); i++) {
      GenerateForNode(GetNodeAt(node, i));
    }
    return;
  }
  if (node->type == kASTExprFuncCall) {
    printf("sub rsp, %d # alloc stack frame\n", node->stack_size_needed);
    int i;
    for (i = 1; i <= NUM_OF_SCRATCH_REGS; i++) {
      printf("push %s # save scratch regs\n", reg_names_64[i]);
    }
    GenerateForNodeRValue(node->func_expr);
    printf("push %s\n", reg_names_64[node->func_expr->reg]);
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
    for (i = NUM_OF_SCRATCH_REGS; i >= 1; i--) {
      printf("pop %s # restore scratch regs\n", reg_names_64[i]);
    }
    int ret_type_size = GetSizeOfType(node->expr_type);
    if (ret_type_size == 4) {
      printf("movsxd %s, eax\n", reg_names_64[node->reg]);
    } else if (ret_type_size == 8) {
      printf("mov %s, rax\n", reg_names_64[node->reg]);
    } else if (ret_type_size == 0) {
      // Return type is "void". Do nothing.
    } else {
      assert(false);
    }
    printf("add rsp, %d # restore stack frame\n", node->stack_size_needed);
    return;
  } else if (node->type == kASTFuncDef) {
    const char *func_name = CreateTokenStr(node->func_name_token);
    printf(".global %s%s\n", symbol_prefix, func_name);
    printf("%s%s:\n", symbol_prefix, func_name);
    printf("push rbp\n");
    printf("mov rbp, rsp\n");
    printf("push r12\n");
    printf("push r13\n");
    printf("push r14\n");
    printf("push r15\n");
    struct Node *arg_var_list = node->arg_var_list;
    assert(arg_var_list);
    assert(GetSizeOfList(arg_var_list) <= NUM_OF_PARAM_REGISTERS);
    for (int i = 0; i < GetSizeOfList(arg_var_list); i++) {
      struct Node *arg_var = GetNodeAt(arg_var_list, i);
      if (!arg_var) continue;
      const char *param_reg_name = GetParamRegName(arg_var->expr_type, i);
      printf("mov [rbp - %d], %s // arg[%d]\n", arg_var->byte_offset,
             param_reg_name, i);
    }
    GenerateForNode(node->func_body);
    printf("pop r15\n");
    printf("pop r14\n");
    printf("pop r13\n");
    printf("pop r12\n");
    printf("mov rsp, rbp\n");
    printf("pop rbp\n");
    printf("ret\n");
    return;
  }
  assert(node && node->op);
  if (node->type == kASTExpr) {
    if (IsTokenWithType(node->op, kTokenIntegerConstant)) {
      printf("mov %s, %ld\n", reg_names_64[node->reg],
             strtol(node->op->begin, NULL, 0));
      return;
    } else if (IsTokenWithType(node->op, kTokenCharLiteral)) {
      if (node->op->length == (1 + 1 + 1)) {
        printf("mov %s, %d\n", reg_names_64[node->reg], node->op->begin[1]);
        return;
      }
      if (node->op->length == (1 + 2 + 1) && node->op->begin[1] == '\\') {
        if (node->op->begin[2] == 'n') {
          printf("mov %s, %d\n", reg_names_64[node->reg], '\n');
          return;
        }
        if (node->op->begin[2] == '\\') {
          printf("mov %s, %d\n", reg_names_64[node->reg], '\\');
          return;
        }
      }
      ErrorWithToken(node->op, "Not implemented char literal");
    } else if (IsEqualTokenWithCStr(node->op, "(")) {
      GenerateForNode(node->right);
      return;
    } else if (IsEqualTokenWithCStr(node->op, ".")) {
      GenerateForNodeRValue(node->left);
      printf("add %s, %d # struct member ofs\n", reg_names_64[node->reg],
             node->byte_offset);
      return;
    } else if (IsEqualTokenWithCStr(node->op, "->")) {
      GenerateForNodeRValue(node->left);
      printf("add %s, %d # struct member ofs\n", reg_names_64[node->reg],
             node->byte_offset);
      return;
    } else if (IsEqualTokenWithCStr(node->op, "[")) {
      GenerateForNodeRValue(node->left);
      GenerateForNodeRValue(node->right);
      int elem_size = GetSizeOfType(node->expr_type);
      printf("imul %s, %s, %d\n", reg_names_64[node->right->reg],
             reg_names_64[node->right->reg], elem_size);
      printf("add %s, %s\n", reg_names_64[node->left->reg],
             reg_names_64[node->right->reg]);
      return;
    } else if (IsTokenWithType(node->op, kTokenIdent)) {
      if (node->expr_type->type == kTypeFunction) {
        const char *label_name = CreateTokenStr(node->op);
        printf(".global %s%s\n", symbol_prefix, label_name);
        printf("mov %s, [rip + %s%s@GOTPCREL]\n", reg_names_64[node->reg],
               symbol_prefix, label_name);
        return;
      }
      if (!node->byte_offset) {
        // global var
        const char *label_name = CreateTokenStr(node->op);
        printf(".global %s%s\n", symbol_prefix, label_name);
        printf("mov %s, [rip + %s%s@GOTPCREL]\n", reg_names_64[node->reg],
               symbol_prefix, label_name);
        return;
      }
      printf("lea %s, [rbp - %d]\n", reg_names_64[node->reg],
             node->byte_offset);
      return;
    } else if (IsTokenWithType(node->op, kTokenStringLiteral)) {
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
      if (IsEqualTokenWithCStr(node->op, "--")) {
        // Prefix --
        int size = GetSizeOfType(node->expr_type);
        GenerateForNode(node->right);
        EmitDecMemory(node->op, node->reg, GetSizeOfType(node->expr_type));
        EmitMoveFromMemory(node->op, node->reg, node->reg, size);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "++")) {
        // Prefix ++
        int size = GetSizeOfType(node->expr_type);
        GenerateForNode(node->right);
        EmitIncMemory(node->op, node->reg, GetSizeOfType(node->expr_type));
        EmitMoveFromMemory(node->op, node->reg, node->reg, size);
        return;
      }
      if (IsTokenWithType(node->op, kTokenKwSizeof)) {
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
    } else if (node->left && !node->right) {
      if (IsEqualTokenWithCStr(node->op, "++")) {
        // Postfix ++
        int size = GetSizeOfType(node->expr_type);
        GenerateForNode(node->left);
        EmitIncMemory(node->op, node->reg, size);
        EmitMoveFromMemory(node->op, node->reg, node->reg, size);
        printf("sub %s, 1\n", reg_names_64[node->reg]);
        return;
      }
      if (IsEqualTokenWithCStr(node->op, "--")) {
        // Postfix --
        int size = GetSizeOfType(node->expr_type);
        GenerateForNode(node->left);
        EmitDecMemory(node->op, node->reg, GetSizeOfType(node->expr_type));
        EmitMoveFromMemory(node->op, node->reg, node->reg, size);
        printf("add %s, 1\n", reg_names_64[node->reg]);
        return;
      }
      ErrorWithToken(node->op,
                     "GenerateForNode: Not implemented unary postfix op");
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
      } else if (IsEqualTokenWithCStr(node->op, "=") ||
                 IsEqualTokenWithCStr(node->op, "+=") ||
                 IsEqualTokenWithCStr(node->op, "-=") ||
                 IsEqualTokenWithCStr(node->op, "*=") ||
                 IsEqualTokenWithCStr(node->op, "/=") ||
                 IsEqualTokenWithCStr(node->op, "%=") ||
                 IsEqualTokenWithCStr(node->op, "<<=") ||
                 IsEqualTokenWithCStr(node->op, ">>=")) {
        GenerateForNode(node->left);
        GenerateForNodeRValue(node->right);
        int size = GetSizeOfType(node->left->expr_type);
        if (IsEqualTokenWithCStr(node->op, "=")) {
          EmitMoveToMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, "+=")) {
          EmitAddToMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, "-=")) {
          EmitSubFromMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, "*=")) {
          EmitMulToMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, "/=")) {
          EmitDivToMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, "%=")) {
          EmitModToMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, "<<=")) {
          EmitLShiftMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        if (IsEqualTokenWithCStr(node->op, ">>=")) {
          EmitRShiftMemory(node->op, node->left->reg, node->right->reg, size);
          return;
        }
        assert(false);
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
        // rdx <- rdx:rax % r/m
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
    if (IsASTDeclOfTypedef(node)) {
      return;
    }
    assert(node->right && node->right->type == kASTDecltor);
    if (!node->right->decltor_init_expr) return;
    GenerateForNode(node->right->decltor_init_expr);
    return;
  } else if (node->type == kASTJumpStmt) {
    if (IsTokenWithType(node->op, kTokenKwBreak)) {
      if (!label_to_break) {
        ErrorWithToken(node->op, "break is not allowed here");
      }
      printf("jmp L%d\n", label_to_break);
      return;
    }
    if (IsTokenWithType(node->op, kTokenKwContinue)) {
      if (!label_to_continue) {
        ErrorWithToken(node->op, "continue is not allowed here");
      }
      printf("jmp L%d\n", label_to_continue);
      return;
    }
    if (IsTokenWithType(node->op, kTokenKwReturn)) {
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
    if (IsTokenWithType(node->op, kTokenKwIf)) {
      GenerateForNodeRValue(node->cond);
      int false_label = GetLabelNumber();
      int end_label = GetLabelNumber();
      EmitConvertToBool(node->cond->reg, node->cond->reg);
      printf("jz L%d\n", false_label);
      GenerateForNodeRValue(node->if_true_stmt);
      printf("jmp L%d\n", end_label);
      printf("L%d:\n", false_label);
      if (node->if_else_stmt) {
        GenerateForNodeRValue(node->if_else_stmt);
      }
      printf("L%d:\n", end_label);
      return;
    }
    ErrorWithToken(node->op, "GenerateForNode: Not implemented jump stmt");
  } else if (node->type == kASTForStmt) {
    int loop_label = GetLabelNumber();
    int end_label = GetLabelNumber();
    int old_label_to_break = label_to_break;
    label_to_break = end_label;
    int old_label_to_continue = label_to_break;
    label_to_continue = loop_label;
    if (node->init) {
      GenerateForNode(node->init);
    }
    printf("L%d:\n", loop_label);
    if (node->cond) {
      GenerateForNodeRValue(node->cond);
      EmitConvertToBool(node->cond->reg, node->cond->reg);
      printf("jz L%d\n", end_label);
    }
    GenerateForNode(node->body);
    if (node->updt) {
      GenerateForNode(node->updt);
    }
    printf("jmp L%d\n", loop_label);
    printf("L%d:\n", end_label);
    label_to_continue = old_label_to_continue;
    label_to_break = old_label_to_break;
    return;
  } else if (node->type == kASTWhileStmt) {
    int loop_label = GetLabelNumber();
    int end_label = GetLabelNumber();
    int old_label_to_break = label_to_break;
    label_to_break = end_label;
    int old_label_to_continue = label_to_break;
    label_to_continue = loop_label;
    printf("L%d:\n", loop_label);
    GenerateForNodeRValue(node->cond);
    EmitConvertToBool(node->cond->reg, node->cond->reg);
    printf("jz L%d\n", end_label);
    GenerateForNode(node->body);
    printf("jmp L%d\n", loop_label);
    printf("L%d:\n", end_label);
    label_to_continue = old_label_to_continue;
    label_to_break = old_label_to_break;
    return;
  }
  ErrorWithToken(node->op, "GenerateForNode: Not implemented");
}

static void GenerateForNodeRValue(struct Node *node) {
  GenerateForNode(node);
  if (!node->expr_type) return;
  if (node->expr_type->type != kTypeLValue) return;
  if (node->expr_type->type == kTypeLValue &&
      node->expr_type->right->type == kTypeArray)
    return;
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

static void GenerateDataSection(struct SymbolEntry *toplevel_names) {
  printf(".data\n");
  for (int i = 0; i < GetSizeOfList(str_list); i++) {
    struct Node *n = GetNodeAt(str_list, i);
    printf("L%d: ", n->label_number);
    printf(".asciz ");
    PrintTokenStrToFile(n->op, stdout);
    putchar('\n');
  }
  struct SymbolEntry *e = toplevel_names;
  for (; e; e = e->prev) {
    if (e->type != kSymbolGlobalVar) continue;
    int size = GetSizeOfType(e->value);
    fprintf(stderr, "Global Var: %s = %d bytes\n", e->key, size);
    printf(".global %s%s\n", symbol_prefix, e->key);
    printf("%s%s:\n", symbol_prefix, e->key);
    printf(".byte ");
    for (int i = 0; i < size; i++) {
      printf("0%s", i == (size - 1) ? "\n" : ", ");
    }
  }
}

void Generate(struct Node *ast, struct SymbolEntry *toplevel_names) {
  label_to_break = 0;
  label_to_continue = 0;
  str_list = AllocList();
  printf(".intel_syntax noprefix\n");
  printf(".text\n");
  GenerateForNode(ast);
  GenerateDataSection(toplevel_names);
}
