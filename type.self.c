
void assert();
typedef void FILE;
void *calloc(unsigned long, unsigned long);
int printf(const char *, ...);
int putchar(int);

typedef enum {
  kIdentifier,
  kStringLiteral,
  kCharacterLiteral,
  kInteger,
  kPunctuator,
  kKeyword,
} TokenType;

typedef enum {
  kASTFuncDef,
  kASTCompStmt,
  kASTExprUnaryPreOp,
  kASTExprUnaryPostOp,
  kASTExprBinOp,
  kASTExprFuncCall,
  kASTInteger,
  kASTString,
  kASTExprStmt,
  kASTJumpStmt,
  kASTCondStmt,
  kASTIfStmt,
  kASTWhileStmt,
  kASTForStmt,
  kASTILOp,
  kASTList,
  kASTKeyword,
  kASTDecltor,
  kASTDirectDecltor,
  kASTIdent,
  kASTDecl,
  kASTParamDecl,
  kASTPointer,
  kASTStructDecl,
  kASTStructSpec,

  kASTDict,
  kASTVar,
  kASTLabel,
  kASTType,

  kNumOfASTNodeType
} ASTNodeType;

typedef enum {
  kTypeNone,
  kTypeLValueOf,
  kTypePointerOf,
  kTypeArrayOf,
  kTypeStruct,
  kTypeFunction,
  kTypeVoid,
  kTypeChar,
  kTypeInt,
} BasicType;

typedef enum {
  kILOpNop,
  kILOpAdd,
  kILOpSub,
  kILOpMul,
  kILOpDiv,
  kILOpMod,
  kILOpAnd,
  kILOpXor,
  kILOpOr,
  kILOpNot,
  kILOpNegate,
  kILOpLogicalAnd,
  kILOpLogicalOr,
  kILOpLogicalNot,
  kILOpShiftLeft,
  kILOpShiftRight,
  kILOpIncrement,
  kILOpDecrement,
  kILOpCmpG,
  kILOpCmpGE,
  kILOpCmpL,
  kILOpCmpLE,
  kILOpCmpE,
  kILOpCmpNE,
  kILOpLoad8,
  kILOpLoad32,
  kILOpLoad64,
  kILOpStore8,
  kILOpStore32,
  kILOpStore64,
  kILOpLoadImm,
  kILOpLoadIdent,
  kILOpLoadArg,
  kILOpFuncBegin,
  kILOpFuncEnd,
  kILOpReturn,
  kILOpCall,
  kILOpCallParam,
  kILOpLoadLocalVarAddr,
  kILOpLabel,
  kILOpJmp,
  kILOpJmpIfZero,
  kILOpJmpIfNotZero,
  kILOpSetLogicalValue,
  kILOpAssign,

  kNumOfILOpFunc
} ILOpType;

typedef enum {
  kKernelDarwin,
  kKernelLinux,
} KernelType;

typedef struct TOKEN_LIST TokenList;
typedef struct TOKEN_STREAM TokenStream;
typedef struct AST_LIST ASTList;
typedef struct AST_DICT ASTDict;
typedef struct CONTEXT Context;
typedef struct AST_LABEL ASTLabel;
typedef struct AST_VAR ASTVar;
typedef struct AST_TYPE ASTType;

typedef struct {
  int vreg_id;
  int spill_index;
  int real_reg;
} Register;

typedef struct {
  char str[64 + 1];
  TokenType type;
  const char *filename;
  int line;
} Token;

typedef struct {
  ASTNodeType type;
} ASTNode;

typedef struct {
  ASTNodeType type;
  const Token *token;
} ASTKeyword;

typedef struct {
  ASTNodeType type;
  const Token *token;
  ASTVar *local_var;
  ASTType *var_type;
} ASTIdent;

typedef struct {
  ASTNodeType type;
  ASTList *root_list;
} ASTRoot;

typedef struct {
  ASTNodeType type;
  ASTList *stmt_list;
} ASTCompStmt;

typedef struct {
  ASTNodeType type;
  const Token *op;
  ASTNode *expr;
  ASTType *expr_type;
} ASTExprUnaryPreOp;

typedef struct {
  ASTNodeType type;
  const Token *op;
  ASTNode *expr;
  ASTType *expr_type;
} ASTExprUnaryPostOp;

typedef struct {
  ASTNodeType type;
  const Token *op;
  ASTNode *left;
  ASTNode *right;
  ASTType *expr_type;
} ASTExprBinOp;

typedef struct {
  ASTNodeType type;
  ASTNode *func;
  ASTNode *args;
} ASTExprFuncCall;

typedef struct {
  ASTNodeType type;
  int value;
} ASTInteger;

typedef struct {
  ASTNodeType type;
  const char *str;
} ASTString;

typedef struct {
  ASTNodeType type;
  ASTNode *expr;
} ASTExprStmt;

typedef struct {
  ASTNodeType type;
  ASTKeyword *kw;
  ASTNode *param;
} ASTJumpStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *cond_expr;
  ASTNode *true_expr;
  ASTNode *false_expr;
  ASTType *expr_type;
} ASTCondStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *cond_expr;
  ASTNode *true_stmt;
  ASTNode *false_stmt;
} ASTIfStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *cond_expr;
  ASTNode *body_stmt;
  ASTLabel *begin_label;
  ASTLabel *end_label;
} ASTWhileStmt;

typedef struct {
  ASTNodeType type;
  ASTNode *init_expr;
  ASTNode *cond_expr;
  ASTNode *updt_expr;
  ASTNode *body_stmt;
  ASTLabel *begin_label;
  ASTLabel *end_label;
} ASTForStmt;

typedef struct {
  ASTNodeType type;
  ILOpType op;
  Register *dst;
  Register *left;
  Register *right;
  ASTNode *ast_node;
} ASTILOp;

typedef struct AST_DIRECT_DECLTOR ASTDirectDecltor;
struct AST_DIRECT_DECLTOR {
  ASTNodeType type;
  const Token *bracket_token;
  ASTDirectDecltor *direct_decltor;
  ASTNode *data;
};

typedef struct {
  ASTNodeType type;
  ASTList *decl_specs;
  ASTList *init_decltors;
} ASTDecl;

typedef struct {
  ASTNodeType type;
  ASTList *decl_specs;
  ASTNode *decltor;
} ASTParamDecl;

typedef struct AST_POINTER ASTPointer;
struct AST_POINTER {
  ASTNodeType type;
  ASTPointer *pointer;
};

typedef struct {
  ASTNodeType type;
  ASTPointer *pointer;
  ASTDirectDecltor *direct_decltor;
} ASTDecltor;

typedef struct {
  ASTNodeType type;
  ASTList *decl_specs;
  ASTDecltor *decltor;
  ASTCompStmt *comp_stmt;
  Context *context;
  ASTType *func_type;
} ASTFuncDef;

typedef struct {
  ASTNodeType type;
  ASTList *spec_qual_list;
  ASTList *struct_decltor_list;
} ASTStructDecl;

typedef struct {
  ASTNodeType type;
  const Token *ident;
  ASTList *struct_decl_list;
} ASTStructSpec;

struct AST_VAR {
  ASTNodeType type;
  int ofs;
  const char *name;
  ASTType *var_type;
};

struct AST_LABEL {
  ASTNodeType type;
  int label_number;
};

void Analyze(ASTNode *root);

void InitASTNodeTypeName();
const char *GetASTNodeTypeName(ASTNode *node);

ASTNode *ToASTNode(void *node);
ASTFuncDef *ToASTFuncDef(ASTNode *node);
ASTCompStmt *ToASTCompStmt(ASTNode *node);
ASTExprUnaryPreOp *ToASTExprUnaryPreOp(ASTNode *node);
ASTExprUnaryPostOp *ToASTExprUnaryPostOp(ASTNode *node);
ASTExprBinOp *ToASTExprBinOp(ASTNode *node);
ASTExprFuncCall *ToASTExprFuncCall(ASTNode *node);
ASTInteger *ToASTInteger(ASTNode *node);
ASTString *ToASTString(ASTNode *node);
ASTExprStmt *ToASTExprStmt(ASTNode *node);
ASTJumpStmt *ToASTJumpStmt(ASTNode *node);
ASTCondStmt *ToASTCondStmt(ASTNode *node);
ASTIfStmt *ToASTIfStmt(ASTNode *node);
ASTWhileStmt *ToASTWhileStmt(ASTNode *node);
ASTForStmt *ToASTForStmt(ASTNode *node);
ASTILOp *ToASTILOp(ASTNode *node);
ASTList *ToASTList(ASTNode *node);
ASTKeyword *ToASTKeyword(ASTNode *node);
ASTDecltor *ToASTDecltor(ASTNode *node);
ASTDirectDecltor *ToASTDirectDecltor(ASTNode *node);
ASTIdent *ToASTIdent(ASTNode *node);
ASTDecl *ToASTDecl(ASTNode *node);
ASTParamDecl *ToASTParamDecl(ASTNode *node);
ASTStructDecl *ToASTStructDecl(ASTNode *node);
ASTStructSpec *ToASTStructSpec(ASTNode *node);
ASTPointer *ToASTPointer(ASTNode *node);
ASTDict *ToASTDict(ASTNode *node);
ASTVar *ToASTVar(ASTNode *node);
ASTLabel *ToASTLabel(ASTNode *node);

ASTFuncDef *AllocASTFuncDef(void);
ASTCompStmt *AllocASTCompStmt(void);
ASTExprUnaryPreOp *AllocASTExprUnaryPreOp(void);
ASTExprUnaryPostOp *AllocASTExprUnaryPostOp(void);
ASTExprBinOp *AllocASTExprBinOp(void);
ASTExprFuncCall *AllocASTExprFuncCall(void);
ASTInteger *AllocASTInteger(void);
ASTString *AllocASTString(void);
ASTExprStmt *AllocASTExprStmt(void);
ASTJumpStmt *AllocASTJumpStmt(void);
ASTCondStmt *AllocASTCondStmt(void);
ASTIfStmt *AllocASTIfStmt(void);
ASTWhileStmt *AllocASTWhileStmt(void);
ASTForStmt *AllocASTForStmt(void);
ASTILOp *AllocASTILOp(void);
ASTList *AllocASTList(int capacity);
ASTKeyword *AllocASTKeyword(void);
ASTDecltor *AllocASTDecltor(void);
ASTDirectDecltor *AllocASTDirectDecltor(void);
ASTIdent *AllocASTIdent(void);
ASTDecl *AllocASTDecl(void);
ASTParamDecl *AllocASTParamDecl(void);
ASTStructDecl *AllocASTStructDecl(void);
ASTStructSpec *AllocASTStructSpec(void);
ASTPointer *AllocASTPointer(void);
ASTDict *AllocASTDict(int capacity);
ASTVar *AllocASTVar(void);
ASTLabel *AllocASTLabel(void);

ASTInteger *AllocAndInitASTInteger(int value);
ASTString *AllocAndInitASTString(const char *str);
ASTIdent *AllocAndInitASTIdent(const Token *token);
ASTKeyword *AllocAndInitASTKeyword(const Token *token);
ASTNode *AllocAndInitASTExprBinOp(const Token *op, ASTNode *left,
                                  ASTNode *right);
ASTNode *AllocAndInitASTExprFuncCall(ASTNode *func, ASTNode *args);
ASTVar *AllocAndInitASTVar(ASTList *decl_specs, ASTDecltor *decltor);

const Token *GetIdentTokenFromDecltor(ASTDecltor *decltor);
const Token *GetFuncNameTokenFromFuncDef(ASTFuncDef *func_def);
int IsTypedefDeclSpecs(ASTList *decl_specs);

void PrintASTNode(void *node, int depth);
void DebugPrintASTNode(void *node);

void PushASTNodeToList(ASTList *list, ASTNode *node);
ASTNode *PopASTNodeFromList(ASTList *list);
ASTNode *GetASTNodeAt(const ASTList *list, int index);
void SetASTNodeAt(ASTList *list, int index, ASTNode *node);
int GetSizeOfASTList(const ASTList *list);
ASTNode *GetLastASTNode(const ASTList *list);

void AppendASTNodeToDict(ASTDict *dict, const char *key, ASTNode *node);
ASTNode *FindASTNodeInDict(ASTDict *dict, const char *key);
ASTNode *GetASTNodeInDictAt(const ASTDict *dict, int index);
int GetSizeOfASTDict(const ASTDict *dict);

extern Context *identifiers;
extern Context *struct_names;
void InitGlobalContext();
Context *AllocContext(const Context *parent);
ASTNode *FindInContext(const Context *context, const char *key);
ASTNode *FindIdentInContext(const Context *context, ASTIdent *ident);
int GetSizeOfContext(const Context *context);
int GetAlignOfContext(const Context *context);
ASTVar *AppendLocalVarToContext(Context *context, ASTList *decl_specs,
                                ASTDecltor *decltor);
ASTVar *AppendStructMemberToContext(Context *context, ASTList *decl_specs,
                                    ASTDecltor *decltor);
void AppendTypeToContext(Context *context, const char *name, ASTType *type);
void SetBreakLabelInContext(Context *context, ASTLabel *label);
ASTLabel *GetBreakLabelInContext(Context *context);
void PrintContext(const Context *context);

_Noreturn void Error(const char *fmt, ...);
_Noreturn void ErrorWithASTNode(void *node, const char *fmt, ...);
void Warning(const char *fmt, ...);

void InitILOpTypeName();
const char *GetILOpTypeName(ILOpType type);
void GenerateCode(FILE *fp, ASTList *il, KernelType kernel_type);

ASTList *GenerateIL(ASTNode *root);

ASTNode *Parse(TokenList *tokens);

Token *AllocToken(const char *s, TokenType type);
Token *AllocTokenWithSubstring(const char *begin, const char *end,
                               TokenType type, const char *filename, int line);
int IsEqualToken(const Token *token, const char *s);
int IsTypeToken(const Token *token);
void DebugPrintToken(const Token *token);
void PrintToken(const Token *token);

TokenList *AllocTokenList(int capacity);
void AppendTokenToList(TokenList *list, const Token *token);
const Token *GetTokenAt(const TokenList *list, int index);
int GetSizeOfTokenList(const TokenList *list);
void SetSizeOfTokenList(TokenList *list, int size);
void PrintTokenList(const TokenList *list);

TokenStream *AllocAndInitTokenStream(const TokenList *list);
const Token *PopToken(TokenStream *stream);
void UnpopToken(TokenStream *stream);
int GetStreamPos(TokenStream *stream);
int SeekStream(TokenStream *stream, int pos);
const Token *PeekToken(const TokenStream *stream);
int IsNextToken(TokenStream *stream, const char *str);
int IsNextTokenInList(TokenStream *stream, const char *list[]);
const Token *ConsumeToken(TokenStream *stream, const char *str);
const Token *ExpectToken(TokenStream *stream, const char *str);
void DebugPrintTokenStream(const char *s, const TokenStream *stream);

char *ReadFile(const char *file_name);
void Tokenize(TokenList *tokens, const char *p, const char *filename);

ASTType *ToASTType(ASTNode *node);
ASTType *AllocASTType(void);
ASTType *AllocAndInitBasicType(BasicType basic_type);
ASTType *AllocAndInitASTTypePointerOf(ASTType *pointer_of);
ASTType *AllocAndInitASTTypeLValueOf(ASTType *lvalue_of);
ASTType *AllocAndInitASTTypeArrayOf(ASTType *array_of, int num_of_elements);
ASTType *AllocAndInitASTType(ASTList *decl_specs, ASTDecltor *decltor);
int IsEqualASTType(ASTType *a, ASTType *b);
int IsBasicType(ASTType *node, BasicType type);
int IsTypePointer(ASTType *node);
int IsTypeStructLValue(ASTType *type);
ASTType *GetRValueTypeOf(ASTType *node);
ASTType *GetDereferencedTypeOf(ASTType *node);
ASTType *ConvertFromArrayToPointer(ASTType *node);
int GetSizeOfType(ASTType *node);
int GetAlignOfType(ASTType *node);
const char *GetStructTagFromType(ASTType *type);
Context *GetStructContextFromType(ASTType *type);
ASTType *GetExprTypeOfASTNode(ASTNode *node);
void PrintASTType(ASTType *node);
void DebugPrintASTType(ASTType *type);

struct AST_TYPE {
  ASTNodeType type;
  BasicType basic_type;
  ASTType *pointer_of;
  ASTType *lvalue_of;
  ASTType *array_of;
  int num_of_elements;
  const Token *struct_ident;
  Context *struct_members;
  ASTType *func_return_type;
};

ASTType *ToASTType(ASTNode *node) {
  if (!node || node->type != kASTType) return 0;
  return (ASTType *)node;
};
ASTType *AllocASTType(void) {
  ASTType *node = (ASTType *)calloc(1, sizeof(ASTType));
  node->type = kASTType;
  return node;
};

ASTType *AllocAndInitBasicType(BasicType basic_type) {
  ASTType *node = AllocASTType();
  node->basic_type = basic_type;
  return node;
}

ASTType *AllocAndInitASTTypePointerOf(ASTType *pointer_of) {
  ASTType *node = AllocASTType();
  node->basic_type = kTypePointerOf;
  node->pointer_of = pointer_of;
  return node;
}

ASTType *AllocAndInitASTTypeLValueOf(ASTType *lvalue_of) {
  ASTType *node = AllocASTType();
  node->basic_type = kTypeLValueOf;
  node->lvalue_of = lvalue_of;
  return node;
}

ASTType *AllocAndInitASTTypeArrayOf(ASTType *array_of, int num_of_elements) {
  ASTType *node = AllocASTType();
  node->basic_type = kTypeArrayOf;
  node->array_of = array_of;
  node->num_of_elements = num_of_elements;
  return node;
}

ASTType *AllocAndInitASTTypeStruct(const Token *struct_ident,
                                   Context *struct_members) {
  ASTType *node = AllocASTType();
  node->basic_type = kTypeStruct;
  node->struct_ident = struct_ident;
  node->struct_members = struct_members;
  return node;
}

ASTType *AllocAndInitASTTypeFunction(ASTType *func_return_type) {
  ASTType *node = AllocASTType();
  node->basic_type = kTypeFunction;
  node->func_return_type = func_return_type;
  return node;
}

ASTType *AllocAndInitASTType(ASTList *decl_specs, ASTDecltor *decltor) {
  ASTType *type = 0;
  for (int t = 0; t < GetSizeOfASTList(decl_specs); t++) {
    ASTNode *type_node = GetASTNodeAt(decl_specs, t);
    if (type_node->type == kASTKeyword) {
      ASTKeyword *kw = ToASTKeyword(type_node);
      if (IsEqualToken(kw->token, "const") ||
          IsEqualToken(kw->token, "typedef") ||
          IsEqualToken(kw->token, "unsigned")) {
        continue;
      }
      assert(!type);
      BasicType basic_type = kTypeNone;
      if (IsEqualToken(kw->token, "int") || IsEqualToken(kw->token, "long")) {
        basic_type = kTypeInt;
      } else if (IsEqualToken(kw->token, "char")) {
        basic_type = kTypeChar;
      } else if (IsEqualToken(kw->token, "void")) {
        basic_type = kTypeChar;
      }
      if (basic_type == kTypeNone) {
        Error("Type %s is not implemented", kw->token->str);
      }
      type = AllocAndInitBasicType(basic_type);
      continue;
    } else if (type_node->type == kASTStructSpec) {
      assert(!type);
      ASTStructSpec *spec = ToASTStructSpec(type_node);
      Context *context = 0;
      if (spec->struct_decl_list) {
        context = AllocContext(0);
        for (int i = 0; i < GetSizeOfASTList(spec->struct_decl_list); i++) {
          ASTStructDecl *decl =
              ToASTStructDecl(GetASTNodeAt(spec->struct_decl_list, i));
          assert(decl);
          for (int k = 0; k < GetSizeOfASTList(decl->struct_decltor_list);
               k++) {
            ASTDecltor *decltor =
                ToASTDecltor(GetASTNodeAt(decl->struct_decltor_list, k));
            assert(decltor);
            AppendStructMemberToContext(context, decl->spec_qual_list, decltor);
          }
        }
      }
      type = AllocAndInitASTTypeStruct(spec->ident, context);
      if (struct_names && spec->ident) {
        ASTType *resolved_type =
            ToASTType(FindInContext(struct_names, spec->ident->str));
        if (resolved_type) type = resolved_type;
      }
      continue;
    } else if (type_node->type == kASTType) {
      type = ToASTType(type_node);
      const char *typedef_name = GetStructTagFromType(type);
      ASTType *resolved_type =
          ToASTType(FindInContext(struct_names, typedef_name));
      if (resolved_type) type = resolved_type;
      continue;
    }
    ErrorWithASTNode(type_node, "not implemented type of decl_specs[0]");
  }
  if (!decltor) return type;
  for (ASTPointer *ptr = decltor->pointer; ptr; ptr = ptr->pointer) {
    type = AllocAndInitASTTypePointerOf(type);
  }
  for (ASTDirectDecltor *d = decltor->direct_decltor; d;
       d = d->direct_decltor) {
    if (IsEqualToken(d->bracket_token, "[")) {
      ASTInteger *integer = ToASTInteger(d->data);
      if (!integer) Error("Array size should be an integer");
      type = AllocAndInitASTTypeArrayOf(type, integer->value);
    } else if (IsEqualToken(d->bracket_token, "(")) {
      type = AllocAndInitASTTypeFunction(type);
    }
  }

  return type;
}

int IsEqualASTType(ASTType *a, ASTType *b) {
  a = GetRValueTypeOf(a);
  b = GetRValueTypeOf(b);
  while (1) {
    if (!a || !b) return 0;
    if (a->basic_type != b->basic_type) return 0;
    if (a->basic_type != kTypePointerOf) break;
    a = a->pointer_of;
    b = b->pointer_of;
  }
  return 1;
}

int IsBasicType(ASTType *node, BasicType type) {
  return node && node->basic_type == type;
}

int IsTypePointer(ASTType *node) {
  ASTType *rtype = GetRValueTypeOf(node);
  return IsBasicType(rtype, kTypePointerOf) || IsBasicType(rtype, kTypeArrayOf);
}

int IsTypeStructLValue(ASTType *type) {
  return IsBasicType(type, kTypeLValueOf) &&
         IsBasicType(type->lvalue_of, kTypeStruct);
}

ASTType *GetRValueTypeOf(ASTType *node) {
  if (!node) return 0;
  if (node->basic_type == kTypeLValueOf) {
    return node->lvalue_of;
  }
  return node;
}

ASTType *GetDereferencedTypeOf(ASTType *node) {
  node = GetRValueTypeOf(node);
  assert((node->basic_type == kTypePointerOf));
  return AllocAndInitASTTypeLValueOf(node->pointer_of);
}

ASTType *ConvertFromArrayToPointer(ASTType *node) {
  assert(node->basic_type == kTypeArrayOf);
  return AllocAndInitASTTypePointerOf(node->array_of);
}

int GetSizeOfType(ASTType *node) {
  node = GetRValueTypeOf(node);
  if (node->basic_type == kTypePointerOf) {
    return 8;
  } else if (node->basic_type == kTypeArrayOf) {
    return node->num_of_elements * GetSizeOfType(node->array_of);
  } else if (node->basic_type == kTypeStruct) {
    if (!node->struct_members) {
      DebugPrintASTType(node);
      Error("Cannot take size of incomplete type");
    }
    return GetSizeOfContext(node->struct_members);
  } else if (node->basic_type == kTypeChar) {
    return 1;
  } else if (node->basic_type == kTypeInt) {
    return 4;
  }
  Error("GetSizeOfType: Not implemented for basic_type %d", node->basic_type);
  return -1;
}

int GetAlignOfType(ASTType *node) {
  node = GetRValueTypeOf(node);
  if (node->basic_type == kTypePointerOf) {
    return 8;
  } else if (node->basic_type == kTypeArrayOf) {
    return GetAlignOfType(node->array_of);
  } else if (node->basic_type == kTypeStruct) {
    if (!node->struct_members) {
      DebugPrintASTType(node);
      Error("Cannot take size of incomplete type");
    }
    return GetAlignOfContext(node->struct_members);
  } else if (node->basic_type == kTypeChar) {
    return 1;
  } else if (node->basic_type == kTypeInt) {
    return 4;
  }
  Error("GetAlignOfType: Not implemented for basic_type %d", node->basic_type);
  return -1;
}

const char *GetStructTagFromType(ASTType *type) {
  type = GetRValueTypeOf(type);
  assert(type->basic_type == kTypeStruct);
  return type->struct_ident ? type->struct_ident->str : "(anonymous)";
}

Context *GetStructContextFromType(ASTType *type) {
  type = GetRValueTypeOf(type);
  assert(type->basic_type == kTypeStruct);
  return type->struct_members;
}

ASTType *GetExprTypeOfASTNode(ASTNode *node) {
  assert(node);
  if (node->type == kASTIdent) {
    return ToASTIdent(node)->var_type;
  } else if (node->type == kASTInteger) {
    return AllocAndInitBasicType(kTypeInt);
  } else if (node->type == kASTExprBinOp) {
    return ToASTExprBinOp(node)->expr_type;
  } else if (node->type == kASTExprUnaryPreOp) {
    return ToASTExprUnaryPreOp(node)->expr_type;
  } else if (node->type == kASTExprUnaryPostOp) {
    return ToASTExprUnaryPostOp(node)->expr_type;
  } else if (node->type == kASTString) {
    return AllocAndInitASTTypePointerOf(AllocAndInitBasicType(kTypeChar));
  } else if (node->type == kASTExprFuncCall) {
    return AllocAndInitBasicType(kTypeInt);
  } else if (node->type == kASTCondStmt) {
    return ToASTCondStmt(node)->expr_type;
  } else if (node->type == kASTVar) {
    return ToASTVar(node)->var_type;
  }
  PrintASTNode(node, 0);
  Error("GetExprTypeOfASTNode is not implemented for this AST type");
  return 0;
}

void PrintASTType(ASTType *node) {
  if (!node) {
    printf("(null)");
  } else if (node->basic_type == kTypeLValueOf) {
    printf("lvalue_of ");
    PrintASTType(node->lvalue_of);
  } else if (node->basic_type == kTypePointerOf) {
    PrintASTType(node->pointer_of);
    putchar('*');
  } else if (node->basic_type == kTypeArrayOf) {
    printf("array[%d] of ", node->num_of_elements);
    PrintASTType(node->array_of);
  } else if (node->basic_type == kTypeStruct) {
    printf("struct %s", node->struct_ident->str);
    if (!node->struct_members) printf("(incomplete)");
  } else if (node->basic_type == kTypeFunction) {
    printf("function returns ");
    PrintASTType(node->func_return_type);
  } else if (node->basic_type == kTypeChar) {
    printf("char");
  } else if (node->basic_type == kTypeInt) {
    printf("int");
  } else {
    Error("PrintASTType: Not implemented for basic_type %d", node->basic_type);
  }
}

void DebugPrintASTType(ASTType *type) {
  PrintASTType(type);
  putchar('\n');
}
