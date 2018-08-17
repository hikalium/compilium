#include "compilium.h"

// https://wiki.osdev.org/System_V_ABI
// registers should be preserved: rbx, rsp, rbp, r12, r13, r14, and r15
// scratch registers: rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11
// return value: rax
// args: rdi, rsi, rdx, rcx, r8, r9, r10, r11

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

#define NUM_OF_ARG_REGS 6

const char *ArgRegNames64[NUM_OF_ARG_REGS] = {"rdi", "rsi", "rdx",
                                              "rcx", "r8",  "r9"};

const char *ScratchRegNames64[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11"};
const char *ScratchRegNames32[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "eax", "edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d"};
const char *ScratchRegNames8[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "al", "dil", "sil", "dl", "cl", "r8b", "r9b", "r10b", "r11b"};

int GetLabelNumber() {
  static int num = 1;
  return num++;
}

int next_spill_index = 1;
int num_of_spill_entries;
int NewSpillIndex(void) {
  if (next_spill_index > num_of_spill_entries) {
    Error("No more spill entries");
  }
  return next_spill_index++;
}
void ResetSpillIndex(void) { next_spill_index = 1; }
void SetNumOfSpillEntries(int num) { num_of_spill_entries = num; }
int GetByteSizeOfSpillArea(void) { return num_of_spill_entries * 8; }

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

void PrintRegisterAssignment() {
  puts("==== ASSIGNMENT ====");
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (RealRegToVirtualReg[i]) {
      printf("\treg[%d] => %s (%d)\n", RealRegToVirtualReg[i]->vreg_id,
             ScratchRegNames64[i], RealRegRefOrder[i]);
    } else {
      printf("\tnone => %s (%d)\n", ScratchRegNames64[i], RealRegRefOrder[i]);
    }
  }
  puts("==== END OF ASSIGNMENT ====");
}

Register *SelectVirtualRegisterToSpill() {
  for (int i = 1; i < NUM_OF_SCRATCH_REGS + 1; i++) {
    if (RealRegToVirtualReg[i]) {
      printf("\tvreg[%d] => %s (%d)\n", RealRegToVirtualReg[i]->vreg_id,
             ScratchRegNames64[i], RealRegRefOrder[i]);
    }
    if (RealRegToVirtualReg[i] &&
        RealRegRefOrder[i] <= order_count - NUM_OF_SCRATCH_REGS)
      return RealRegToVirtualReg[i];
  }
  Error("SelectVirtualRegisterToSpill: NOT_REACHED");
}

void SpillVirtualRegister(FILE *fp, Register *reg) {
  if (!reg->spill_index) reg->spill_index = NewSpillIndex();
  fprintf(fp, "mov [rbp - %d], %s\n", 8 * reg->spill_index,
          ScratchRegNames64[reg->real_reg]);
  RealRegToVirtualReg[reg->real_reg] = NULL;
  reg->real_reg = 0;
  printf("\tvreg[%d] is spilled to spill[%d]\n", reg->vreg_id,
         reg->spill_index);
}

void SpillRealRegister(FILE *fp, int rreg) {
  if (!RealRegToVirtualReg[rreg]) return;
  SpillVirtualRegister(fp, RealRegToVirtualReg[rreg]);
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
           ScratchRegNames64[reg->real_reg]);
    fprintf(fp, "mov %s, %s # vreg %d\n", ScratchRegNames64[real_reg],
            ScratchRegNames64[reg->real_reg], reg->vreg_id);
    RealRegToVirtualReg[reg->real_reg] = NULL;
  } else if (reg->spill_index) {
    printf("\tvreg[%d] is stored at label %d, restoring...\n", reg->vreg_id,
           reg->spill_index);
    fprintf(fp, "mov %s, [rbp - %d]\n", ScratchRegNames64[real_reg],
            8 * reg->spill_index);
  }
  RealRegToVirtualReg[real_reg] = reg;
  RealRegRefOrder[real_reg] = order_count++;
  reg->real_reg = real_reg;
  printf("\tvirtual_reg[%d] => %s\n", reg->vreg_id,
         ScratchRegNames64[real_reg]);
}

int AssignRegister(FILE *fp, Register *reg) {
  printf("requested vreg_id = %d\n", reg->vreg_id);
  if (reg->real_reg) {
    printf("\texisted on %s\n", ScratchRegNames64[reg->real_reg]);
    RealRegRefOrder[reg->real_reg] = order_count++;
    return reg->real_reg;
  }
  int real_reg = FindFreeRealReg(fp);
  AssignVirtualRegToRealReg(fp, reg, real_reg);
  return real_reg;
}

const char *AssignRegister64(FILE *fp, Register *reg) {
  return ScratchRegNames64[AssignRegister(fp, reg)];
}

const char *AssignRegister32(FILE *fp, Register *reg) {
  return ScratchRegNames32[AssignRegister(fp, reg)];
}

const char *AssignRegister8(FILE *fp, Register *reg) {
  return ScratchRegNames8[AssignRegister(fp, reg)];
}

const char *GetParamRegister(int param_index) {
  // param-index: 1-based
  if (param_index < 1 || NUM_OF_SCRATCH_REGS <= param_index) {
    Error("param_index exceeded (%d)", param_index);
  }
  return ScratchRegNames64[param_index];
}

void GenerateFuncEpilogue(FILE *fp) {
  fprintf(fp, "mov     rsp, rbp\n");
  fprintf(fp, "pop     rbp\n");
  fprintf(fp, "ret\n");
}

int CalcStackFrameSize(ASTFuncDef *func_def) {
  return GetByteSizeOfSpillArea() + GetSizeOfContext(func_def->context) +
         func_def->has_variable_length_args * 6 * 8;
}

int CalcLocalVarOfs(int local_var_ofs) {
  return local_var_ofs + GetByteSizeOfSpillArea();
}

void GenerateCode(FILE *fp, ASTList *il, KernelType kernel_type) {
  // Stack layout:
  // qword [rbp]: return address
  // qword [rbp - 8 * i]: ith spill area
  // qword [rbp - GetByteSizeOfSpillArea()]: last spill area
  //
  // nth local variable can be accessed as [rbp - 8 * n] (n is one based)
  // push qword: SS[rsp -= 8] = data64;
  // pop qword: data64 = SS[rsp]; rsp += 8;
  int func_param_requested = 0;
  int call_param_used = 0;

  fputs(".intel_syntax noprefix\n", fp);
  // generate func symbol
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    if (op->op == kILOpFuncBegin) {
      const char *func_name =
          GetFuncNameTokenFromFuncDef(ToASTFuncDef(op->ast_node))->str;
      assert(func_name);
      fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
              func_name);
    }
  }
  // generate code
  int stack_frame_size;
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    assert(op);
    switch (op->op) {
      case kILOpFuncBegin: {
        ClearRegisterAllocation();
        ResetSpillIndex();
        // TODO: The number of spill entries should be determined automatically
        SetNumOfSpillEntries(2048);
        func_param_requested = 0;
        ASTFuncDef *func_def = ToASTFuncDef(op->ast_node);
        const char *func_name = GetFuncNameTokenFromFuncDef(func_def)->str;
        stack_frame_size = CalcStackFrameSize(func_def);
        assert(func_name);
        fprintf(fp, "%s%s:\n", kernel_type == kKernelDarwin ? "_" : "",
                func_name);
        fprintf(fp, "push    rbp\n");
        fprintf(fp, "mov     rbp, rsp\n");
        fprintf(fp, "sub     rsp, %d\n", ((stack_frame_size + 15) >> 4) << 4);
        if (func_def->has_variable_length_args) {
          for (int i = 0; i < NUM_OF_ARG_REGS; i++) {
            fprintf(fp, "mov     [rbp - %d], %s\n",
                    stack_frame_size - 8 * (1 + NUM_OF_ARG_REGS - i),
                    ArgRegNames64[i]);
          }
        }
      } break;
      case kILOpFuncEnd:
        GenerateFuncEpilogue(fp);
        break;
      case kILOpLoad8: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "movsx %s, byte ptr [%s]\n", dst, left);
      } break;
      case kILOpLoad32: {
        const char *dst = AssignRegister32(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "mov %s, [%s]\n", dst, left);
      } break;
      case kILOpLoad64: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "mov %s, [%s]\n", dst, left);
      } break;
      case kILOpStore8: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister8(fp, op->left);
        fprintf(fp, "mov [%s], %s\n", dst, left);
      } break;
      case kILOpStore32: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister32(fp, op->left);
        fprintf(fp, "mov [%s], %s\n", dst, left);
      } break;
      case kILOpStore64: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "mov [%s], %s\n", dst, left);
      } break;
      case kILOpLoadImm: {
        const char *dst_name = AssignRegister64(fp, op->dst);
        if (op->ast_node->type == kASTInteger) {
          ASTInteger *ast_int = ToASTInteger(op->ast_node);
          fprintf(fp, "mov %s, %d\n", dst_name, ast_int->value);
          break;
        } else if (op->ast_node->type == kASTString) {
          ASTString *ast_str = ToASTString(op->ast_node);
          int label_for_skip = GetLabelNumber();
          int label_str = GetLabelNumber();
          fprintf(fp, "jmp L%d\n", label_for_skip);
          fprintf(fp, "L%d:\n", label_str);
          fprintf(fp, ".asciz  \"%s\"\n", ast_str->str);
          fprintf(fp, "L%d:\n", label_for_skip);
          fprintf(fp, "lea     %s, [rip + L%d]\n", dst_name, label_str);
          break;
        }
        Error("kILOpLoadImm: not implemented for AST type %d",
              op->ast_node->type);
      } break;
      case kILOpLoadIdent: {
        const char *dst_name = AssignRegister64(fp, op->dst);
        ASTIdent *ident = ToASTIdent(op->ast_node);
        assert(ident);
        fprintf(fp, "lea     %s, [rip + %s%s]\n", dst_name,
                kernel_type == kKernelDarwin ? "_" : "", ident->token->str);
      } break;
      case kILOpLoadArg: {
        AssignVirtualRegToRealReg(fp, op->dst,
                                  REAL_REG_RDI + func_param_requested++);
      } break;
      case kILOpAdd: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "add %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpSub: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "sub %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpMul: {
        // rdx:rax <- rax * r/m
        AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        const char *dst = AssignRegister64(fp, op->dst);
        const char *right = AssignRegister64(fp, op->right);
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
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "and %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpXor: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "xor %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpOr: {
        const char *dst = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "or %s, %s\n", left, right);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpNot: {
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "not %s\n", left);
      } break;
      case kILOpNegate: {
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "neg %s\n", left);
      } break;
      case kILOpLogicalAnd: {
        const char *dst = AssignRegister8(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, " %s, %s\n", left, right);
        fprintf(fp, "setnz %s\n", dst);
      } break;
      case kILOpLogicalOr: {
        const char *dst = AssignRegister8(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "xor rax, rax\n");
        fprintf(fp, "or %s, %s\n", left, right);
        fprintf(fp, "setnz %s\n", dst);
      } break;
      case kILOpLogicalNot: {
        const char *dst8 = AssignRegister8(fp, op->dst);
        const char *dst64 = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "setz %s\n", dst8);
        fprintf(fp, "and %s, 1\n", dst64);
      } break;
      case kILOpShiftLeft: {
        // r/m <<= CL
        AssignVirtualRegToRealReg(fp, op->right, REAL_REG_RCX);
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "SAL %s, cl\n", left);
      } break;
      case kILOpShiftRight: {
        // r/m >>= CL
        AssignVirtualRegToRealReg(fp, op->right, REAL_REG_RCX);
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "SAR %s, cl\n", left);
      } break;
      case kILOpIncrement: {
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "inc %s\n", left);
      } break;
      case kILOpDecrement: {
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "dec %s\n", left);
      } break;
      case kILOpCmpG:
      case kILOpCmpGE:
      case kILOpCmpL:
      case kILOpCmpLE:
      case kILOpCmpE:
      case kILOpCmpNE: {
        const char *cc = NULL;
        if (op->op == kILOpCmpG) cc = "g";
        if (op->op == kILOpCmpGE) cc = "ge";
        if (op->op == kILOpCmpL) cc = "l";
        if (op->op == kILOpCmpLE) cc = "le";
        if (op->op == kILOpCmpE) cc = "e";
        if (op->op == kILOpCmpNE) cc = "ne";
        assert(cc);
        const char *dst8 = AssignRegister8(fp, op->dst);
        const char *dst64 = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "xor %s, %s\n", dst64, dst64);
        fprintf(fp, "cmp %s, %s\n", left, right);
        fprintf(fp, "set%s %s\n", cc, dst8);
      } break;
      case kILOpReturn:
        // left: reg which have return value (can be NULL)
        if (op->left) {
          AssignVirtualRegToRealReg(fp, op->left, REAL_REG_RAX);
        }
        GenerateFuncEpilogue(fp);
        break;
      case kILOpCallParam: {
        // left: param_value
        AssignVirtualRegToRealReg(fp, op->left, call_param_used + REAL_REG_RDI);
        call_param_used++;
      } break;
      case kILOpCall: {
        // ast_node: func_ident
        ASTIdent *func_ident = ToASTIdent(op->ast_node);
        assert(func_ident);
        fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        SpillAllRealRegisters(fp);
        fprintf(fp, "call %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        AssignVirtualRegToRealReg(fp, op->dst, REAL_REG_RAX);
        call_param_used = 0;
      } break;
      case kILOpLoadLocalVarAddr: {
        const char *dst = AssignRegister64(fp, op->dst);
        ASTVar *var = ToASTVar(op->ast_node);
        assert(var);
        fprintf(fp, "lea %s, [rbp - %d] # local_var[%s]\n", dst,
                CalcLocalVarOfs(var->ofs), var->name);
      } break;
      case kILOpLabel: {
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        SpillAllRealRegisters(fp);
        fprintf(fp, "L%d:\n", label->label_number);
      } break;
      case kILOpJmp: {
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        SpillAllRealRegisters(fp);
        fprintf(fp, "jmp L%d\n", label->label_number);
      } break;
      case kILOpJmpIfZero: {
        const char *left = AssignRegister64(fp, op->left);
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        SpillAllRealRegisters(fp);
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "je L%d\n", label->label_number);
      } break;
      case kILOpJmpIfNotZero: {
        const char *left = AssignRegister64(fp, op->left);
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        SpillAllRealRegisters(fp);
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "jne L%d\n", label->label_number);
      } break;
      case kILOpSetLogicalValue: {
        const char *dst8 = AssignRegister8(fp, op->dst);
        const char *dst64 = AssignRegister64(fp, op->dst);
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "xor %s, %s\n", dst64, dst64);
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "setne %s\n", dst8);
      } break;
      case kILOpAssign: {
        const char *left = AssignRegister64(fp, op->left);
        const char *dst = AssignRegister64(fp, op->dst);
        fprintf(fp, "mov %s, %s\n", dst, left);
      } break;
      case kILOpVAStart: {
        ASTExprFuncCall *func_call = ToASTExprFuncCall(op->ast_node);
        ASTList *func_call_args = ToASTList(func_call->args);
        int va_list_base = CalcLocalVarOfs(
            ToASTIdent(GetASTNodeAt(func_call_args, 0))->local_var->ofs);
        int va_index =
            ToASTIdent(GetASTNodeAt(func_call_args, 1))->local_var->arg_index;
        fprintf(fp, "mov dword ptr [rbp - %d], %d # va_list->gp_offset\n",
                va_list_base - 0, 8 * (NUM_OF_ARG_REGS - va_index - 2));
        fprintf(fp, "push rax\n");
        fprintf(fp, "lea rax, [rbp - %d]\n", stack_frame_size);
        fprintf(fp, "mov [rbp - %d], rax # va_list->\n",
                va_list_base - (4 * 2 + 8));
        fprintf(fp, "pop rax\n");
      } break;
      default:
        Error("Not implemented code generation for ILOp%s",
              GetILOpTypeName(op->op));
    }
  }
}
