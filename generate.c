#include "compilium.h"

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
        const char *func_name =
            GetFuncNameStrFromFuncDef(ToASTFuncDef(op->ast_node));
        if (!func_name) {
          Error("func_name is null");
        }
        fprintf(fp, "%s%s:\n", kernel_type == kKernelDarwin ? "_" : "",
                func_name);
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
        const char *dst_name = ScratchRegNames[op->dst_reg];
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
      case kILOpCall: {
        ASTList *call_params = ToASTList(op->ast_node);
        if (!call_params) Error("call_params is not an ASTList");
        for (int i = 1; i < GetSizeOfASTList(call_params); i++) {
          fprintf(fp, "mov rdi, %s\n",
                  ScratchRegNames[ToASTILOp(GetASTNodeAt(call_params, i))
                                      ->dst_reg]);
        }
        ASTIdent *func_ident = ToASTIdent(GetASTNodeAt(call_params, 0));
        if (!func_ident) Error("call_params[0] is not an ASTIdent");
        fprintf(fp, "call _%s\n", func_ident->token->str);
      } break;
      default:
        Error("Not implemented code generation for ILOp%s",
              GetILOpTypeName(op->op));
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
