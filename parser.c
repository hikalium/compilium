#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "compilium.h"

ASTExprStmt *ParseExprStmt(TokenStream *stream);
ASTList *ParseDeclSpecs(TokenStream *stream);
ASTDecltor *ParseDecltor(TokenStream *stream);
ASTDecl *ParseDecl(TokenStream *stream);
ASTNode *ParseAssignExpr(TokenStream *stream);
ASTNode *ParseStmt(TokenStream *stream);
ASTCompStmt *ParseCompStmt(TokenStream *stream);
ASTNode *ParseCastExpr(TokenStream *stream);
ASTNode *ParseExpression(TokenStream *stream);
ASTKeyword *ParseTypeQual(TokenStream *stream);
ASTNode *ParseTypeSpec(TokenStream *stream);

// Utils

#define MAX_NUM_OF_NODES_IN_COMMA_SEPARATED_LIST 8
ASTList *ParseListSeparatedByToken(TokenStream *stream,
                                   ASTNode *(elem_parser)(TokenStream *stream),
                                   const char *separator) {
  // elem [separator elem]*
  ASTList *list = AllocASTList(MAX_NUM_OF_NODES_IN_COMMA_SEPARATED_LIST);
  ASTNode *node = elem_parser(stream);
  if (!node) return NULL;
  PushASTNodeToList(list, node);
  for (;;) {
    if (!ConsumeToken(stream, separator)) break;
    node = elem_parser(stream);
    if (!node) {
      UnpopToken(stream);  // push back a separator
      break;
    }
    PushASTNodeToList(list, node);
  }
  return list;
}
ASTList *ParseCommaSeparatedList(TokenStream *stream,
                                 ASTNode *(elem_parser)(TokenStream *stream)) {
  // elem [, elem]*
  return ParseListSeparatedByToken(stream, elem_parser, ",");
}

ASTNode *ParseLeftAssocBinOp(TokenStream *stream,
                             ASTNode *(sub_parser)(TokenStream *stream),
                             const char *ops[]) {
  ASTNode *last = sub_parser(stream);
  while (last) {
    if (!IsNextTokenInList(stream, ops)) break;
    const Token *op = PopToken(stream);
    ASTNode *node = sub_parser(stream);
    if (!node) Error("node should not be NULL");
    last = AllocAndInitASTExprBinOp(op, last, node);
  }
  return last;
}

// Parser

ASTIdent *ParseIdent(TokenStream *stream) {
  const Token *token;
  token = PeekToken(stream);
  if (token->type != kIdentifier) {
    return NULL;
  }
  PopToken(stream);
  ASTIdent *ident = AllocASTIdent();
  ident->token = token;
  return ident;
}

ASTNode *ParseIdentNode(TokenStream *stream) {
  return ToASTNode(ParseIdent(stream));
}

ASTList *ParseIdentList(TokenStream *stream) {
  return ParseCommaSeparatedList(stream, ParseIdentNode);
}

ASTNode *ParsePrimaryExpr(TokenStream *stream) {
  const Token *token = PeekToken(stream);
  if (token->type == kInteger) {
    PopToken(stream);
    char *p;
    const char *s = token->str;
    int n = strtol(s, &p, 0);
    if (!(s[0] != 0 && *p == 0)) {
      Error("%s is not valid as integer.", s);
    }
    return ToASTNode(AllocAndInitASTInteger(n));
  } else if (token->type == kCharacterLiteral) {
    PopToken(stream);
    return ToASTNode(AllocAndInitASTInteger(token->str[0]));
  } else if (token->type == kStringLiteral) {
    PopToken(stream);
    return ToASTNode(AllocAndInitASTString(token->str));
  } else if (token->type == kIdentifier) {
    PopToken(stream);
    return ToASTNode(AllocAndInitASTIdent(token));
  } else if (ConsumeToken(stream, "(")) {
    ASTNode *expr = ParseExpression(stream);
    if (!expr) Error("Expected expression");
    ExpectToken(stream, ")");
    return expr;
  }
  return NULL;
}

ASTNode *ParsePostExpr(TokenStream *stream) {
  // postfix-expression:
  //   primary-expression
  //   postfix-expression ( argument-expression-list_opt )
  const static char *ops[] = {"++", "--", NULL};
  const static char *ops_followed_by_ident[] = {".", "->", NULL};
  ASTNode *last = ParsePrimaryExpr(stream);
  if (!last) return NULL;
  for (;;) {
    if (ConsumeToken(stream, "(")) {
      ASTList *arg_expr_list = ParseCommaSeparatedList(stream, ParseAssignExpr);
      ExpectToken(stream, ")");
      last = AllocAndInitASTExprFuncCall(last, ToASTNode(arg_expr_list));
    } else if (ConsumeToken(stream, "[")) {
      ASTNode *expr = ParseExpression(stream);
      ExpectToken(stream, "]");

      ASTExprUnaryPreOp *op = AllocASTExprUnaryPreOp();
      op->op = AllocToken("*", kPunctuator);
      op->expr =
          AllocAndInitASTExprBinOp(AllocToken("+", kPunctuator), last, expr);

      last = ToASTNode(op);
    } else if (IsNextTokenInList(stream, ops)) {
      ASTExprUnaryPostOp *op = AllocASTExprUnaryPostOp();
      op->op = PopToken(stream);
      op->expr = last;
      last = ToASTNode(op);
    } else if (IsNextTokenInList(stream, ops_followed_by_ident)) {
      const Token *op_token = PopToken(stream);
      last = AllocAndInitASTExprBinOp(op_token, last, ParseIdentNode(stream));
    } else {
      break;
    }
  }
  return last;
}

ASTNode *ParseUnaryExpr(TokenStream *stream) {
  const static char *ops_follows_cast_expr[] = {"&", "*",  "+",  "-", "~",
                                                "!", "++", "--", NULL};
  const static char *ops_follows_unary_expr[] = {"++", "--", "sizeof"};
  if (IsNextTokenInList(stream, ops_follows_cast_expr)) {
    ASTExprUnaryPreOp *op = AllocASTExprUnaryPreOp();
    op->op = PopToken(stream);
    op->expr = ParseCastExpr(stream);
    if (!op->expr) Error("op->expr expected");
    return ToASTNode(op);
  }
  if (IsNextTokenInList(stream, ops_follows_unary_expr)) {
    ASTExprUnaryPreOp *op = AllocASTExprUnaryPreOp();
    op->op = PopToken(stream);
    op->expr = ParseUnaryExpr(stream);
    if (!op->expr) Error("op->expr expected");
    return ToASTNode(op);
  }
  return ParsePostExpr(stream);
}

ASTNode *ParseCastExpr(TokenStream *stream) { return ParseUnaryExpr(stream); }

ASTNode *ParseMultiplicativeExpr(TokenStream *stream) {
  const static char *ops[] = {"*", "/", "%", NULL};
  return ParseLeftAssocBinOp(stream, ParseCastExpr, ops);
}

ASTNode *ParseAdditiveExpr(TokenStream *stream) {
  const static char *ops[] = {"+", "-", NULL};
  return ParseLeftAssocBinOp(stream, ParseMultiplicativeExpr, ops);
}

ASTNode *ParseShiftExpr(TokenStream *stream) {
  const static char *ops[] = {"<<", ">>", NULL};
  return ParseLeftAssocBinOp(stream, ParseAdditiveExpr, ops);
}

ASTNode *ParseRelationalExpr(TokenStream *stream) {
  const static char *ops[] = {"<", ">", "<=", ">=", NULL};
  return ParseLeftAssocBinOp(stream, ParseShiftExpr, ops);
}

ASTNode *ParseEqualityExpr(TokenStream *stream) {
  const static char *ops[] = {"==", "!=", NULL};
  return ParseLeftAssocBinOp(stream, ParseRelationalExpr, ops);
}

ASTNode *ParseAndExpr(TokenStream *stream) {
  const static char *ops[] = {"&", NULL};
  return ParseLeftAssocBinOp(stream, ParseEqualityExpr, ops);
}

ASTNode *ParseExclusiveOrExpr(TokenStream *stream) {
  const static char *ops[] = {"^", NULL};
  return ParseLeftAssocBinOp(stream, ParseAndExpr, ops);
}

ASTNode *ParseInclusiveOrExpr(TokenStream *stream) {
  const static char *ops[] = {"|", NULL};
  return ParseLeftAssocBinOp(stream, ParseExclusiveOrExpr, ops);
}

ASTNode *ParseLogicalAndExpr(TokenStream *stream) {
  const static char *ops[] = {"&&", NULL};
  return ParseLeftAssocBinOp(stream, ParseInclusiveOrExpr, ops);
}

ASTNode *ParseLogicalOrExpr(TokenStream *stream) {
  const static char *ops[] = {"||", NULL};
  return ParseLeftAssocBinOp(stream, ParseLogicalAndExpr, ops);
}

ASTNode *ParseConditionalExpr(TokenStream *stream) {
  ASTNode *cond_expr = ParseLogicalOrExpr(stream);
  if (!ConsumeToken(stream, "?")) return cond_expr;
  ASTNode *true_expr = ParseExpression(stream);
  ExpectToken(stream, ":");
  ASTNode *false_expr = ParseConditionalExpr(stream);

  if (!cond_expr || !true_expr || !false_expr) {
    Error("ParseConditionalExpr failed.");
  }

  ASTCondStmt *cond_stmt = AllocASTCondStmt();
  cond_stmt->cond_expr = cond_expr;
  cond_stmt->true_expr = true_expr;
  cond_stmt->false_expr = false_expr;
  return ToASTNode(cond_stmt);
}

ASTNode *ParseAssignExpr(TokenStream *stream) {
  // assignment-expression:
  // [unary-expression assignment-operator]* conditional-expression
  const static char *ops[] = {
      "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", NULL};
  DebugPrintTokenStream(__func__, stream);
  int pos = GetStreamPos(stream);
  ASTNode *last = ParseUnaryExpr(stream);
  if (!last) {
    return ParseConditionalExpr(stream);
  }
  if (!IsNextTokenInList(stream, ops)) {
    SeekStream(stream, pos);
    return ParseConditionalExpr(stream);
  };
  const Token *op = PopToken(stream);
  ASTNode *node = ParseAssignExpr(stream);
  if (!node) Error("node should not be NULL");
  return AllocAndInitASTExprBinOp(op, last, node);
}

ASTNode *ParseExpression(TokenStream *stream) {
  const static char *ops[] = {",", NULL};
  return ParseLeftAssocBinOp(stream, ParseAssignExpr, ops);
}

ASTNode *ParseJumpStmt(TokenStream *stream) {
  DebugPrintTokenStream(__func__, stream);
  if (IsNextToken(stream, "return")) {
    ASTJumpStmt *return_stmt = AllocASTJumpStmt();
    return_stmt->kw = AllocAndInitASTKeyword(PopToken(stream));
    return_stmt->param = ToASTNode(ParseExpression(stream));
    ExpectToken(stream, ";");
    return ToASTNode(return_stmt);
  } else if (IsNextToken(stream, "break")) {
    ASTJumpStmt *stmt = AllocASTJumpStmt();
    stmt->kw = AllocAndInitASTKeyword(PopToken(stream));
    stmt->param = NULL;  // should be determined by an enclosing statement
    ExpectToken(stream, ";");
    return ToASTNode(stmt);
  }
  return NULL;
}

ASTNode *ParseIterationStmt(TokenStream *stream) {
  DebugPrintTokenStream(__func__, stream);
  if (ConsumeToken(stream, "while")) {
    ASTWhileStmt *while_stmt = AllocASTWhileStmt();
    ExpectToken(stream, "(");
    while_stmt->cond_expr = ParseExpression(stream);
    ExpectToken(stream, ")");
    while_stmt->body_stmt = ParseStmt(stream);
    return ToASTNode(while_stmt);
  } else if (ConsumeToken(stream, "for")) {
    ASTForStmt *for_stmt = AllocASTForStmt();
    ExpectToken(stream, "(");
    for_stmt->init_expr = ParseExpression(stream);
    ExpectToken(stream, ";");
    for_stmt->cond_expr = ParseExpression(stream);
    ExpectToken(stream, ";");
    for_stmt->updt_expr = ParseExpression(stream);
    ExpectToken(stream, ")");
    for_stmt->body_stmt = ParseStmt(stream);
    return ToASTNode(for_stmt);
  }
  return NULL;
}

ASTNode *ParseSelectionStmt(TokenStream *stream) {
  DebugPrintTokenStream(__func__, stream);
  const Token *token;
  if ((token = ConsumeToken(stream, "if"))) {
    ASTIfStmt *if_stmt = AllocASTIfStmt();
    ExpectToken(stream, "(");
    if_stmt->cond_expr = ParseExpression(stream);
    if (!if_stmt->cond_expr) Error("expr is expected.");
    ExpectToken(stream, ")");
    if_stmt->true_stmt = ParseStmt(stream);
    if (!if_stmt->true_stmt) Error("true_stmt is expected.");
    if (ConsumeToken(stream, "else")) {
      if_stmt->false_stmt = ParseStmt(stream);
      if (!if_stmt->false_stmt) Error("false_stmt is expected.");
    } else {
      if_stmt->false_stmt = NULL;
    }
    return ToASTNode(if_stmt);
  }
  return NULL;
}

ASTNode *ParseStmt(TokenStream *stream) {
  // statement:
  //   labeled-statement
  //   compound-statement
  //   expression-statement
  //   selection-statement
  //   iteration-statement
  //   jump-statement
  DebugPrintTokenStream(__func__, stream);
  ASTNode *statement;
  if ((statement = ToASTNode(ParseCompStmt(stream))) ||
      (statement = ParseSelectionStmt(stream)) ||
      (statement = ParseIterationStmt(stream)) ||
      (statement = ParseJumpStmt(stream)) ||
      (statement = ToASTNode(ParseExprStmt(stream)))) {
    return statement;
  }
  return NULL;
}

ASTExprStmt *ParseExprStmt(TokenStream *stream) {
  // expression-statement:
  //   expression_opt ;
  DebugPrintTokenStream(__func__, stream);
  ASTNode *expr = ParseExpression(stream);
  ExpectToken(stream, ";");
  ASTExprStmt *expr_stmt = AllocASTExprStmt();
  expr_stmt->expr = expr;
  return expr_stmt;
}

#define MAX_NUM_OF_STATEMENTS_IN_BLOCK 64
ASTCompStmt *ParseCompStmt(TokenStream *stream) {
  // compound-statement:
  //   { [declaration | statement]* }
  if (!ConsumeToken(stream, "{")) return NULL;
  ASTList *stmt_list = AllocASTList(MAX_NUM_OF_STATEMENTS_IN_BLOCK);
  while (!IsNextToken(stream, "}")) {
    ASTNode *stmt = ToASTNode(ParseDecl(stream));
    if (!stmt) stmt = ParseStmt(stream);
    if (!stmt) break;
    PushASTNodeToList(stmt_list, stmt);
  }
  ASTCompStmt *comp_stmt = AllocASTCompStmt();
  comp_stmt->stmt_list = stmt_list;
  ExpectToken(stream, "}");
  return comp_stmt;
}

ASTParamDecl *ParseParamDecl(TokenStream *stream) {
  // parameter-declaration
  // TODO: Impl abstract case.
  ASTList *decl_specs = ParseDeclSpecs(stream);
  if (!decl_specs) return NULL;
  ASTDecltor *decltor = ParseDecltor(stream);
  if (!decltor) return NULL;

  ASTParamDecl *param_decl = AllocASTParamDecl();
  param_decl->decl_specs = decl_specs;
  param_decl->decltor = ToASTNode(decltor);

  return param_decl;
}

ASTNode *ParseParamDeclNode(TokenStream *stream) {
  return ToASTNode(ParseParamDecl(stream));
}

ASTList *ParseParamList(TokenStream *stream) {
  // parameter-list
  return ParseCommaSeparatedList(stream, ParseParamDeclNode);
}

ASTList *ParseParamTypeList(TokenStream *stream) {
  // parameter-type-list
  ASTList *list = ParseParamList(stream);
  if (!ConsumeToken(stream, ",")) {
    return list;
  }
  PushASTNodeToList(
      list, ToASTNode(AllocAndInitASTKeyword(ExpectToken(stream, "..."))));
  return list;
}

ASTDirectDecltor *ParseDirectDecltor(TokenStream *stream) {
  // direct-declarator:
  //   identifier
  //   direct-declarator ( parameter_type_list )
  // TODO: Impl ( declarator ) case

  ASTDirectDecltor *last_direct_decltor = NULL;
  const Token *token = PeekToken(stream);
  if (!token) return NULL;
  if (token->type == kIdentifier) {
    PopToken(stream);
    ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
    direct_decltor->direct_decltor = last_direct_decltor;
    direct_decltor->data = ToASTNode(AllocAndInitASTIdent(token));
    last_direct_decltor = direct_decltor;
  } else {
    return NULL;
  }

  for (;;) {
    ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
    direct_decltor->direct_decltor = last_direct_decltor;
    direct_decltor->bracket_token = PeekToken(stream);
    if (ConsumeToken(stream, "(")) {
      if (ConsumeToken(stream, ")")) {
        // direct-declarator ( )
        direct_decltor->data = ToASTNode(ParseIdentList(stream));
        last_direct_decltor = direct_decltor;
        continue;
      }
      // direct-declarator ( parameter_type_list )
      ASTList *list = ParseParamTypeList(stream);
      if (!list) Error("ParseParamTypeList should not be empty");
      ExpectToken(stream, ")");
      direct_decltor->data = ToASTNode(list);
      last_direct_decltor = direct_decltor;
      continue;
    } else if (ConsumeToken(stream, "[")) {
      direct_decltor->data = ParseAssignExpr(stream);
      ExpectToken(stream, "]");
      last_direct_decltor = direct_decltor;
      continue;
    }
    break;
  }
  return last_direct_decltor;
}

ASTPointer *ParsePointer(TokenStream *stream) {
  if (!ConsumeToken(stream, "*")) return NULL;
  // TODO: impl type-qual-list(opt)
  ASTPointer *pointer = AllocASTPointer();
  pointer->pointer = ParsePointer(stream);
  return pointer;
}

ASTDecltor *ParseDecltor(TokenStream *stream) {
  // declarator
  ASTPointer *pointer = ParsePointer(stream);  // optional
  ASTDirectDecltor *direct_decltor = ParseDirectDecltor(stream);
  if (!direct_decltor) {
    return NULL;
  }
  ASTDecltor *decltor = AllocASTDecltor();
  decltor->pointer = pointer;
  decltor->direct_decltor = direct_decltor;
  return decltor;
}

ASTNode *ParseDecltorNode(TokenStream *stream) {
  return ToASTNode(ParseDecltor(stream));
}

#define MAX_NODES_IN_DECL_SPECS 4
ASTList *ParseSpecQualList(TokenStream *stream) {
  // specifier-qualifier-list:
  //  [type-specifier type-qualifier]+
  ASTList *list = AllocASTList(MAX_NODES_IN_DECL_SPECS);
  ASTNode *node;
  for (;;) {
    node = ParseTypeSpec(stream);
    if (!node) node = ToASTNode(ParseTypeQual(stream));
    if (!node) break;
    PushASTNodeToList(list, node);
  }
  if (GetSizeOfASTList(list) == 0) return NULL;
  return list;
}

ASTNode *ParseStructDecl(TokenStream *stream) {
  ASTList *spec_qual_list = ParseSpecQualList(stream);
  if (!spec_qual_list) return NULL;
  ASTList *struct_decltor_list =
      ParseCommaSeparatedList(stream, ParseDecltorNode);

  ASTStructDecl *struct_decl = AllocASTStructDecl();
  struct_decl->spec_qual_list = spec_qual_list;
  struct_decl->struct_decltor_list = struct_decltor_list;
  return ToASTNode(struct_decl);
}

ASTNode *ParseStorageClassSpec(TokenStream *stream) {
  // storage-class-specifier
  const static char *storage_class_specs[] = {"typedef", NULL};
  if (!IsNextTokenInList(stream, storage_class_specs)) return NULL;
  return ToASTNode(AllocAndInitASTKeyword(PopToken(stream)));
}

ASTNode *ParseTypeSpec(TokenStream *stream) {
  // type-specifier
  const static char *single_token_type_specs[] = {"void", "char", "int", NULL};
  if (IsNextTokenInList(stream, single_token_type_specs)) {
    return ToASTNode(AllocAndInitASTKeyword(PopToken(stream)));
  } else if (ConsumeToken(stream, "struct")) {
    ASTStructSpec *struct_spec = AllocASTStructSpec();
    if (!IsNextToken(stream, "{")) {
      struct_spec->ident = PopToken(stream);
      assert(struct_spec->ident);
      if (!IsNextToken(stream, "{")) return ToASTNode(struct_spec);
    }
    ExpectToken(stream, "{");
    struct_spec->struct_decl_list =
        ParseListSeparatedByToken(stream, ParseStructDecl, ";");
    ExpectToken(stream, ";");
    ExpectToken(stream, "}");
    return ToASTNode(struct_spec);
  }
  const Token *token = PeekToken(stream);
  if (!token) return NULL;
  ASTNode *typedef_type = FindInContext(identifiers, token->str);
  if (!typedef_type) return NULL;
  PopToken(stream);
  return typedef_type;
}

ASTKeyword *ParseTypeQual(TokenStream *stream) {
  // type-qualifier
  // ASTKeyword
  if (!IsNextToken(stream, "const")) return NULL;
  return AllocAndInitASTKeyword(PopToken(stream));
}

#define MAX_NODES_IN_DECL_SPECS 4
ASTList *ParseDeclSpecs(TokenStream *stream) {
  // ASTList<ASTKeyword>
  // declaration-specifiers:
  //  [type-specifier type-qualifier]+
  ASTList *list = AllocASTList(MAX_NODES_IN_DECL_SPECS);
  ASTNode *node;
  for (;;) {
    node = ParseStorageClassSpec(stream);
    if (!node) node = ParseTypeSpec(stream);
    if (!node) node = ToASTNode(ParseTypeQual(stream));
    if (!node) break;
    PushASTNodeToList(list, node);
  }
  if (GetSizeOfASTList(list) == 0) return NULL;
  return list;
}

ASTNode *ParseFuncDef(TokenStream *stream) {
  // function-definition
  // TODO: Impl better fallback
  int org_pos = GetStreamPos(stream);
  ASTList *decl_specs = ParseDeclSpecs(stream);
  ASTDecltor *decltor = ParseDecltor(stream);
  ASTCompStmt *comp_stmt = ParseCompStmt(stream);
  if (!decl_specs || !decltor || !comp_stmt) {
    SeekStream(stream, org_pos);
    return NULL;
  }
  ASTFuncDef *func_def = AllocASTFuncDef();
  func_def->decl_specs = decl_specs;
  func_def->decltor = decltor;
  func_def->comp_stmt = comp_stmt;
  return ToASTNode(func_def);
}

#define MAX_NODES_IN_INIT_DECLTORS 8
ASTList *ParseInitDecltors(TokenStream *stream) {
  // ASTList<ASTKeyword>
  // declaration-specifiers
  return ParseCommaSeparatedList(stream, ParseDecltorNode);
}

ASTDecl *ParseDecl(TokenStream *stream) {
  // declaration:
  //   declaration-specifiers init-declarator-list_opt ;
  ASTList *decl_specs = ParseDeclSpecs(stream);
  if (!decl_specs) return NULL;
  ASTList *init_decltors = ParseInitDecltors(stream);
  // init_decltors is optional
  ExpectToken(stream, ";");
  ASTDecl *decl = AllocASTDecl();
  decl->decl_specs = decl_specs;
  decl->init_decltors = init_decltors;

  if (IsTypedefDeclSpecs(decl_specs)) {
    assert(GetSizeOfASTList(decl->init_decltors) == 1);
    ASTDecltor *typedef_decltor =
        ToASTDecltor(GetASTNodeAt(decl->init_decltors, 0));
    DebugPrintASTNode(typedef_decltor);
    ASTType *type = AllocAndInitASTType(decl_specs, typedef_decltor);
    AppendTypeToContext(identifiers,
                        GetIdentTokenFromDecltor(typedef_decltor)->str, type);
    PrintContext(identifiers);
  }
  return decl;
}

#define MAX_NODES_IN_TRANSLATION_UNIT 64
ASTList *ParseTranslationUnit(TokenStream *stream) {
  // ASTList<ASTFuncDef | ASTDecl>
  // translation-unit:
  //   [function-definition | declaration]+
  ASTList *list = AllocASTList(MAX_NODES_IN_TRANSLATION_UNIT);
  ASTNode *node;
  for (;;) {
    node = ParseFuncDef(stream);
    if (!node) node = ToASTNode(ParseDecl(stream));
    if (!node) {
      break;
    }
    printf("Read in TopLevel: ");
    PrintASTNode(node, 0);
    PushASTNodeToList(list, node);
    putchar('\n');
  }
  if (PeekToken(stream)) {
    const Token *token = PeekToken(stream);
    Error("Unexpected Token %s (%s:%d)", token->str, token->filename,
          token->line);
  }
  return list;
}

ASTNode *Parse(TokenList *tokens) {
  identifiers = AllocContext(NULL);
  TokenStream *stream = AllocAndInitTokenStream(tokens);
  return ToASTNode(ParseTranslationUnit(stream));
}
