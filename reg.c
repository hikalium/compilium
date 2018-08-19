#include "compilium.h"

// https://wiki.osdev.org/System_V_ABI
// registers should be preserved: rsp, rbp, rbx, r12, r13, r14, r15
// scratch registers: rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11
// return value: rax
// args: rdi, rsi, rdx, rcx, r8, r9, r10, r11

const char *ArgRegNames64[NUM_OF_ARG_REGS] = {"rdi", "rsi", "rdx",
                                              "rcx", "r8",  "r9"};

const char *ScratchRegNames64[NUM_OF_SCRATCH_REGS + 1] = {"NULL", "rbx", "r12",
                                                          "r13",  "r14", "r15"};
const char *ScratchRegNames32[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "ebx", "r12d", "r13d", "r14d", "r15d"};
const char *ScratchRegNames8[NUM_OF_SCRATCH_REGS + 1] = {
    "NULL", "bl", "r12b", "r13b", "r14b", "r15b"};

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
  fprintf(fp, "mov [rbp - %d], %s # Spill vreg[%d]\n", 8 * reg->spill_index,
          ScratchRegNames64[reg->real_reg], reg->vreg_id);
  RealRegToVirtualReg[reg->real_reg] = NULL;
  reg->real_reg = 0;
  printf("\tvreg[%d] is spilled to spill[%d]\n", reg->vreg_id,
         reg->spill_index);
}

void SpillRealRegister(FILE *fp, int rreg) {
  if (!RealRegToVirtualReg[rreg]) return;
  SpillVirtualRegister(fp, RealRegToVirtualReg[rreg]);
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

const char *GetArgumentRegisterName64(int arg_index) {
  return ArgRegNames64[arg_index];
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

void PushAllWorkingRegisters(FILE *fp) {
  fprintf(fp, "push    rbp\n");
  for (int i = 1; i <= NUM_OF_SCRATCH_REGS; i++) {
    fprintf(fp, "push    %s\n", ScratchRegNames64[i]);
  }
  if (NUM_OF_SCRATCH_REGS & 1) {
    fprintf(fp, "push    rbp # dummy for stack alignment\n");
  }
}

void PopAllWorkingRegisters(FILE *fp) {
  if (NUM_OF_SCRATCH_REGS & 1) {
    fprintf(fp, "pop    rbp # dummy for stack alignment\n");
  }
  for (int i = NUM_OF_SCRATCH_REGS; i >= 1; i--) {
    fprintf(fp, "pop    %s\n", ScratchRegNames64[i]);
  }
  fprintf(fp, "pop    rbp\n");
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
  PopAllWorkingRegisters(fp);
  fprintf(fp, "ret\n");
}

int CalcStackFrameSize(ASTFuncDef *func_def) {
  return GetByteSizeOfSpillArea() + func_def->var_stack_size +
         func_def->has_variable_length_args * 6 * 8;
}

int CalcLocalVarOfs(int local_var_ofs) {
  return local_var_ofs + GetByteSizeOfSpillArea();
}

void KillRegister(Register *reg) {
  if (!reg->real_reg) return;
  RealRegToVirtualReg[reg->real_reg] = NULL;
  reg->real_reg = 0;
}
