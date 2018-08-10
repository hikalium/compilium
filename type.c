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

ASTType *AllocAndInitASTType(ASTList *decl_specs, ASTDecltor *decltor) {
  if (GetSizeOfASTList(decl_specs) != 1) {
    Error("decl_specs contains 2 tokens or more is not supported");
  }
  ASTKeyword *kw = ToASTKeyword(GetASTNodeAt(decl_specs, 0));
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
  ASTType *node = AllocAndInitBasicType(basic_type);
  for (ASTPointer *ptr = decltor->pointer; ptr; ptr = ptr->pointer) {
    node = AllocAndInitASTTypePointerOf(node);
  }
  for (ASTDirectDecltor *d = decltor->direct_decltor; d;
       d = d->direct_decltor) {
    if (IsEqualToken(d->bracket_token, "[")) {
      ASTInteger *integer = ToASTInteger(d->data);
      if (!integer) Error("Array size should be an integer");
      node = AllocAndInitASTTypeArrayOf(node, integer->value);
    }
  }

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

ASTType *GetRValueTypeOf(ASTType *node) {
  if (!node) return NULL;
  if (node->basic_type == kTypeLValueOf) {
    return node->lvalue_of;
  }
  return node;
}

ASTType *GetDereferencedTypeOf(ASTType *node) {
  node = GetRValueTypeOf(node);
  if (node->basic_type == kTypePointerOf) {
    return node->pointer_of;
  }
  Error("GetDereferencedTypeOf: Not implemented for basic_type %d",
        node->basic_type);
  return NULL;
}

int GetSizeOfType(ASTType *node) {
  if (node->basic_type == kTypePointerOf) {
    return 8;
  } else if (node->basic_type == kTypeArrayOf) {
    return node->num_of_elements * GetSizeOfType(node->array_of);
  } else if (node->basic_type == kTypeChar) {
    return 1;
  } else if (node->basic_type == kTypeInt) {
    // TODO: Change this from 8 to 4
    return 8;
  }
  Error("GetSizeOfType: Not implemented for basic_type %d", node->basic_type);
  return -1;
}

int GetSizeOfTypeForASTNode(ASTNode *node) {
  if (!node) {
    Error("GetSizeOfTypeForASTNode: Can not take size of NULL AST");
  }
  if (node->type == kASTIdent) {
    return GetSizeOfType(ToASTIdent(node)->local_var->var_type);
  }
  PrintASTNode(node, 0);
  Error("GetSizeOfTypeForASTNode is not implemented for this AST type");
  return -1;
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
  } else if (node->basic_type == kTypeChar) {
    printf("char");
  } else if (node->basic_type == kTypeInt) {
    printf("int");
  } else {
    Error("PrintASTType: Not implemented for basic_type %d", node->basic_type);
  }
}
