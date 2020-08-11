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

static int CalcStructAlignFromDict(struct Node *dict) {
  assert(dict && dict->type == kASTList);
  int align = 1;
  for (int i = 0; i < GetSizeOfList(dict); i++) {
    struct Node *kv = GetNodeAt(dict, i);
    struct Node *member_info = kv->value;
    assert(member_info->struct_member_ent_type);
    int member_align = GetAlignOfType(member_info->struct_member_ent_type);
    if (align < member_align) align = member_align;
  }
  return align;
}

int CalcStructAlign(struct Node *spec) {
  assert(spec && spec->type == kASTStructSpec);
  return CalcStructAlignFromDict(spec->struct_member_dict);
}

void AddMemberOfStructFromDecl(struct Node *struct_spec, struct Node *decl) {
  struct Node *struct_member = AllocNode(kNodeStructMember);
  struct_member->struct_member_decl = decl;
  struct Node *type = CreateTypeFromDecl(decl);
  assert(type && type->left);
  const char *name = CreateTokenStr(type->left);
  struct Node *dict = struct_spec->struct_member_dict;
  PushKeyValueToList(dict, name, struct_member);
}

struct Node *FindStructMember(struct Node *struct_type,
                              struct Node *key_token) {
  assert(key_token->type == kNodeToken);
  struct_type = GetTypeWithoutAttr(struct_type);
  assert(struct_type && struct_type->type == kTypeStruct);
  assert(struct_type->type_struct_spec);
  assert(struct_type->type_struct_spec->struct_member_dict);
  assert(struct_type->type_struct_spec->struct_member_dict->type == kASTList);
  return GetNodeByTokenKey(struct_type->type_struct_spec->struct_member_dict,
                           key_token);
}

void ResolveTypesOfMembersOfStruct(struct SymbolEntry *ctx, struct Node *spec) {
  if (!spec) {
    // Skip resolving members since it is incomplete.
    return;
  }
  struct Node *dict = spec->struct_member_dict;
  fprintf(stderr, "Resolving types of struct...\n");
  struct Node *resolved_dict = AllocList();
  for (int i = 0; i < GetSizeOfList(dict); i++) {
    struct Node *kv = GetNodeAt(dict, i);
    struct Node *member_info = kv->value;
    struct Node *type =
        CreateTypeFromDeclInContext(ctx, member_info->struct_member_decl);
    assert(type && type->left);
    member_info->struct_member_ent_type = GetTypeWithoutAttr(type);
    member_info->struct_member_ent_ofs =
        CalcNextMemberOffset(resolved_dict, type);
    PrintASTNode(member_info);
    PushKeyValueToList(resolved_dict, kv->key, kv->value);
  }
  spec->struct_member_dict = resolved_dict;
}
