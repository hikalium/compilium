#include "compilium.h"

// https://wiki.osdev.org/System_V_ABI
// registers should be preserved: rbx, rsp, rbp, r12, r13, r14, and r15
// scratch registers: rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11
// return value: rax
// args: rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11

#define NUM_OF_SCRATCH_REGS 9

#define REAL_REG_RAX 1
#define REAL_REG_RDI 2
#define REAL_REG_RSI 3
#define REAL_REG_RDX 4
#define REAL_REG_RCX 5
#define REAL_REG_R8 6
#define REAL_REG_R9 7
#define REAL_REG_R10 8
#define REAL_REG_R11 9

const char *ScratchRegNames[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11"};

int GetLabelNumber() {
  static int num = 1;
  return num++;
}

int spill_label_num_next = 1;
int GetSpillLabelNumber() { return spill_label_num_next++; }

Register *RealRegToVirtualReg[NUM_OF_SCRATCH_REGS + 1];
int RealRegRefOrder[NUM_OF_SCRATCH_REGS + 1];
int order_count = 1;

void ClearRegisterAllocation() {
  for (int i = 0; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    RealRegToVirtualReg[i] = 0;
    RealRegRefOrder[i] = 0;
  }
  order_count = 1;
}

void GenerateSpillData(FILE *fp) {
  fprintf(fp, ".data\n");
  for (int i = 1; i < spill_label_num_next; i++) {
    fprintf(fp, "LSpill%d: .quad 0\n", i);
  }
}

void PrintRegisterAssignment() {
  puts("==== ASSIGNMENT ====");
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (RealRegToVirtualReg[i]) {
      printf("\treg[%d] => %s (%d)\n", RealRegToVirtualReg[i]->vreg_id,
             ScratchRegNames[i], RealRegRefOrder[i]);
    } else {
      printf("\tnone => %s (%d)\n", ScratchRegNames[i], RealRegRefOrder[i]);
    }
  }
  puts("==== END OF ASSIGNMENT ====");
}

Register *SelectVirtualRegisterToSpill() {
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (RealRegToVirtualReg[i]) {
      printf("\tvreg[%d] => %s (%d)\n", RealRegToVirtualReg[i]->vreg_id,
             ScratchRegNames[i], RealRegRefOrder[i]);
    }
    if (RealRegToVirtualReg[i] &&
        RealRegRefOrder[i] <= order_count - NUM_OF_SCRATCH_REGS)
      return RealRegToVirtualReg[i];
  }
  Error("SelectVirtualRegisterToSpill: NOT_REACHED");
  return NULL;
}

void SpillVirtualRegister(FILE *fp, Register *reg) {
  if (!reg->save_label_num) reg->save_label_num = GetSpillLabelNumber();
  //
  /*
  fprintf(fp, "mov [rip + LSpill%d], %s\n", reg->save_label_num,
          ScratchRegNames[reg->real_reg]);
          */
  int byte_ofs = 8 * reg->save_label_num;
  fprintf(fp, "mov [rbp - %d], %s\n", byte_ofs, ScratchRegNames[reg->real_reg]);
  //
  RealRegToVirtualReg[reg->real_reg] = NULL;
  reg->real_reg = 0;
  printf("\tvreg[%d] is spilled to LSpill%d\n", reg->vreg_id,
         reg->save_label_num);
}

void SpillRealRegister(FILE *fp, int rreg) {
  if (RealRegToVirtualReg[rreg]) {
    SpillVirtualRegister(fp, RealRegToVirtualReg[rreg]);
  }
}

void SpillAllRealRegisters(FILE *fp) {
  for (int i = 1; i < 1 + NUM_OF_SCRATCH_REGS; i++) {
    SpillRealRegister(fp, i);
  }
}

int FindFreeRealReg(FILE *fp) {
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (!RealRegToVirtualReg[i]) return i;
  }
  PrintRegisterAssignment();
  SpillVirtualRegister(fp, SelectVirtualRegisterToSpill());
  PrintRegisterAssignment();
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (!RealRegToVirtualReg[i]) return i;
  }
  Error("FindFreeRealReg: NOT_REACHED");
  return 0;
}

void AssignVirtualRegToRealReg(FILE *fp, Register *reg, int real_reg) {
  if (reg->real_reg == real_reg) {
    // already satisfied.
    RealRegRefOrder[real_reg] = order_count++;
    return;
  }
  // first, free target real reg.
  SpillRealRegister(fp, real_reg);
  // next, assign virtual reg to real reg.
  if (reg->real_reg) {
    printf("\tvreg[%d] is stored on %s, moving...\n", reg->vreg_id,
           ScratchRegNames[reg->real_reg]);
    fprintf(fp, "mov %s, %s # vreg %d\n", ScratchRegNames[real_reg],
            ScratchRegNames[reg->real_reg], reg->vreg_id);
    RealRegToVirtualReg[reg->real_reg] = NULL;
  } else if (reg->save_label_num) {
    printf("\tvreg[%d] is stored at label %d, restoring...\n", reg->vreg_id,
           reg->save_label_num);
    /*
    fprintf(fp, "mov %s, [rip + LSpill%d]\n", ScratchRegNames[real_reg],
            reg->save_label_num);
            */
    int byte_ofs = 8 * reg->save_label_num;
    fprintf(fp, "mov %s, [rbp - %d]\n", ScratchRegNames[real_reg], byte_ofs);
  }
  RealRegToVirtualReg[real_reg] = reg;
  RealRegRefOrder[real_reg] = order_count++;
  reg->real_reg = real_reg;
  printf("\tvirtual_reg[%d] => %s\n", reg->vreg_id, ScratchRegNames[real_reg]);
}

const char *AssignRegister(FILE *fp, Register *reg) {
  printf("requested vreg_id = %d\n", reg->vreg_id);
  if (reg->real_reg) {
    printf("\texisted on %s\n", ScratchRegNames[reg->real_reg]);
    RealRegRefOrder[reg->real_reg] = order_count++;
    return ScratchRegNames[reg->real_reg];
  }
  int real_reg = FindFreeRealReg(fp);
  AssignVirtualRegToRealReg(fp, reg, real_reg);
  return ScratchRegNames[real_reg];
}

const char *GetParamRegister(int param_index) {
  // param-index: 1-based
  if (param_index < 1 || NUM_OF_SCRATCH_REGS <= param_index) {
    Error("param_index exceeded (%d)", param_index);
  }
  return ScratchRegNames[param_index];
}

void GenerateFuncEpilogue(FILE *fp) {
  fprintf(fp, "mov     rsp, rbp\n");
  fprintf(fp, "pop     rbp\n");
  fprintf(fp, "ret\n");
}

int func_param_requested = 0;
int local_var_base_in_stack = 256;

void GenerateCode(FILE *fp, ASTList *il, KernelType kernel_type) {
  fputs(".intel_syntax noprefix\n", fp);
  // generate func symbol
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    if (op->op == kILOpFuncBegin) {
      const char *func_name =
          GetFuncNameTokenFromFuncDef(ToASTFuncDef(op->ast_node))->str;
      if (!func_name) {
        Error("func_name is null");
      }
      fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
              func_name);
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
        // nth local variable can be accessed as [rbp - 8 * n] (n is one based)
        // push qword: SS[rsp -= 8] = data64;
        // pop qword: data64 = SS[rsp]; rsp += 8;
        ClearRegisterAllocation();
        func_param_requested = 0;
        ASTFuncDef *func_def = ToASTFuncDef(op->ast_node);
        const char *func_name = GetFuncNameTokenFromFuncDef(func_def)->str;
        if (!func_name) {
          Error("func_name is null");
        }
        fprintf(fp, "%s%s:\n", kernel_type == kKernelDarwin ? "_" : "",
                func_name);
        fprintf(fp, "push    rbp\n");
        fprintf(fp, "mov     rbp, rsp\n");
        fprintf(fp, "mov     rax, 0xf\n");
        fprintf(fp, "not     rax\n");
        fprintf(fp, "sub     rsp, %d\n",
                local_var_base_in_stack +
                    GetStackSizeForContext(func_def->context));
        fprintf(fp, "and     rsp, rax\n");
      } break;
      case kILOpFuncEnd:
        GenerateFuncEpilogue(fp);
        break;
      case kILOpLoadImm: {
        const char *dst_name = AssignRegister(fp, op->dst);
        //
        ASTConstant *val = ToASTConstant(op->ast_node);
        switch (val->token->type) {
          case kInteger: {
            char *p;
            const char *s = val->token->str;
            int n = strtol(s, &p, 0);
            if (!(s[0] != 0 && *p == 0)) {
              Error("%s is not valid as integer.", s);
            }
            fprintf(fp, "mov %s, %d\n", dst_name, n);

          } break;
          case kStringLiteral: {
            int label_for_skip = GetLabelNumber();
            int label_str = GetLabelNumber();
            fprintf(fp, "jmp L%d\n", label_for_skip);
            fprintf(fp, "L%d:\n", label_str);
            fprintf(fp, ".asciz  \"%s\"\n", val->token->str);
            fprintf(fp, "L%d:\n", label_for_skip);
            fprintf(fp, "lea     %s, [rip + L%d]\n", dst_name, label_str);
          } break;
          default:
            Error("kILOpLoadImm: not implemented for token type %d",
                  val->token->type);
        }
      } break;
      case kILOpLoadIdent: {
        const char *dst_name = AssignRegister(fp, op->dst);
        //
        ASTIdent *ident = ToASTIdent(op->ast_node);
        switch (ident->token->type) {
          case kIdentifier: {
            fprintf(fp, "lea     %s, [rip + %s%s]\n", dst_name,
                    kernel_type == kKernelDarwin ? "_" : "", ident->token->str);
          } break;
          default:
            Error("kILOpLoadIdent: not implemented for token type %d",
                  ident->token->type);
        }
      } break;
      case kILOpLoadArg: {
        AssignVirtualRegToRealReg(
            fp, op->dst, REAL_REG_RDI + func_param_requested++);  // TODO:
      } break;
      case kILOpAdd: {
        const char *dst = AssignRegister(fp, op->dst);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "add %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpSub: {
        const char *dst = AssignRegister(fp, op->dst);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "sub %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpMul: {
        // rdx:rax <- rax * r/m
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        const char *dst = AssignRegister(fp, op->dst);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "push rdx\n");
        fprintf(fp, "imul %s\n", right);
        fprintf(fp, "pop rdx\n");
        fprintf(fp, "mov %s, rax\n", dst);
      } break;
      case kILOpDiv: {
        // rax <- rdx:rax / r/m
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        AssignVirtualRegToRealReg(fp, op->right, REAL_REG_RCX);
        SpillRealRegister(fp, REAL_REG_RDX);
        fprintf(fp, "mov rdx, 0\n");
        fprintf(fp, "idiv rcx\n");
      } break;
      case kILOpMod: {
        // rdx <- rdx:rax % r/m
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        AssignVirtualRegToRealReg(fp, op->right, REAL_REG_RCX);
        SpillRealRegister(fp, REAL_REG_RDX);
        fprintf(fp, "mov rdx, 0\n");
        fprintf(fp, "idiv rcx\n");
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RDX);
      } break;
      case kILOpAnd: {
        const char *dst = AssignRegister(fp, op->dst);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "and %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpXor: {
        const char *dst = AssignRegister(fp, op->dst);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpOr: {
        const char *dst = AssignRegister(fp, op->dst);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "or %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpNegate: {
        const char *left = AssignRegister(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "neg %s\n", left);
      } break;
      case kILOpLogicalAnd: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, " %s, %s\n", left, right);
        fprintf(fp, "setnz al\n");
      } break;
      case kILOpLogicalOr: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "or %s, %s\n", left, right);
        fprintf(fp, "setnz al\n");
      } break;
      case kILOpShiftLeft: {
        // r/m <<= CL
        // TODO: left and dst can be any register
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        AssignVirtualRegToRealReg(fp, op->right, REAL_REG_RCX);
        fprintf(fp, "SAL rax, cl\n");
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
      } break;
      case kILOpShiftRight: {
        // r/m >>= CL
        // TODO: left and dst can be any register
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        AssignVirtualRegToRealReg(fp, op->right, REAL_REG_RCX);
        fprintf(fp, "SAR rax, cl\n");
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
      } break;
      case kILOpCmpG: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "setg al\n");
      } break;
      case kILOpCmpGE: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "setge al\n");
      } break;
      case kILOpCmpL: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "setl al\n");
      } break;
      case kILOpCmpLE: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "setle al\n");
      } break;
      case kILOpCmpE: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "sete al\n");
      } break;
      case kILOpCmpNE: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        const char *right = AssignRegister(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "setne al\n");
      } break;
      case kILOpReturn:
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        GenerateFuncEpilogue(fp);
        break;
      case kILOpCall: {
        ASTList *call_params = ToASTList(op->ast_node);
        if (!call_params) Error("call_params is not an ASTList");
        for (int i = 1; i < GetSizeOfASTList(call_params); i++) {
          AssignVirtualRegToRealReg(
              fp, ToASTILOp(GetASTNodeAt(call_params, i))->dst, i + 1);
        }
        ASTIdent *func_ident = ToASTIdent(GetASTNodeAt(call_params, 0));
        if (!func_ident) Error("call_params[0] is not an ASTIdent");
        fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        SpillAllRealRegisters(fp);
        fprintf(fp, "call %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
      } break;
      case kILOpWriteLocalVar: {
        const char *right = AssignRegister(fp, op->right);
        ASTLocalVar *var = ToASTLocalVar(op->ast_node);
        if (!var) Error("var is not an ASTLocalVar");
        int byte_ofs = 8 * var->ofs_in_stack + local_var_base_in_stack;
        fprintf(fp, "mov [rbp - %d], %s # local_var[%s] @ %d\n", byte_ofs,
                right, var->name, var->ofs_in_stack);
      } break;
      case kILOpReadLocalVar: {
        const char *dst = AssignRegister(fp, op->dst);
        ASTLocalVar *var = ToASTLocalVar(op->ast_node);
        if (!var) Error("var is not an ASTLocalVar");
        int byte_ofs = 8 * var->ofs_in_stack + local_var_base_in_stack;
        fprintf(fp, "mov %s, [rbp - %d] # local_var[%s] @ %d\n", dst, byte_ofs,
                var->name, var->ofs_in_stack);
      } break;
      case kILOpLabel: {
        ASTLabel *label = ToASTLabel(op->ast_node);
        if (!label) Error("Label is null");
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "L%d:\n", label->label_number);
        SpillAllRealRegisters(fp);
      } break;
      case kILOpJmp: {
        ASTLabel *label = ToASTLabel(op->ast_node);
        if (!label) Error("Label is null");
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "jmp L%d\n", label->label_number);
      } break;
      case kILOpJmpIfZero: {
        const char *left = AssignRegister(fp, op->left);
        ASTLabel *label = ToASTLabel(op->ast_node);
        if (!label) Error("Label is null");
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "je L%d\n", label->label_number);
      } break;
      case kILOpJmpIfNotZero: {
        const char *left = AssignRegister(fp, op->left);
        ASTLabel *label = ToASTLabel(op->ast_node);
        if (!label) Error("Label is null");
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "jne L%d\n", label->label_number);
      } break;
      case kILOpSetLogicalValue: {
        // TODO: dst can be any registers which can access as a byte reg
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        const char *left = AssignRegister(fp, op->left);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "setne al\n");
      } break;
      default:
        Error("Not implemented code generation for ILOp%s",
              GetILOpTypeName(op->op));
    }
  }
  GenerateSpillData(fp);
}
