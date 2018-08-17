#include "compilium.h"

typedef struct {
  KernelType kernel_type;
  int is_parse_only;
  const char *src_file;
  const char *dst_file;
  FILE *dst_fp;
} CompilerArgs;

void ParseArgs(CompilerArgs *args, int argc, char **argv) {
  args->kernel_type = kKernelDarwin;
  args->is_parse_only = 0;
  args->src_file = NULL;
  args->dst_file = "out.S";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--prefix_type") == 0) {
      i++;
      if (strcmp(argv[i], "Darwin") == 0) {
        args->kernel_type = kKernelDarwin;
      } else if (strcmp(argv[i], "Linux") == 0) {
        args->kernel_type = kKernelLinux;
      } else {
        Error("Unknown kernel type %s", argv[i]);
      }
    } else if (strcmp(argv[i], "--parse_only") == 0) {
      args->is_parse_only = 1;
    } else if (strcmp(argv[i], "-o") == 0) {
      i++;
      args->dst_file = argv[i];
    } else if (strcmp(argv[i], "-I") == 0) {
      i++;
      include_path = argv[i];
    } else {
      args->src_file = argv[i];
    }
  }
  if (!args->src_file) Error("Usage: %s -o <dst_file> <src_file>", argv[0]);
  args->dst_fp = fopen(args->dst_file, "wb");
  if (!args->dst_fp) Error("Failed to open %s", args->dst_file);
}

#define MAX_TOKENS 16384
int main(int argc, char **argv) {
  CompilerArgs args;

  InitASTNodeTypeName();
  InitILOpTypeName();
  InitGlobalContext();

  ParseArgs(&args, argc, argv);

  TokenList *tokens = AllocTokenList(MAX_TOKENS);

  Tokenize(tokens,
           "typedef struct{int gp_ofs;int "
           "fp_ofs;void*ov_arg_area;void*reg_area;}__builtin_va_list[1];",
           "builtin");
  TokenizeFile(tokens, args.src_file);
  DebugPrintTokenList(tokens);
  ASTNode *ast = Parse(tokens);
  Analyze(ast);
  if (args.is_parse_only) return 0;
  ASTList *il = GenerateIL(ast);
  GenerateCode(args.dst_fp, il, args.kernel_type);

  return 0;
}
