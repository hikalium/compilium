#include "compilium.h"

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
        SetNumOfSpillEntries(128);
        func_param_requested = 0;
        ASTFuncDef *func_def = ToASTFuncDef(op->ast_node);
        const char *func_name = GetFuncNameTokenFromFuncDef(func_def)->str;
        stack_frame_size = CalcStackFrameSize(func_def);
        assert(func_name);
        fprintf(fp, "%s%s:\n", kernel_type == kKernelDarwin ? "_" : "",
                func_name);
        PushAllWorkingRegisters(fp);
        fprintf(fp, "mov     rbp, rsp\n");
        fprintf(fp, "sub     rsp, %d\n", ((stack_frame_size + 15) >> 4) << 4);
        if (func_def->has_variable_length_args) {
          for (int i = 0; i < NUM_OF_ARG_REGS; i++) {
            fprintf(fp, "mov     [rbp - %d], %s\n", stack_frame_size - 8 * i,
                    GetArgumentRegisterName64(i));
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
        const char *dst = AssignRegister64(fp, op->dst);
        if (op->ast_node->type == kASTInteger) {
          ASTInteger *ast_int = ToASTInteger(op->ast_node);
          fprintf(fp, "mov %s, %d\n", dst, ast_int->value);
          break;
        } else if (op->ast_node->type == kASTString) {
          ASTString *ast_str = ToASTString(op->ast_node);
          int label_for_skip = GetLabelNumber();
          int label_str = GetLabelNumber();
          fprintf(fp, "jmp L%d\n", label_for_skip);
          fprintf(fp, "L%d:\n", label_str);
          fprintf(fp, ".asciz  \"%s\"\n", ast_str->str);
          fprintf(fp, "L%d:\n", label_for_skip);
          fprintf(fp, "lea     %s, [rip + L%d]\n", dst, label_str);
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
        const char *dst = AssignRegister64(fp, op->dst);
        fprintf(fp, "mov %s, %s\n", dst,
                GetArgumentRegisterName64(func_param_requested++));
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
        const char *left = AssignRegister64(fp, op->left);
        const char *dst = AssignRegister64(fp, op->dst);
        const char *right = AssignRegister64(fp, op->right);
        fprintf(fp, "mov rax, %s\n", left);
        fprintf(fp, "push rdx\n");
        fprintf(fp, "imul %s\n", right);
        fprintf(fp, "pop rdx\n");
        fprintf(fp, "mov %s, rax\n", dst);
      } break;
      case kILOpDiv: {
        // rax <- rdx:rax / r/m
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        const char *dst = AssignRegister64(fp, op->dst);
        fprintf(fp, "mov rax, %s\n", left);
        fprintf(fp, "mov rdx, 0\n");
        fprintf(fp, "idiv %s\n", right);
        fprintf(fp, "mov %s, rax\n", dst);
      } break;
      case kILOpMod: {
        // rdx <- rdx:rax % r/m
        const char *left = AssignRegister64(fp, op->left);
        const char *right = AssignRegister64(fp, op->right);
        const char *dst = AssignRegister64(fp, op->dst);
        fprintf(fp, "mov rax, %s\n", left);
        fprintf(fp, "mov rdx, 0\n");
        fprintf(fp, "idiv %s\n", right);
        fprintf(fp, "mov %s, rdx\n", dst);
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
        const char *right = AssignRegister64(fp, op->right);
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "mov rcx, %s\n", right);
        fprintf(fp, "sal %s, cl\n", left);
      } break;
      case kILOpShiftRight: {
        // r/m >>= CL
        const char *right = AssignRegister64(fp, op->right);
        const char *left = AssignRegister64(fp, op->left);
        AssignVirtualRegToRealReg(fp, op->dst, op->left->real_reg);
        fprintf(fp, "mov rcx, %s\n", right);
        fprintf(fp, "sar %s, cl\n", left);
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
          const char *left = AssignRegister64(fp, op->left);
          fprintf(fp, "mov rax, %s\n", left);
        }
        GenerateFuncEpilogue(fp);
        break;
      case kILOpCallParam: {
        // left: param_value
        const char *left = AssignRegister64(fp, op->left);
        fprintf(fp, "mov %s, %s\n", GetArgumentRegisterName64(call_param_used),
                left);
        call_param_used++;
      } break;
      case kILOpCall: {
        // ast_node: func_ident
        ASTIdent *func_ident = ToASTIdent(op->ast_node);
        assert(func_ident);
        fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        fprintf(fp, "call %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
                func_ident->token->str);
        const char *dst = AssignRegister64(fp, op->dst);
        fprintf(fp, "mov %s, rax\n", dst);
        call_param_used = 0;
      } break;
      case kILOpLoadLocalVarAddr: {
        const char *dst = AssignRegister64(fp, op->dst);
        ASTVar *var = ToASTVar(op->ast_node);
        assert(var);
        if (var->is_global) {
          fprintf(fp, "lea %s, [rip + %s%s] # local_var[%s]\n", dst,
                  kernel_type == kKernelDarwin ? "_" : "", var->name,
                  var->name);
        } else {
          fprintf(fp, "lea %s, [rbp - %d] # local_var[%s]\n", dst,
                  CalcLocalVarOfs(var->ofs), var->name);
        }
      } break;
      case kILOpLabel: {
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "L%d:\n", label->label_number);
      } break;
      case kILOpJmp: {
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "jmp L%d\n", label->label_number);
      } break;
      case kILOpJmpIfZero: {
        const char *left = AssignRegister64(fp, op->left);
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
        fprintf(fp, "cmp %s, 0\n", left);
        fprintf(fp, "je L%d\n", label->label_number);
      } break;
      case kILOpJmpIfNotZero: {
        const char *left = AssignRegister64(fp, op->left);
        ASTLabel *label = ToASTLabel(op->ast_node);
        assert(label);
        if (!label->label_number) label->label_number = GetLabelNumber();
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
                va_list_base - 0, 8 * va_index);
        fprintf(fp, "lea rax, [rbp - %d]\n", stack_frame_size);
        fprintf(fp, "mov [rbp - %d], rax # va_list->\n",
                va_list_base - (4 * 2 + 8));
      } break;
      case kILOpData:
        // do notihing
        break;
      case kILOpKill:
        KillRegister(op->dst);
        break;
      default:
        Error("Not implemented code generation for ILOp%s",
              GetILOpTypeName(op->op));
    }
  }
  // generate data section
  fprintf(fp, ".bss\n");
  for (int i = 0; i < GetSizeOfASTList(il); i++) {
    ASTNode *node = GetASTNodeAt(il, i);
    ASTILOp *op = ToASTILOp(node);
    assert(op);
    if (op->op != kILOpData) continue;
    ASTVar *var = ToASTVar(op->ast_node);
    int size = GetSizeOfType(GetExprTypeOfASTNode(op->ast_node));
    fprintf(fp, ".global %s%s\n", kernel_type == kKernelDarwin ? "_" : "",
            var->name);
    fprintf(fp, "%s%s:\n", kernel_type == kKernelDarwin ? "_" : "", var->name);
    if (size == 4) {
      fprintf(fp, ".long 0\n");
    } else if (size == 8) {
      fprintf(fp, ".quad 0\n");
    } else {
      ErrorWithASTNode(var, "global data of size = %d is not implemented",
                       size);
    }
  }
}
