#include "compilium.h"

// 6.2.3 Name spaces of identifiers
static struct Node *ord_idents;  // ordinary identifiers

struct Node *ParseStmt();
struct Node *ParseCompStmt();
struct Node *ParseDeclBody();
struct Node *ParseCastExpr();
struct Node *ParseExpr(void);

struct Node *ParsePrimaryExpr() {
  struct Node *t;
  if ((t = ConsumeToken(kTokenIntegerConstant)) ||
      (t = ConsumeToken(kTokenIdent)) ||
      (t = ConsumeToken(kTokenCharLiteral)) ||
      (t = ConsumeToken(kTokenStringLiteral))) {
    struct Node *op = AllocNode(kASTExpr);
    op->op = t;
    return op;
  }
  if ((t = ConsumePunctuator("("))) {
    struct Node *op = AllocNode(kASTExpr);
    op->op = t;
    op->right = ParseExpr();
    if (!op->right) ErrorWithToken(t, "Expected expr after this token");
    ExpectPunctuator(")");
    return op;
  }
  return NULL;
}

struct Node *ParseAssignExpr();
struct Node *ParsePostfixExpr() {
  struct Node *n = ParsePrimaryExpr();
  while (n) {
    struct Node *t;
    if (ConsumePunctuator("(")) {
      struct Node *args = AllocList();
      if (!ConsumePunctuator(")")) {
        do {
          struct Node *arg_expr = ParseAssignExpr();
          if (!arg_expr)
            ErrorWithToken(NextToken(), "Expected expression here");
          PushToList(args, arg_expr);
        } while (ConsumePunctuator(","));
        ExpectPunctuator(")");
      }
      struct Node *nn = AllocNode(kASTExprFuncCall);
      nn->func_expr = n;
      nn->arg_expr_list = args;
      n = nn;
      continue;
    }
    if ((t = ConsumePunctuator("["))) {
      n = CreateASTBinOp(t, n, ParseExpr());
      ExpectPunctuator("]");
      continue;
    }
    if ((t = ConsumePunctuator(".")) || (t = ConsumePunctuator("->"))) {
      struct Node *right = ConsumeToken(kTokenIdent);
      assert(right);
      n = CreateASTBinOp(t, n, right);
      continue;
    }
    if ((t = ConsumePunctuator("++"))) {
      n = CreateASTUnaryPostfixOp(n, t);
      continue;
    }
    if ((t = ConsumePunctuator("--"))) {
      n = CreateASTUnaryPostfixOp(n, t);
      continue;
    }
    break;
  }
  return n;
}

struct Node *ParseUnaryExpr() {
  struct Node *t;
  if ((t = ConsumePunctuator("+")) || (t = ConsumePunctuator("-")) ||
      (t = ConsumePunctuator("~")) || (t = ConsumePunctuator("!")) ||
      (t = ConsumePunctuator("&")) || (t = ConsumePunctuator("*"))) {
    return CreateASTUnaryPrefixOp(t, ParseCastExpr());
  } else if ((t = ConsumePunctuator("--")) || (t = ConsumePunctuator("++")) ||
             (t = ConsumeToken(kTokenKwSizeof))) {
    return CreateASTUnaryPrefixOp(t, ParseUnaryExpr());
  }
  return ParsePostfixExpr();
}

struct Node *ParseCastExpr() {
  return ParseUnaryExpr();
}

struct Node *ParseMulExpr() {
  struct Node *op = ParseCastExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("*")) || (t = ConsumePunctuator("/")) ||
         (t = ConsumePunctuator("%"))) {
    op = CreateASTBinOp(t, op, ParseCastExpr());
  }
  return op;
}

struct Node *ParseAddExpr() {
  struct Node *op = ParseMulExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("+")) || (t = ConsumePunctuator("-"))) {
    op = CreateASTBinOp(t, op, ParseMulExpr());
  }
  return op;
}

struct Node *ParseShiftExpr() {
  struct Node *op = ParseAddExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("<<")) || (t = ConsumePunctuator(">>"))) {
    op = CreateASTBinOp(t, op, ParseAddExpr());
  }
  return op;
}

struct Node *ParseRelExpr() {
  struct Node *op = ParseShiftExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("<")) || (t = ConsumePunctuator(">")) ||
         (t = ConsumePunctuator("<=")) || (t = ConsumePunctuator(">="))) {
    op = CreateASTBinOp(t, op, ParseShiftExpr());
  }
  return op;
}

struct Node *ParseEqExpr() {
  struct Node *op = ParseRelExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("==")) || (t = ConsumePunctuator("!="))) {
    op = CreateASTBinOp(t, op, ParseRelExpr());
  }
  return op;
}

struct Node *ParseAndExpr() {
  struct Node *op = ParseEqExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("&"))) {
    op = CreateASTBinOp(t, op, ParseEqExpr());
  }
  return op;
}

struct Node *ParseXorExpr() {
  struct Node *op = ParseAndExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("^"))) {
    op = CreateASTBinOp(t, op, ParseAndExpr());
  }
  return op;
}

struct Node *ParseOrExpr() {
  struct Node *op = ParseXorExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("|"))) {
    op = CreateASTBinOp(t, op, ParseXorExpr());
  }
  return op;
}

struct Node *ParseBoolAndExpr() {
  struct Node *op = ParseOrExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("&&"))) {
    op = CreateASTBinOp(t, op, ParseOrExpr());
  }
  return op;
}

struct Node *ParseBoolOrExpr() {
  struct Node *op = ParseBoolAndExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("||"))) {
    op = CreateASTBinOp(t, op, ParseBoolAndExpr());
  }
  return op;
}

struct Node *ParseConditionalExpr() {
  struct Node *expr = ParseBoolOrExpr();
  if (!expr) return NULL;
  struct Node *t;
  if ((t = ConsumePunctuator("?"))) {
    struct Node *op = AllocNode(kASTExpr);
    op->op = t;
    op->cond = expr;
    op->left = ParseConditionalExpr();
    if (!op->left)
      ErrorWithToken(t, "Expected true-expr for this conditional expr");
    ExpectPunctuator(":");
    op->right = ParseConditionalExpr();
    if (!op->right)
      ErrorWithToken(t, "Expected false-expr for this conditional expr");
    return op;
  }
  return expr;
}

struct Node *ParseAssignExpr() {
  struct Node *left = ParseConditionalExpr();
  if (!left) return NULL;
  struct Node *t;
  if ((t = ConsumePunctuator("=")) || (t = ConsumePunctuator("+=")) ||
      (t = ConsumePunctuator("-=")) || (t = ConsumePunctuator("*=")) ||
      (t = ConsumePunctuator("/=")) || (t = ConsumePunctuator("%=")) ||
      (t = ConsumePunctuator("<<=")) || (t = ConsumePunctuator(">>="))) {
    struct Node *right = ParseAssignExpr();
    if (!right) ErrorWithToken(t, "Expected expr after this token");
    return CreateASTBinOp(t, left, right);
  }
  return left;
}

struct Node *ParseExpr() {
  struct Node *op = ParseAssignExpr();
  if (!op) return NULL;
  struct Node *t;
  while ((t = ConsumePunctuator(","))) {
    op = CreateASTBinOp(t, op, ParseAssignExpr());
  }
  return op;
}

struct Node *ParseExprStmt() {
  struct Node *expr = ParseExpr();
  struct Node *t;
  if ((t = ConsumePunctuator(";"))) {
    return CreateASTExprStmt(t, expr);
  } else if (expr) {
    ExpectPunctuator(";");
  }
  return NULL;
}

struct Node *ParseSelectionStmt() {
  struct Node *t;
  if ((t = ConsumeToken(kTokenKwIf))) {
    ExpectPunctuator("(");
    struct Node *expr = ParseExpr();
    assert(expr);
    ExpectPunctuator(")");
    struct Node *stmt_true = ParseStmt();
    assert(stmt_true);
    struct Node *stmt = AllocNode(kASTSelectionStmt);
    stmt->op = t;
    stmt->cond = expr;
    stmt->if_true_stmt = stmt_true;
    if (ConsumeToken(kTokenKwElse)) {
      stmt->if_else_stmt = ParseStmt();
    }
    return stmt;
  }
  return NULL;
}

struct Node *ParseJumpStmt() {
  // jump-statement
  struct Node *t;
  if ((t = ConsumeToken(kTokenKwBreak)) ||
      (t = ConsumeToken(kTokenKwContinue))) {
    ExpectPunctuator(";");
    struct Node *stmt = AllocNode(kASTJumpStmt);
    stmt->op = t;
    return stmt;
  }
  if ((t = ConsumeToken(kTokenKwReturn))) {
    struct Node *expr = ParseExpr();
    ExpectPunctuator(";");
    struct Node *stmt = AllocNode(kASTJumpStmt);
    stmt->op = t;
    stmt->right = expr;
    return stmt;
  }
  return NULL;
}

struct Node *ParseIterationStmt() {
  struct Node *t;
  if ((t = ConsumeToken(kTokenKwFor))) {
    ExpectPunctuator("(");
    struct Node *init = ParseDeclBody();
    if (!init) init = ParseExpr();
    ExpectPunctuator(";");
    struct Node *cond = ParseExpr();
    ExpectPunctuator(";");
    struct Node *updt = ParseExpr();
    ExpectPunctuator(")");
    struct Node *body = ParseStmt();
    assert(body);

    struct Node *stmt = AllocNode(kASTForStmt);
    stmt->op = t;
    stmt->init = init;
    stmt->cond = cond;
    stmt->updt = updt;
    stmt->body = body;
    return stmt;
  }
  if ((t = ConsumeToken(kTokenKwWhile))) {
    ExpectPunctuator("(");
    struct Node *cond = ParseExpr();
    assert(cond);
    ExpectPunctuator(")");
    struct Node *body = ParseStmt();
    assert(body);

    struct Node *stmt = AllocNode(kASTWhileStmt);
    stmt->op = t;
    stmt->cond = cond;
    stmt->body = body;
    return stmt;
  }
  return NULL;
}

struct Node *ParseStmt() {
  struct Node *stmt;
  if ((stmt = ParseExprStmt()) || (stmt = ParseJumpStmt()) ||
      (stmt = ParseSelectionStmt()) || (stmt = ParseCompStmt()) ||
      (stmt = ParseIterationStmt()))
    return stmt;
  return NULL;
}

struct Node *ParseDecl();
struct Node *ParseDeclSpecs() {
  // returns Node<kASTList> or NULL
  struct Node *decl_specs = AllocList();
  for (;;) {
    struct Node *decl_spec;
    // storage-class-specifier
    if ((decl_spec = ConsumeToken(kTokenKwTypedef)) ||
        (decl_spec = ConsumeToken(kTokenKwExtern))) {
      PushToList(decl_specs, decl_spec);
      continue;
    }
    if ((decl_spec = ConsumeToken(kTokenKwStatic))) {
      continue;
    }
    // type-qualifier
    if ((decl_spec = ConsumeToken(kTokenKwConst))) {
      PushToList(decl_specs, decl_spec);
      continue;
    }
    if ((decl_spec = ConsumeToken(kTokenKwVoid)) ||
        (decl_spec = ConsumeToken(kTokenKwChar)) ||
        (decl_spec = ConsumeToken(kTokenKwInt)) ||
        (decl_spec = ConsumeToken(kTokenKwLong)) ||
        (decl_spec = ConsumeToken(kTokenKwUnsigned))) {
      PushToList(decl_specs, decl_spec);
      continue;
    }
    // builtin type name
    if ((decl_spec = ConsumeTokenStr("__builtin_va_list"))) {
      PushToList(decl_specs, decl_spec);
      continue;
    }
    // typedef name
    struct Node *typedef_type = GetNodeByTokenKey(ord_idents, PeekToken());
    if (typedef_type) {
      PushToList(decl_specs, typedef_type);
      NextToken();
      continue;
    }
    // struct-or-union-specifier
    if (ConsumeToken(kTokenKwStruct)) {
      struct Node *struct_spec = AllocNode(kASTStructSpec);
      struct_spec->tag = ConsumeToken(kTokenIdent);
      assert(struct_spec->tag);
      if (ConsumePunctuator("{")) {
        struct_spec->struct_member_dict = AllocList();
        struct Node *decl;
        while ((decl = ParseDecl())) {
          AddMemberOfStructFromDecl(struct_spec, decl);
        }
        ExpectPunctuator("}");
      }
      PushToList(decl_specs, struct_spec);
      continue;
    }
    break;
  };
  if (GetSizeOfList(decl_specs) == 0) {
    return NULL;
  }
  return decl_specs;
}

struct Node *ParseParamDecl();
struct Node *ParseDecltor();
struct Node *ParseDirectDecltor() {
  // always allow abstract decltors
  struct Node *n = NULL;
  struct Node *t;
  if ((t = ConsumePunctuator("("))) {
    n = AllocNode(kASTDirectDecltor);
    n->op = t;
    n->value = ParseDecltor();
    assert(n->value);
    ExpectPunctuator(")");
  } else if ((t = ConsumeToken(kTokenIdent))) {
    n = AllocNode(kASTDirectDecltor);
    n->op = t;
  }
  while (true) {
    if ((t = ConsumePunctuator("("))) {
      struct Node *op = t;
      struct Node *args = AllocList();
      if (!ConsumePunctuator(")")) {
        while (1) {
          if ((t = ConsumePunctuator("..."))) {
            PushToList(args, t);
          } else {
            struct Node *arg = ParseParamDecl();
            if (!arg) {
              ErrorWithToken(NextToken(), "Expected ParamDecl here");
            }
            PushToList(args, arg);
          }
          if (!ConsumePunctuator(",")) break;
        }
        ExpectPunctuator(")");
      }
      struct Node *nn = AllocNode(kASTDirectDecltor);
      nn->op = op;
      nn->right = args;
      nn->left = n;
      n = nn;
    }
    if ((t = ConsumePunctuator("["))) {
      struct Node *nn = AllocNode(kASTDirectDecltor);
      nn->op = t;
      nn->right = ParseAssignExpr();
      nn->left = n;
      n = nn;
      ExpectPunctuator("]");
      continue;
    }
    break;
  }
  return n;
}

struct Node *ParseDecltor() {
  struct Node *n = AllocNode(kASTDecltor);
  struct Node *pointer = NULL;
  struct Node *t;
  while ((t = ConsumePunctuator("*"))) {
    pointer = CreateTypePointer(pointer);
  }
  n->left = pointer;
  n->right = ParseDirectDecltor();
  return n;
}

struct Node *ParseInitDecltor() {
  struct Node *decltor = ParseDecltor();
  if (!decltor) return NULL;
  struct Node *t;
  if (!(t = ConsumePunctuator("="))) return decltor;
  struct Node *init_expr = ParseAssignExpr();
  assert(init_expr);
  decltor->decltor_init_expr = CreateASTBinOp(t, NULL, init_expr);
  return decltor;
}

struct Node *ParseParamDecl() {
  struct Node *decl_specs = ParseDeclSpecs();
  if (!decl_specs) return NULL;
  struct Node *n = AllocNode(kASTDecl);
  n->op = decl_specs;
  n->right = ParseDecltor();
  return n;
}

struct Node *ParseDeclBody() {
  struct Node *decl_specs = ParseDeclSpecs();
  if (!decl_specs) return NULL;
  struct Node *n = AllocNode(kASTDecl);
  n->op = decl_specs;
  n->right = ParseInitDecltor();
  return n;
}

struct Node *ParseDecl() {
  struct Node *decl_body = ParseDeclBody();
  if (!decl_body) return NULL;
  ExpectPunctuator(";");
  return decl_body;
}

struct Node *ParseCompStmt() {
  struct Node *t;
  if (!(t = ConsumePunctuator("{"))) return NULL;
  struct Node *list = AllocList();
  list->op = t;
  struct Node *stmt;
  while ((stmt = ParseDecl()) || (stmt = ParseStmt())) {
    PushToList(list, stmt);
  }
  ExpectPunctuator("}");
  return list;
}

struct Node *ParseFuncDef(struct Node *decl_body) {
  struct Node *comp_stmt = ParseCompStmt();
  if (!comp_stmt) return NULL;
  return CreateASTFuncDef(decl_body, comp_stmt);
}

void InitParser(struct Node **head_token) {
  InitTokenStream(RemoveDelimiterTokens(head_token));
  ord_idents = AllocList();
}

struct Node *Parse(struct Node **head_token) {
  InitParser(head_token);
  struct Node *list = AllocList();
  struct Node *decl_body;
  while ((decl_body = ParseDeclBody())) {
    if (ConsumePunctuator(";")) {
      PushToList(list, decl_body);
      assert(IsASTList(decl_body->op));
      if (IsASTDeclOfTypedef(decl_body)) {
        // typedef case
        struct Node *typedef_type = CreateTypeFromDecl(decl_body);
        struct Node *typedef_name =
            GetIdentifierTokenFromTypeAttr(typedef_type);
        PrintASTNode(typedef_name);
        PushKeyValueToList(ord_idents, CreateTokenStr(typedef_name),
                           GetTypeWithoutAttr(typedef_type));
      }
      continue;
    }
    struct Node *func_def = ParseFuncDef(decl_body);
    if (!func_def) {
      ErrorWithToken(NextToken(), "Unexpected token");
    }
    PushToList(list, func_def);
  }
  struct Node *t;
  if (!(t = NextToken())) return list;
  ErrorWithToken(t, "Unexpected token");
}
