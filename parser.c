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

// Utils

#define MAX_NUM_OF_NODES_IN_COMMA_SEPARATED_LIST 8
ASTList *ParseCommaSeparatedList(TokenStream *stream,
                                 ASTNode *(elem_parser)(TokenStream *stream)) {
  // elem [, elem]*
  ASTList *list = AllocASTList(MAX_NUM_OF_NODES_IN_COMMA_SEPARATED_LIST);
  ASTNode *node = elem_parser(stream);
  if (!node) return NULL;
  PushASTNodeToList(list, node);
  for (;;) {
    if (!ConsumeToken(stream, ",")) break;
    node = elem_parser(stream);
    if (!node) {
      UnpopToken(stream);  // ","
      break;
    }
    PushASTNodeToList(list, node);
  }
  return list;
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

ASTNode *ParsePrimaryExpr(TokenStream *stream) {
  const Token *token = PeekToken(stream);
  if (token->type == kInteger || token->type == kStringLiteral) {
    PopToken(stream);
    return AllocAndInitASTConstant(token);
  } else if (token->type == kIdentifier) {
    PopToken(stream);
    return ToASTNode(AllocAndInitASTIdent(token));
  }
  return NULL;
}

ASTNode *ParsePostExpr(TokenStream *stream) {
  // postfix-expression:
  //   primary-expression
  //   postfix-expression ( argument-expression-list_opt )
  // TODO: not completed
  const static char *ops[] = {"++", "--", NULL};
  ASTNode *last = ParsePrimaryExpr(stream);
  if (!last) return NULL;
  for (;;) {
    if (IsNextToken(stream, "(")) {
      const Token *op = PopToken(stream);
      ASTList *arg_expr_list = ParseCommaSeparatedList(stream, ParseAssignExpr);
      ExpectToken(stream, ")");
      last = AllocAndInitASTExprBinOp(op, last, ToASTNode(arg_expr_list));
    } else if (IsNextTokenInList(stream, ops)) {
      ASTExprUnaryPostOp *op = AllocASTExprUnaryPostOp();
      op->op = PopToken(stream);
      op->expr = last;
      last = ToASTNode(op);
    } else {
      break;
    }
  }
  return last;
}

ASTNode *ParseUnaryExpr(TokenStream *stream) {
  const static char *ops[] = {"+", "-", "~", "!", "++", "--", NULL};
  if (IsNextTokenInList(stream, ops)) {
    ASTExprUnaryPreOp *op = AllocASTExprUnaryPreOp();
    op->op = PopToken(stream);
    op->expr = ParseCastExpr(stream);
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
    ASTKeyword *kw = AllocASTKeyword();

    kw->token = PopToken(stream);
    return_stmt->kw = kw;
    ASTNode *expr_stmt = ToASTNode(ParseExprStmt(stream));
    if (!expr_stmt) return NULL;
    return_stmt->param = expr_stmt;
    return ToASTNode(return_stmt);
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
  // 6.8
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
      (statement = ToASTNode(ParseExprStmt(stream))) ||
      (statement = ParseSelectionStmt(stream)) ||
      (statement = ParseIterationStmt(stream)) ||
      (statement = ParseJumpStmt(stream))) {
    return statement;
  }
  return NULL;
}

ASTExprStmt *ParseExprStmt(TokenStream *stream) {
  // expression-statement:
  //   expression_opt ;
  DebugPrintTokenStream(__func__, stream);
  ASTNode *expr = ParseExpression(stream);
  if (!ConsumeToken(stream, ";")) return NULL;
  ASTExprStmt *expr_stmt = AllocASTExprStmt();
  expr_stmt->expr = expr;
  return expr_stmt;
}

#define MAX_NUM_OF_STATEMENTS_IN_BLOCK 64
ASTCompStmt *ParseCompStmt(TokenStream *stream) {
  // compound-statement:
  //   { block-item-list_opt }
  //
  // block-item:
  //   declaration
  //   statement
  if (!ConsumeToken(stream, "{")) return NULL;
  ASTList *stmt_list = AllocASTList(MAX_NUM_OF_STATEMENTS_IN_BLOCK);
  while (!IsNextToken(stream, "}")) {
    ASTNode *stmt = ToASTNode(ParseDecl(stream));
    if (!stmt) stmt = ParseStmt(stream);
    if (!stmt) break;

    printf("InCompStmt: ");
    PrintASTNode(stmt, 0);
    putchar('\n');

    PushASTNodeToList(stmt_list, stmt);
  }
  ASTCompStmt *comp_stmt = AllocASTCompStmt();
  comp_stmt->stmt_list = stmt_list;
  ExpectToken(stream, "}");
  return comp_stmt;
}

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
    if (ConsumeToken(stream, "(")) {
      if (ConsumeToken(stream, ")")) {
        // direct-declarator ( )
        ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
        direct_decltor->direct_decltor = last_direct_decltor;
        direct_decltor->data = ToASTNode(ParseIdentList(stream));
        last_direct_decltor = direct_decltor;
        continue;
      }
      // direct-declarator ( parameter_type_list )
      ASTList *list = ParseParamTypeList(stream);
      if (!list) Error("ParseParamTypeList should not be empty");
      ExpectToken(stream, ")");
      ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
      direct_decltor->direct_decltor = last_direct_decltor;
      direct_decltor->data = ToASTNode(list);
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

ASTNode *ParseTypeSpec(TokenStream *stream) {
  // type-specifier
  // ASTKeyword | ASTSpec
  // TODO: Impl struct cases (ASTSpec)
  // TODO: Impl utility function to simplify multiple token matching.
  if (!IsNextToken(stream, "int") && !IsNextToken(stream, "char")) {
    return NULL;
  }
  ASTKeyword *kw = AllocASTKeyword();
  kw->token = PopToken(stream);
  return ToASTNode(kw);
}

ASTKeyword *ParseTypeQual(TokenStream *stream) {
  // type-qualifier
  // ASTKeyword
  if (!IsNextToken(stream, "const")) {
    return NULL;
  }
  ASTKeyword *kw = AllocASTKeyword();
  kw->token = PopToken(stream);
  return kw;
}

#define MAX_NODES_IN_DECL_SPECS 4
ASTList *ParseDeclSpecs(TokenStream *stream) {
  // ASTList<ASTKeyword>
  // declaration-specifiers:
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
  // declaration-specifiers
  // ASTList<ASTKeyword>
  return ParseCommaSeparatedList(stream, ParseDecltorNode);
}

ASTDecl *ParseDecl(TokenStream *stream) {
  // declaration:
  //   declaration-specifiers init-declarator-list_opt ;
  ASTList *decl_specs = ParseDeclSpecs(stream);
  if (!decl_specs) {
    return NULL;
  }
  ASTList *init_decltors = ParseInitDecltors(stream);
  // init_decltors is optional
  ExpectToken(stream, ";");
  ASTDecl *decl = AllocASTDecl();
  decl->decl_specs = decl_specs;
  decl->init_decltors = init_decltors;
  return decl;
}

#define MAX_NODES_IN_TRANSLATION_UNIT 64
// ASTList<ASTFuncDef | ASTDecl>
ASTList *ParseTranslationUnit(TokenStream *stream) {
  // translation-unit:
  //   [function-definition declaration]+
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

#define MAX_ROOT_NODES 64
ASTNode *Parse(TokenList *tokens) {
  TokenStream *stream = AllocAndInitTokenStream(tokens);
  return ToASTNode(ParseTranslationUnit(stream));
}
