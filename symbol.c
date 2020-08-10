#include "compilium.h"

static void PushSymbol(struct SymbolEntry **prev, struct SymbolEntry *sym) {
  sym->prev = *prev;
  *prev = sym;
}

static struct SymbolEntry *AllocSymbolEntry(enum SymbolType type,
                                            const char *key,
                                            struct Node *value) {
  struct SymbolEntry *e = calloc(1, sizeof(struct SymbolEntry));
  e->type = type;
  e->key = key;
  e->value = value;
  return e;
}

int GetLastLocalVarOffset(struct SymbolEntry *e) {
  for (; e; e = e->prev) {
    if (e->type != kSymbolLocalVar) continue;
    assert(e->value && e->value->type == kASTLocalVar);
    return e->value->byte_offset;
  }
  return 0;
}

struct Node *AddLocalVar(struct SymbolEntry **ctx, const char *key,
                         struct Node *var_type) {
  assert(ctx);
  int ofs = GetLastLocalVarOffset(*ctx);
  ofs += GetSizeOfType(var_type);
  int align = GetSizeOfType(var_type);
  ofs = (ofs + align - 1) / align * align;
  struct Node *local_var = CreateASTLocalVar(ofs, var_type);
  struct SymbolEntry *e = AllocSymbolEntry(kSymbolLocalVar, key, local_var);
  PushSymbol(ctx, e);
  return local_var;
}

void AddGlobalVar(struct SymbolEntry **ctx, const char *key,
                  struct Node *var_type) {
  fprintf(stderr, "Gvar: %s: ", key);
  PrintASTNode(var_type);
  fprintf(stderr, "\n");
  assert(ctx);
  struct SymbolEntry *e = AllocSymbolEntry(kSymbolGlobalVar, key, var_type);
  PushSymbol(ctx, e);
}
void AddExternVar(struct SymbolEntry **ctx, const char *key,
                  struct Node *var_type) {
  fprintf(stderr, "Evar: %s: ", key);
  PrintASTNode(var_type);
  fprintf(stderr, "\n");
  assert(ctx);
  struct SymbolEntry *e = AllocSymbolEntry(kSymbolExternVar, key, var_type);
  PushSymbol(ctx, e);
}

struct Node *FindExternVar(struct SymbolEntry *e, struct Node *key_token) {
  // returns ASTNode which represents Type
  for (; e; e = e->prev) {
    if (e->type != kSymbolExternVar) continue;
    if (!IsEqualTokenWithCStr(key_token, e->key)) continue;
    return e->value;
  }
  return NULL;
}

struct Node *FindGlobalVar(struct SymbolEntry *e, struct Node *key_token) {
  // returns ASTNode which represents Type
  for (; e; e = e->prev) {
    if (e->type != kSymbolGlobalVar) continue;
    if (!IsEqualTokenWithCStr(key_token, e->key)) continue;
    return e->value;
  }
  return NULL;
}

struct Node *FindLocalVar(struct SymbolEntry *e, struct Node *key_token) {
  for (; e; e = e->prev) {
    if (e->type != kSymbolLocalVar) continue;
    if (!IsEqualTokenWithCStr(key_token, e->key)) continue;
    return e->value;
  }
  return NULL;
}

void AddFuncDef(struct SymbolEntry **ctx, const char *key,
                struct Node *func_def) {
  assert(ctx);
  struct SymbolEntry *e = AllocSymbolEntry(kSymbolFuncDef, key, func_def);
  PushSymbol(ctx, e);
}

struct Node *FindFuncDef(struct SymbolEntry *e, struct Node *key_token) {
  for (; e; e = e->prev) {
    if (e->type != kSymbolFuncDef) continue;
    if (!IsEqualTokenWithCStr(key_token, e->key)) continue;
    return e->value;
  }
  return NULL;
}

void AddFuncDeclType(struct SymbolEntry **ctx, const char *key,
                     struct Node *func_decl) {
  assert(ctx);
  struct SymbolEntry *e = AllocSymbolEntry(kSymbolFuncDeclType, key, func_decl);
  PushSymbol(ctx, e);
}

struct Node *FindFuncDeclType(struct SymbolEntry *e, struct Node *key_token) {
  for (; e; e = e->prev) {
    if (e->type != kSymbolFuncDeclType) continue;
    if (!IsEqualTokenWithCStr(key_token, e->key)) continue;
    return e->value;
  }
  return NULL;
}

void AddStructType(struct SymbolEntry **ctx, const char *key,
                   struct Node *type) {
  assert(ctx);
  struct SymbolEntry *e = AllocSymbolEntry(kSymbolStructType, key, type);
  PushSymbol(ctx, e);
  PrintASTNode(type);
}

struct Node *FindStructType(struct SymbolEntry *e, struct Node *key_token) {
  for (; e; e = e->prev) {
    if (e->type != kSymbolStructType) continue;
    if (!IsEqualTokenWithCStr(key_token, e->key)) continue;
    return e->value;
  }
  return NULL;
}
