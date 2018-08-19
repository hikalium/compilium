#include "compilium.h"

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
  const Token *ident;
};

ASTType *ToASTType(ASTNode *node) {
  if (!node || node->type != kASTType) return 0;
  return (ASTType *)node;
}

ASTType *AllocASTType(void) {
  ASTType *node = (ASTType *)calloc(1, sizeof(ASTType));
  node->type = kASTType;
  return node;
}

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
  if (!node) return NULL;
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
      ASTType *complete_type =
          ToASTType(FindInContext(struct_names, node->struct_ident->str));
      if (complete_type) node->struct_members = complete_type->struct_members;
    }
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
  ErrorWithASTNode(node, "GetAlignOfType: Not implemented for basic_type %d",
                   node->basic_type);
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
  if (!type->struct_members) {
    ASTType *resolved_type =
        ToASTType(FindInContext(struct_names, type->struct_ident->str));
    if (resolved_type) type->struct_members = resolved_type->struct_members;
  }
  return type->struct_members;
}

ASTType *GetReturningTypeFromFunctionType(ASTType *type) {
  type = GetRValueTypeOf(type);
  assert(type->basic_type == kTypeFunction);
  return type->func_return_type;
}

const Token *GetIdentTokenOfType(ASTType *type) {
  type = GetRValueTypeOf(type);
  return type->ident;
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
    return ToASTExprFuncCall(node)->func_type->func_return_type;
  } else if (node->type == kASTCondStmt) {
    return ToASTCondStmt(node)->expr_type;
  } else if (node->type == kASTVar) {
    return ToASTVar(node)->var_type;
  } else if (node->type == kASTExprCast) {
    return ToASTExprCast(node)->to_expr_type;
  }
  PrintASTNode(node, 0);
  Error("GetExprTypeOfASTNode is not implemented for this AST type");
  return NULL;
}

void PrintASTType(ASTType *node) {
  if (!node) {
    printf("(null)");
  } else if (node->basic_type == kTypeLValueOf) {
    printf("lvalue_of(");
    PrintASTType(node->lvalue_of);
    putchar(')');
  } else if (node->basic_type == kTypePointerOf) {
    PrintASTType(node->pointer_of);
    putchar('*');
  } else if (node->basic_type == kTypeArrayOf) {
    printf("array[%d] of ", node->num_of_elements);
    PrintASTType(node->array_of);
  } else if (node->basic_type == kTypeStruct) {
    printf("struct %s",
           node->struct_ident ? node->struct_ident->str : "(Anonymous)");
    if (!node->struct_members) printf("(incomplete)");
  } else if (node->basic_type == kTypeFunction) {
    printf("function returns ");
    PrintASTType(node->func_return_type);
  } else if (node->basic_type == kTypeChar) {
    printf("char");
  } else if (node->basic_type == kTypeInt) {
    printf("int");
  } else if (node->basic_type == kTypeVoid) {
    printf("void");
  } else {
    Error("PrintASTType: Not implemented for basic_type %d", node->basic_type);
  }
}

void DebugPrintASTType(ASTType *type) {
  PrintASTType(type);
  putchar('\n');
}
