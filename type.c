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
ASTType *AllocAndInitASTTypeStruct(const Token *struct_ident,
                                   Context *struct_members);

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
