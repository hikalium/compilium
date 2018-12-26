#include "compilium.h"

struct Node *tokens;
int token_stream_index;

static struct Node *ConsumeToken(enum NodeType type) {
  if (token_stream_index >= GetSizeOfList(tokens)) return NULL;
  struct Node *t = GetNodeAt(tokens, token_stream_index);
  if (t->type != type) return NULL;
  token_stream_index++;
  return t;
}

static struct Node *ExpectToken(enum NodeType type) {
  if (token_stream_index >= GetSizeOfList(tokens))
    Error("Expect token type %d but got EOF", type);
  struct Node *t = GetNodeAt(tokens, token_stream_index);
  if (t->type != type) ErrorWithToken(t, "Expected token type %d here", type);
  token_stream_index++;
  return t;
}

static struct Node *ConsumePunctuator(const char *s) {
  if (token_stream_index >= GetSizeOfList(tokens)) return NULL;
  struct Node *t = GetNodeAt(tokens, token_stream_index);
  if (!IsEqualTokenWithCStr(t, s)) return NULL;
  token_stream_index++;
  return t;
}

static struct Node *ExpectPunctuator(const char *s) {
  if (token_stream_index >= GetSizeOfList(tokens))
    Error("Expect token %s but got EOF", s);
  struct Node *t = GetNodeAt(tokens, token_stream_index);
  if (!IsEqualTokenWithCStr(t, s))
    ErrorWithToken(t, "Expected token %s here", s);
  token_stream_index++;
  return t;
}

static struct Node *NextToken() {
  if (token_stream_index >= GetSizeOfList(tokens)) return NULL;
  return GetNodeAt(tokens, token_stream_index++);
}

struct Node *ParseCastExpr();
struct Node *ParseExpr(void);

struct Node *ParsePrimaryExpr() {
  struct Node *t;
  if ((t = ConsumeToken(kTokenDecimalNumber)) ||
      (t = ConsumeToken(kTokenOctalNumber)) ||
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

struct Node *ParseUnaryExpr() {
  struct Node *t;
  if ((t = ConsumePunctuator("+")) || (t = ConsumePunctuator("-")) ||
      (t = ConsumePunctuator("~")) || (t = ConsumePunctuator("!")) ||
      (t = ConsumePunctuator("&")) || (t = ConsumePunctuator("*"))) {
    return CreateASTUnaryPrefixOp(t, ParseCastExpr());
  } else if ((t = ConsumeToken(kTokenKwSizeof))) {
    return CreateASTUnaryPrefixOp(t, ParseUnaryExpr());
  }
  return ParsePrimaryExpr();
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
  if ((t = ConsumePunctuator("="))) {
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

struct Node *ParseJumpStmt() {
  struct Node *t;
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

struct Node *ParseStmt() {
  struct Node *stmt;
  if ((stmt = ParseExprStmt()) || (stmt = ParseJumpStmt())) return stmt;
  return NULL;
}

struct Node *ParseDeclSpecs() {
  struct Node *decl_spec;
  (decl_spec = ConsumeToken(kTokenKwInt)) ||
      (decl_spec = ConsumeToken(kTokenKwChar));
  return decl_spec;
}

struct Node *ParseParamDecl();
struct Node *ParseDirectDecltor() {
  struct Node *n = AllocNode(kASTDirectDecltor);
  n->op = ExpectToken(kTokenIdent);
  while (true) {
    struct Node *t;
    if ((t = ConsumePunctuator("("))) {
      struct Node *arg = ParseParamDecl();
      ExpectPunctuator(")");
      struct Node *nn = AllocNode(kASTDirectDecltor);
      nn->op = t;
      nn->right = arg;
      nn->left = n;
      n = nn;
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

struct Node *ParseParamDecl() {
  struct Node *decl_spec = ParseDeclSpecs();
  if (!decl_spec) return NULL;
  struct Node *n = AllocNode(kASTDecl);
  n->op = decl_spec;
  n->right = ParseDecltor();
  return n;
}

struct Node *ParseDecl() {
  struct Node *decl_spec = ParseDeclSpecs();
  if (!decl_spec) return NULL;
  struct Node *n = AllocNode(kASTDecl);
  n->op = decl_spec;
  n->right = ParseDecltor();
  ExpectPunctuator(";");
  return n;
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

struct Node *toplevel_names;

struct Node *Parse(struct Node *passed_tokens) {
  tokens = passed_tokens;
  token_stream_index = 0;
  struct Node *list = AllocList();
  struct Node *n;
  toplevel_names = AllocList();
  while ((n = ParseCompStmt()) || (n = ParseDecl())) {
    if (n->type == kASTDecl) {
      PrintASTNode(n);
      putc('\n', stderr);
      continue;
    }
    PushToList(list, n);
  }
  struct Node *t;
  if (!(t = NextToken())) return list;
  ErrorWithToken(t, "Unexpected token");
}
