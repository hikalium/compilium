#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
};

GenToAST(Type);
GenAllocAST(Type);

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

ASTType *AllocAndInitASTType(ASTList *decl_specs, ASTDecltor *decltor,
                             Context *struct_names) {
  ASTType *type = NULL;
  for (int t = 0; t < GetSizeOfASTList(decl_specs); t++) {
    ASTNode *type_node = GetASTNodeAt(decl_specs, t);
    if (type_node->type == kASTKeyword) {
      ASTKeyword *kw = ToASTKeyword(type_node);
      if (IsEqualToken(kw->token, "const")) {
        continue;
      }
      assert(!type);
      BasicType basic_type = kTypeNone;
      if (IsEqualToken(kw->token, "int")) {
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
      Context *context = NULL;
      if (spec->struct_decl_list) {
        context = AllocContext(NULL);
        for (int i = 0; i < GetSizeOfASTList(spec->struct_decl_list); i++) {
          ASTStructDecl *decl =
              ToASTStructDecl(GetASTNodeAt(spec->struct_decl_list, i));
          assert(decl);
          for (int k = 0; k < GetSizeOfASTList(decl->struct_decltor_list);
               k++) {
            ASTDecltor *decltor =
                ToASTDecltor(GetASTNodeAt(decl->struct_decltor_list, k));
            assert(decltor);
            AppendStructMemberToContext(context, decl->spec_qual_list, decltor,
                                        struct_names);
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
      // TODO: Add types of args
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
  return NULL;
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
