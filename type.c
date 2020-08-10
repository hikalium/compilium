#include "compilium.h"

int IsSameTypeExceptAttr(struct Node *a, struct Node *b) {
  assert(a && b);
  a = GetTypeWithoutAttr(a);
  b = GetTypeWithoutAttr(b);
  if (a->type != b->type) return 0;
  if (a->type == kTypeBase) {
    assert(a->op && b->op);
    return a->op->type == b->op->type;
  } else if (a->type == kTypePointer) {
    return IsSameTypeExceptAttr(a->right, b->right);
  } else if (a->type == kTypeFunction) {
    if (!IsSameTypeExceptAttr(a->left, b->left)) return 0;
    if (GetSizeOfList(a->right) != GetSizeOfList(b->right)) return 0;
    for (int i = 0; i < GetSizeOfList(a->right); i++) {
      if (!IsSameTypeExceptAttr(GetNodeAt(a->right, i), GetNodeAt(b->right, i)))
        return 0;
    }
    return 1;
  }
  Error("IsSameTypeExceptAttr: Comparing non-type nodes");
}

struct Node *GetTypeWithoutAttr(struct Node *t) {
  if (!t) return NULL;
  if (t->type != kTypeLValue && t->type != kTypeAttrIdent) return t;
  return GetTypeWithoutAttr(t->right);
}

struct Node *GetIdentifierTokenFromTypeAttr(struct Node *t) {
  if (!t || t->type != kTypeAttrIdent) return NULL;
  return t->left;
}

struct Node *GetRValueType(struct Node *t) {
  if (!t) return NULL;
  if (t->type != kTypeLValue) return t;
  return t->right;
}

int IsLValueType(struct Node *t) { return t && t->type == kTypeLValue; }

int IsAssignable(struct Node *dst, struct Node *src) {
  assert(dst && src);
  if (dst->type != kTypeLValue) return 0;
  return IsSameTypeExceptAttr(GetRValueType(dst), src);
}

int EvalExprAsInt(struct Node *n) {
  assert(n);
  if (IsTokenWithType(n->op, kTokenIntegerConstant)) {
    return strtol(n->op->begin, NULL, 0);
  }
  if (n->type == kASTExpr && IsEqualTokenWithCStr(n->op, "+")) {
    return EvalExprAsInt(n->left) + EvalExprAsInt(n->right);
  }
  assert(false);
}

int GetSizeOfType(struct Node *t) {
  t = GetTypeWithoutAttr(t);
  assert(t);
  if (t->type == kTypeBase) {
    assert(IsToken(t->op));
    switch (t->op->token_type) {
      case kTokenKwInt:
      case kTokenKwLong:
        return 4;
      case kTokenKwChar:
        return 1;
      case kTokenKwVoid:
        return 0;
      default:
        PrintASTNode(t->op);
        assert(false);
    }
  } else if (t->type == kTypePointer) {
    return 8;
  } else if (t->type == kTypeStruct) {
    if (!t->type_struct_spec) {
      ErrorWithToken(t->tag, "Cannot take sizeof incomplete struct");
    }
    return CalcStructSize(t->type_struct_spec);
  } else if (t->type == kTypeArray) {
    return GetSizeOfType(t->type_array_type_of) *
           EvalExprAsInt(t->type_array_index_decl);
  }
  PrintASTNode(t);
  assert(false);
}

int GetAlignOfType(struct Node *t) {
  t = GetTypeWithoutAttr(t);
  assert(t);
  if (t->type == kTypeBase) {
    assert(IsToken(t->op));
    switch (t->op->token_type) {
      case kTokenKwInt:
        return 4;
      case kTokenKwChar:
        return 1;
      default:
        assert(false);
    }
  } else if (t->type == kTypePointer) {
    return 8;
  } else if (t->type == kTypeStruct) {
    return CalcStructAlign(t->type_struct_spec);
  }
  PrintASTNode(t);
  assert(false);
}

struct Node *CreateTypeFromDecl(struct Node *decl);
struct Node *CreateType(struct Node *decl_spec, struct Node *decltor);
struct Node *CreateTypeFromDecltor(struct Node *decltor, struct Node *type) {
  assert(decltor && decltor->type == kASTDecltor);
  struct Node *pointer = decltor->left;
  if (pointer) {
    struct Node *p = pointer;
    while (p->right) {
      p = p->right;
    }
    p->right = type;
    type = pointer;
  }
  for (struct Node *dd = decltor->right; dd; dd = dd->left) {
    assert(dd->type == kASTDirectDecltor);
    if (dd->left) {
      assert(dd->op);
      if (IsEqualTokenWithCStr(dd->op, "(")) {
        // direct-declarator ( parameter-type-list | identifier-list_opt )
        struct Node *arg_type_list = AllocList();
        for (int i = 0; i < GetSizeOfList(dd->right); i++) {
          struct Node *arg = GetNodeAt(dd->right, i);
          if (IsEqualTokenWithCStr(arg, "...")) {
            if (i != GetSizeOfList(dd->right) - 1) {
              ErrorWithToken(arg,
                             "va arg is only allowed at the end of params.");
            }
            PushToList(arg_type_list, arg);
            break;
          }
          PushToList(arg_type_list, CreateTypeFromDecl(arg));
        }
        type = CreateTypeFunction(type, arg_type_list);
        continue;
      }
      if (IsEqualTokenWithCStr(dd->op, "[")) {
        // direct-declarator [ list ]
        type = CreateTypeArray(type, dd->right);
        continue;
      }
      assert(false);
    }
    assert(!dd->left);
    if (IsEqualTokenWithCStr(dd->op, "(")) {
      assert(dd->value && dd->value->type == kASTDecltor);
      type = CreateTypeFromDecltor(dd->value, type);
      continue;
    }
    assert(IsTokenWithType(dd->op, kTokenIdent));
    type = CreateTypeAttrIdent(dd->op, type);
  }
  return type;
}

static struct Node *CreateBaseTypeFromDeclSpecs(struct SymbolEntry *ctx,
                                                struct Node *decl_specs) {
  // 6.2.5 Types
  assert(IsASTList(decl_specs));
  struct Node *type_qual = NULL;
  struct Node *type_spec = NULL;
  for (int i = 0; i < GetSizeOfList(decl_specs); i++) {
    struct Node *t = GetNodeAt(decl_specs, i);
    if (IsTokenWithType(t, kTokenKwTypedef) ||
        IsTokenWithType(t, kTokenKwUnsigned) ||
        IsTokenWithType(t, kTokenKwExtern)) {
      continue;
    }
    if (IsTokenWithType(t, kTokenKwConst)) {
      assert(!type_qual);
      type_qual = t;
      continue;
    }
    assert(!type_spec);
    type_spec = t;
  }
  assert(type_spec);
  if (IsToken(type_spec)) {
    if (!IsTokenWithType(type_spec, kTokenKwInt) &&
        !IsTokenWithType(type_spec, kTokenKwChar) &&
        !IsTokenWithType(type_spec, kTokenKwLong) &&
        !IsEqualTokenWithCStr(type_spec, "__builtin_va_list") &&
        !IsTokenWithType(type_spec, kTokenKwVoid)) {
      ErrorWithToken(type_spec, "Unexpected token for base type specifier");
    }
    return CreateTypeBase(type_spec);
  }
  if (type_spec->type == kASTStructSpec) {
    if (!type_spec->struct_member_dict) {
      assert(type_spec->tag);
      struct Node *resolved_type = FindStructType(ctx, type_spec->tag);
      if (resolved_type) return resolved_type;
      return CreateTypeStruct(type_spec->tag, NULL);
    }
    return CreateTypeStruct(type_spec->tag, type_spec);
  }
  if (type_spec->type == kTypeStruct || type_spec->type == kTypeBase) {
    // typedef_type
    return type_spec;
  }
  PrintASTNode(decl_specs);
  assert(false);
}

struct Node *CreateTypeInContext(struct SymbolEntry *ctx,
                                 struct Node *decl_specs,
                                 struct Node *decltor) {
  struct Node *type = CreateBaseTypeFromDeclSpecs(ctx, decl_specs);
  if (!decltor) return type;
  return CreateTypeFromDecltor(decltor, type);
}

struct Node *CreateType(struct Node *decl_spec, struct Node *decltor) {
  return CreateTypeInContext(NULL, decl_spec, decltor);
}

struct Node *CreateTypeFromDecl(struct Node *decl) {
  assert(decl && decl->type == kASTDecl);
  return CreateType(decl->op, decl->right);
}

struct Node *CreateTypeFromDeclInContext(struct SymbolEntry *ctx,
                                         struct Node *decl) {
  assert(decl && decl->type == kASTDecl);
  return CreateTypeInContext(ctx, decl->op, decl->right);
}

struct Node *Tokenize(const char *input);
struct Node *ParseDecl(void);
static struct Node *CreateTypeFromInput(const char *s) {
  fprintf(stderr, "CreateTypeFromInput: %s\n", s);
  struct Node *tokens = Tokenize(s);
  InitParser(&tokens);
  return CreateTypeFromDecl(ParseDecl());
}
_Noreturn void TestType() {
  fprintf(stderr, "Testing Type...\n");

  struct Node *int_type = CreateTypeBase(CreateToken("int"));
  struct Node *another_int_type = CreateTypeBase(CreateToken("int"));
  struct Node *lvalue_int_type = CreateTypeLValue(int_type);
  struct Node *pointer_of_int_type = CreateTypePointer(int_type);
  struct Node *another_pointer_of_int_type =
      CreateTypePointer(another_int_type);

  assert(IsSameTypeExceptAttr(int_type, int_type));
  assert(IsSameTypeExceptAttr(int_type, another_int_type));
  assert(IsSameTypeExceptAttr(int_type, lvalue_int_type));
  assert(IsSameTypeExceptAttr(lvalue_int_type, lvalue_int_type));
  assert(!IsSameTypeExceptAttr(int_type, pointer_of_int_type));
  assert(
      IsSameTypeExceptAttr(pointer_of_int_type, another_pointer_of_int_type));
  assert(GetSizeOfType(int_type) == 4);
  assert(GetSizeOfType(pointer_of_int_type) == 8);

  struct Node *char_type = CreateTypeBase(CreateToken("char"));
  assert(GetSizeOfType(char_type) == 1);

  struct Node *long_type = CreateTypeBase(CreateToken("long"));
  assert(GetSizeOfType(long_type) == 4);

  struct Node *ppi_type = CreateTypePointer(pointer_of_int_type);

  struct Node *args_i = AllocList();
  PushToList(args_i, int_type);
  struct Node *if_i_type = CreateTypeFunction(int_type, args_i);
  struct Node *ppif_i_type = CreateTypeFunction(ppi_type, args_i);

  struct Node *args_pi = AllocList();
  PushToList(args_pi, pointer_of_int_type);
  struct Node *if_pi_type = CreateTypeFunction(int_type, args_pi);
  struct Node *ppif_pi_type = CreateTypeFunction(ppi_type, args_pi);

  struct Node *type;

  type = CreateTypeFromInput("void* (*f)(int size);");
  PrintASTNode(type);
  type = GetTypeWithoutAttr(type);
  assert(type->type == kTypePointer);
  type = type->right;
  assert(type->type == kTypeFunction);
  type = GetReturnTypeOfFunction(type);
  assert(type->type == kTypePointer);
  assert(GetSizeOfType(type) == 8);

  type = CreateTypeFromInput("int v;");
  PrintASTNode(type);
  assert(IsSameTypeExceptAttr(type, int_type));

  type = CreateTypeFromInput("int *p;");
  PrintASTNode(type);
  assert(IsSameTypeExceptAttr(type, pointer_of_int_type));

  type = CreateTypeFromInput("int **p;");
  PrintASTNode(type);
  assert(IsSameTypeExceptAttr(type, ppi_type));

  type = CreateTypeFromInput("int f(int a);");
  PrintASTNode(type);
  assert(IsSameTypeExceptAttr(type, if_i_type));
  assert(!IsSameTypeExceptAttr(type, if_pi_type));

  type = CreateTypeFromInput("int f(int *a);");
  PrintASTNode(type);
  assert(!IsSameTypeExceptAttr(type, if_i_type));
  assert(IsSameTypeExceptAttr(type, if_pi_type));

  type = CreateTypeFromInput("int **f(int a);");
  PrintASTNode(type);
  assert(IsSameTypeExceptAttr(type, ppif_i_type));
  assert(!IsSameTypeExceptAttr(type, ppif_pi_type));
  PrintASTNode(if_i_type);
  assert(!IsSameTypeExceptAttr(type, if_i_type));
  assert(!IsSameTypeExceptAttr(type, if_pi_type));

  type =
      CreateTypeFromInput("void (*signal(int sig, void (*func)(int)))(int);");
  assert(type && GetTypeWithoutAttr(type) &&
         GetTypeWithoutAttr(type)->type == kTypeFunction);
  PrintASTNode(type);

  type = CreateTypeFromInput("struct IncompleteStruct;");
  assert(type);
  PrintASTNode(type);

  fprintf(stderr, "PASS\n");
  exit(EXIT_SUCCESS);
}
