#include "compilium.h"

struct SymbolEntry *AllocSymbolEntry(struct SymbolEntry *prev, const char *key,
                                     struct Node *value) {
  struct SymbolEntry *e = calloc(1, sizeof(struct SymbolEntry));
  e->prev = prev;
  e->key = key;
  e->value = value;
  return e;
}

int GetLastLocalVarOffset(struct SymbolEntry *last) {
  if (!last) return 0;
  assert(last->value && last->value->type == kASTLocalVar);
  return last->value->byte_offset;
}

struct Node *AddLocalVar(struct SymbolEntry **ctx, const char *key,
                         struct Node *var_type) {
  assert(ctx);
  int ofs = GetLastLocalVarOffset(*ctx);
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct Node *local_var = CreateASTLocalVar(ofs, var_type);
  struct SymbolEntry *e = AllocSymbolEntry(*ctx, key, local_var);
  *ctx = e;
  return local_var;
}

struct Node *FindLocalVar(struct SymbolEntry *e, struct Node *key_token) {
  while (e) {
    if (IsEqualTokenWithCStr(key_token, e->key)) {
      return e->value;
    }
    e = e->prev;
  }
  return NULL;
}
