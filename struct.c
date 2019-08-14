#include "compilium.h"

static int CalcStructSizeFromDict(struct Node *dict) {
  if (!GetSizeOfList(dict)) return 0;
  struct Node *last_kv = GetNodeAt(dict, GetSizeOfList(dict) - 1);
  struct Node *last_member = last_kv->value;
  return last_member->struct_member_ent_ofs +
         GetSizeOfType(last_member->struct_member_ent_type);
}

static int CalcNextMemberOffset(struct Node *dict, struct Node *type) {
  if (!GetSizeOfList(dict)) return 0;
  int align = GetAlignOfType(type);
  return (CalcStructSizeFromDict(dict) + align - 1) / align * align;
}

int CalcStructSize(struct Node *spec) {
  assert(spec && spec->type == kASTStructSpec);
  return CalcStructSizeFromDict(spec->struct_member_dict);
}

void AddMemberOfStructFromDecl(struct Node *struct_spec, struct Node *decl) {
  struct Node *struct_member = AllocNode(kNodeStructMember);
  struct Node *type = CreateTypeFromDecl(decl);
  assert(type && type->left);
  const char *name = CreateTokenStr(type->left);
  struct_member->struct_member_ent_type = GetTypeWithoutAttr(type);
  struct Node *dict = struct_spec->struct_member_dict;
  PrintASTNode(type);
  struct_member->struct_member_ent_ofs = CalcNextMemberOffset(dict, type);
  PushKeyValueToList(dict, name, struct_member);
  PrintASTNode(struct_member);
}
