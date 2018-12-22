#include "compilium.h"

struct ASTNode *ParseCastExpr();
struct ASTNode *ParseExpr(void);

struct ASTNode *ParsePrimaryExpr() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenDecimalNumber)) ||
      (t = ConsumeToken(kTokenOctalNumber)) ||
      (t = ConsumeToken(kTokenIdent)) ||
      (t = ConsumeToken(kTokenCharLiteral)) ||
      (t = ConsumeToken(kTokenStringLiteral))) {
    struct ASTNode *op = AllocASTNode(kASTTypeExpr);
    op->op = t;
    return op;
  }
  if ((t = ConsumePunctuator("("))) {
    struct ASTNode *op = AllocASTNode(kASTTypeExpr);
    op->op = t;
    op->right = ParseExpr();
    if (!op->right) ErrorWithToken(t, "Expected expr after this token");
    ExpectPunctuator(")");
    return op;
  }
  return NULL;
}

struct ASTNode *ParseUnaryExpr() {
  struct Token *t;
  if ((t = ConsumePunctuator("+")) || (t = ConsumePunctuator("-")) ||
      (t = ConsumePunctuator("~")) || (t = ConsumePunctuator("!")) ||
      (t = ConsumePunctuator("&")) || (t = ConsumePunctuator("*"))) {
    return AllocAndInitASTNodeUnaryPrefixOp(t, ParseCastExpr());
  } else if ((t = ConsumeToken(kTokenKwSizeof))) {
    return AllocAndInitASTNodeUnaryPrefixOp(t, ParseUnaryExpr());
  }
  return ParsePrimaryExpr();
}

struct ASTNode *ParseCastExpr() {
  return ParseUnaryExpr();
}

struct ASTNode *ParseMulExpr() {
  struct ASTNode *op = ParseCastExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("*")) || (t = ConsumePunctuator("/")) ||
         (t = ConsumePunctuator("%"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseCastExpr());
  }
  return op;
}

struct ASTNode *ParseAddExpr() {
  struct ASTNode *op = ParseMulExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("+")) || (t = ConsumePunctuator("-"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseMulExpr());
  }
  return op;
}

struct ASTNode *ParseShiftExpr() {
  struct ASTNode *op = ParseAddExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("<<")) || (t = ConsumePunctuator(">>"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseAddExpr());
  }
  return op;
}

struct ASTNode *ParseRelExpr() {
  struct ASTNode *op = ParseShiftExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("<")) || (t = ConsumePunctuator(">")) ||
         (t = ConsumePunctuator("<=")) || (t = ConsumePunctuator(">="))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseShiftExpr());
  }
  return op;
}

struct ASTNode *ParseEqExpr() {
  struct ASTNode *op = ParseRelExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("==")) || (t = ConsumePunctuator("!="))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseRelExpr());
  }
  return op;
}

struct ASTNode *ParseAndExpr() {
  struct ASTNode *op = ParseEqExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("&"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseEqExpr());
  }
  return op;
}

struct ASTNode *ParseXorExpr() {
  struct ASTNode *op = ParseAndExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("^"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseAndExpr());
  }
  return op;
}

struct ASTNode *ParseOrExpr() {
  struct ASTNode *op = ParseXorExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("|"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseXorExpr());
  }
  return op;
}

struct ASTNode *ParseBoolAndExpr() {
  struct ASTNode *op = ParseOrExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("&&"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseOrExpr());
  }
  return op;
}

struct ASTNode *ParseBoolOrExpr() {
  struct ASTNode *op = ParseBoolAndExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("||"))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseBoolAndExpr());
  }
  return op;
}

struct ASTNode *ParseConditionalExpr() {
  struct ASTNode *expr = ParseBoolOrExpr();
  if (!expr) return NULL;
  struct Token *t;
  if ((t = ConsumePunctuator("?"))) {
    struct ASTNode *op = AllocASTNode(kASTTypeExpr);
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

struct ASTNode *ParseAssignExpr() {
  struct ASTNode *left = ParseConditionalExpr();
  if (!left) return NULL;
  struct Token *t;
  if ((t = ConsumePunctuator("="))) {
    struct ASTNode *right = ParseAssignExpr();
    if (!right) ErrorWithToken(t, "Expected expr after this token");
    return AllocAndInitASTNodeBinOp(t, left, right);
  }
  return left;
}

struct ASTNode *ParseExpr() {
  struct ASTNode *op = ParseAssignExpr();
  if (!op) return NULL;
  struct Token *t;
  while ((t = ConsumePunctuator(","))) {
    op = AllocAndInitASTNodeBinOp(t, op, ParseAssignExpr());
  }
  return op;
}

struct ASTNode *ParseExprStmt() {
  struct ASTNode *expr = ParseExpr();
  struct Token *t;
  if ((t = ConsumePunctuator(";"))) {
    return AllocAndInitASTNodeExprStmt(t, expr);
  } else if (expr) {
    ExpectPunctuator(";");
  }
  return NULL;
}

struct ASTNode *ParseJumpStmt() {
  struct Token *t;
  if ((t = ConsumeToken(kTokenKwReturn))) {
    struct ASTNode *expr = ParseExpr();
    ExpectPunctuator(";");
    struct ASTNode *stmt = AllocASTNode(kASTTypeJumpStmt);
    stmt->op = t;
    stmt->right = expr;
    return stmt;
  }
  return NULL;
}

struct ASTNode *ParseStmt() {
  struct ASTNode *stmt;
  if ((stmt = ParseExprStmt()) || (stmt = ParseJumpStmt())) return stmt;
  return NULL;
}

struct Token *ParseDeclSpecs() {
  struct Token *decl_spec;
  (decl_spec = ConsumeToken(kTokenKwInt)) ||
      (decl_spec = ConsumeToken(kTokenKwChar));
  return decl_spec;
}

struct ASTNode *ParseParamDecl();
struct ASTNode *ParseDirectDecltor() {
  struct ASTNode *n = AllocASTNode(kASTTypeDirectDecltor);
  n->op = ExpectToken(kTokenIdent);
  while (true) {
    struct Token *t;
    if ((t = ConsumePunctuator("("))) {
      struct ASTNode *arg = ParseParamDecl();
      ExpectPunctuator(")");
      struct ASTNode *nn = AllocASTNode(kASTTypeDirectDecltor);
      nn->op = t;
      nn->right = arg;
      nn->left = n;
      n = nn;
    }
    break;
  }
  return n;
}

struct ASTNode *ParseDecltor() {
  struct ASTNode *n = AllocASTNode(kASTTypeDecltor);
  struct ASTNode *pointer = NULL;
  struct Token *t;
  while ((t = ConsumePunctuator("*"))) {
    pointer = AllocAndInitPointerOf(pointer);
  }
  n->left = pointer;
  n->right = ParseDirectDecltor();
  return n;
}

struct ASTNode *ParseParamDecl() {
  struct Token *decl_spec = ParseDeclSpecs();
  if (!decl_spec) return NULL;
  struct ASTNode *n = AllocASTNode(kASTTypeDecl);
  n->op = decl_spec;
  n->right = ParseDecltor();
  return n;
}

struct ASTNode *ParseDecl() {
  struct Token *decl_spec = ParseDeclSpecs();
  if (!decl_spec) return NULL;
  struct ASTNode *n = AllocASTNode(kASTTypeDecl);
  n->op = decl_spec;
  n->right = ParseDecltor();
  ExpectPunctuator(";");
  return n;
}

struct ASTNode *ParseCompStmt() {
  struct Token *t;
  if (!(t = ConsumePunctuator("{"))) return NULL;
  struct ASTNode *list = AllocList();
  list->op = t;
  struct ASTNode *stmt;
  while ((stmt = ParseDecl()) || (stmt = ParseStmt())) {
    PushToList(list, stmt);
  }
  ExpectPunctuator("}");
  return list;
}

struct ASTNode *toplevel_names;

struct ASTNode *Parse() {
  struct ASTNode *list = AllocList();
  struct ASTNode *n;
  toplevel_names = AllocList();
  while ((n = ParseCompStmt()) || (n = ParseDecl())) {
    if (n->type == kASTTypeDecl) {
      PrintASTNode(n);
      putc('\n', stderr);
      // PushKeyValueToList(toplevel_names, )
      continue;
    }
    PushToList(list, n);
  }
  struct Token *t;
  if (!(t = NextToken())) return list;
  ErrorWithToken(t, "Unexpected token");
}
