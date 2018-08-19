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

static ASTType *ParseStructSpec(ASTStructSpec *struct_spec) {
  assert(struct_spec);
  ASTType *struct_type = NULL;
  Context *struct_context = NULL;
  if (struct_spec->struct_decl_list) {
    struct_context = AllocContext(NULL);
    for (int i = 0; i < GetSizeOfASTList(struct_spec->struct_decl_list); i++) {
      ASTStructDecl *decl =
          ToASTStructDecl(GetASTNodeAt(struct_spec->struct_decl_list, i));
      assert(decl);
      for (int k = 0; k < GetSizeOfASTList(decl->struct_decltor_list); k++) {
        ASTDecltor *member_decltor =
            ToASTDecltor(GetASTNodeAt(decl->struct_decltor_list, k));
        assert(member_decltor);
        AppendStructMemberToContext(struct_context, decl->spec_qual_list,
                                    member_decltor);
      }
    }
  }
  struct_type = AllocAndInitASTTypeStruct(struct_spec->ident, struct_context);
  if (struct_names && struct_spec->ident) {
    ASTType *resolved_type =
        ToASTType(FindInContext(struct_names, struct_spec->ident->str));
    if (resolved_type) struct_type = resolved_type;
  }
  return struct_type;
}

ASTType *AllocAndInitASTType(ASTList *decl_specs, ASTDecltor *decltor) {
  ASTType *type = NULL;
  for (int t = 0; t < GetSizeOfASTList(decl_specs); t++) {
    ASTNode *type_node = GetASTNodeAt(decl_specs, t);
    if (type_node->type == kASTKeyword) {
      ASTKeyword *kw = ToASTKeyword(type_node);
      if (IsEqualToken(kw->token, "const") ||
          IsEqualToken(kw->token, "typedef") ||
          IsEqualToken(kw->token, "unsigned") ||
          IsEqualToken(kw->token, "_Noreturn") ||
          IsEqualToken(kw->token, "extern") ||
          IsEqualToken(kw->token, "static")) {
        continue;
      }
      assert(!type);
      BasicType basic_type = kTypeNone;
      if (IsEqualToken(kw->token, "int") || IsEqualToken(kw->token, "long")) {
        basic_type = kTypeInt;
      } else if (IsEqualToken(kw->token, "char")) {
        basic_type = kTypeChar;
      } else if (IsEqualToken(kw->token, "void")) {
        basic_type = kTypeVoid;
      }
      if (basic_type == kTypeNone) {
        Error("Type %s is not implemented", kw->token->str);
      }
      type = AllocAndInitBasicType(basic_type);
      continue;
    } else if (type_node->type == kASTStructSpec) {
      type = ParseStructSpec(ToASTStructSpec(type_node));
      continue;
    } else if (type_node->type == kASTType) {
      type = ToASTType(type_node);
      if (IsBasicType(type, kTypeStruct)) {
        ASTType *resolved_type =
            ToASTType(FindInContext(struct_names, GetStructTagFromType(type)));
        if (resolved_type) type = resolved_type;
      }
      continue;
    } else if (type_node->type == kASTEnumSpec) {
      type = AllocAndInitBasicType(kTypeInt);
      continue;
    }
    ErrorWithASTNode(type_node, "not implemented type of decl_specs");
  }
  if (!decltor) return type;
  for (ASTPointer *ptr = decltor->pointer; ptr; ptr = ptr->pointer) {
    type = AllocAndInitASTTypePointerOf(type);
  }
  for (ASTDirectDecltor *d = decltor->direct_decltor; d;
       d = d->direct_decltor) {
    if (IsEqualToken(d->bracket_token, "[")) {
      type = AllocAndInitASTTypeArrayOf(type, EvalConstantExpression(d->data));
    } else if (IsEqualToken(d->bracket_token, "(")) {
      type = AllocAndInitASTTypeFunction(type);
      // TODO: Add types of args
    } else if (!d->bracket_token && d->data && d->data->type == kASTIdent) {
      type->ident = ToASTIdent(d->data)->token;
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
