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

int IsAssignable(struct Node *dst, struct Node *src) {
  assert(dst && src);
  if (dst->type != kTypeLValue) return 0;
  return IsSameTypeExceptAttr(GetRValueType(dst), src);
}

int GetSizeOfType(struct Node *t) {
  t = GetRValueType(t);
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
  }
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
      if (IsEqualTokenWithCStr(dd->op, "(")) {
        struct Node *arg_type_list = AllocList();
        for (int i = 0; i < GetSizeOfList(dd->right); i++) {
          PushToList(arg_type_list,
                     CreateTypeFromDecl(GetNodeAt(dd->right, i)));
        }
        type = CreateTypeFunction(type, arg_type_list);
        continue;
      }
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

struct Node *CreateBaseTypeFromDeclSpec(struct Node *decl_spec) {
  assert(decl_spec);
  if (IsToken(decl_spec)) return CreateTypeBase(decl_spec);
  if (decl_spec->type == kASTStructSpec)
    return CreateTypeStruct(decl_spec->tag);
  assert(false);
}

struct Node *CreateType(struct Node *decl_spec, struct Node *decltor) {
  struct Node *type = CreateBaseTypeFromDeclSpec(decl_spec);
  if (!decltor) return type;
  return CreateTypeFromDecltor(decltor, type);
}

struct Node *CreateTypeFromDecl(struct Node *decl) {
  assert(decl && decl->type == kASTDecl);
  return CreateType(decl->op, decl->right);
}

struct Node *Tokenize(const char *input);
struct Node *ParseDecl(void);
static struct Node *CreateTypeFromInput(const char *s) {
  fprintf(stderr, "CreateTypeFromInput: %s\n", s);
  InitParser(Tokenize(s));
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
