#include "compilium.h"

void AddMemberOfStructFromDecl(struct Node *struct_spec, struct Node *decl) {
  struct Node *struct_member = AllocNode(kNodeStructMember);
  struct Node *type = CreateTypeFromDecl(decl);
  assert(type && type->left);
  const char *name = CreateTokenStr(type->left);
  struct_member->struct_member_ent_type = GetTypeWithoutAttr(type);
  PushKeyValueToList(struct_spec->struct_member_dict, name, struct_member);
}
