#include "compilium.h"

ASTExprStmt *ParseExprStmt(TokenStream *stream);
ASTList *ParseDeclSpecs(TokenStream *stream);
ASTDecltor *ParseDecltor(TokenStream *stream);
ASTDecl *ParseDecl(TokenStream *stream);
ASTNode *ParseAssignExpr(TokenStream *stream);

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
    int i;
    for (i = 0; ops[i]; i++) {
      if (IsNextToken(stream, ops[i])) break;
    }
    if (!ops[i]) break;
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
  ASTNode *last = ParsePrimaryExpr(stream);
  if (!last) return NULL;
  for (;;) {
    if (IsNextToken(stream, "(")) {
      const Token *op = PopToken(stream);
      ASTList *arg_expr_list = ParseCommaSeparatedList(stream, ParseAssignExpr);
      if (!ConsumeToken(stream, ")")) {
        Error("expected )");
      }
      last = AllocAndInitASTExprBinOp(op, last, ToASTNode(arg_expr_list));
    } else {
      break;
    }
  }
  return last;
}

ASTNode *ParseUnaryExpr(TokenStream *stream) { return ParsePostExpr(stream); }

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
  return ParseShiftExpr(stream);
}
ASTNode *ParseEqualityExpr(TokenStream *stream) {
  return ParseRelationalExpr(stream);
}
ASTNode *ParseAndExpr(TokenStream *stream) { return ParseEqualityExpr(stream); }
ASTNode *ParseExclusiveOrExpr(TokenStream *stream) {
  return ParseAndExpr(stream);
}
ASTNode *ParseInclusiveOrExpr(TokenStream *stream) {
  return ParseExclusiveOrExpr(stream);
}
ASTNode *ParseLogicalAndExpr(TokenStream *stream) {
  return ParseInclusiveOrExpr(stream);
}
ASTNode *ParseLogicalOrExpr(TokenStream *stream) {
  return ParseLogicalAndExpr(stream);
}
ASTNode *ParseConditionalExpr(TokenStream *stream) {
  return ParseLogicalOrExpr(stream);
}

ASTNode *ParseAssignExpr(TokenStream *stream) {
  // assignment-expression:
  // [unary-expression assignment-operator]* conditional-expression
  DebugPrintTokenStream(__func__, stream);
  int pos = GetStreamPos(stream);
  ASTNode *last = ParseUnaryExpr(stream);
  if (!last) {
    return ParseConditionalExpr(stream);
  }
  if (!IsNextToken(stream, "=") && !IsNextToken(stream, "*=")) {
    SeekStream(stream, pos);
    return ParseConditionalExpr(stream);
  }
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
  const Token *token;
  if ((token = ConsumeToken(stream, "return"))) {
    // jump-statement(return)
    printf("return-stmt");
    ASTNode *expr_stmt = ToASTNode(ParseExprStmt(stream));
    if (!expr_stmt) {
      DebugPrintTokenStream("return-stmt expr fail", stream);
      return NULL;
    }
    ASTKeyword *kw = AllocASTKeyword();
    kw->token = token;
    ASTJumpStmt *return_stmt = AllocASTJumpStmt();
    return_stmt->kw = kw;
    return_stmt->param = expr_stmt;
    return ToASTNode(return_stmt);
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
  if ((statement = ParseJumpStmt(stream)) ||
      (statement = ToASTNode(ParseExprStmt(stream))))
    return statement;
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
  ASTNode *stmt;
  while (!IsNextToken(stream, "}")) {
    stmt = ToASTNode(ParseDecl(stream));
    if (!stmt) stmt = ParseStmt(stream);
    if (!stmt) break;

    printf("InCompStmt: ");
    PrintASTNode(stmt, 0);
    putchar('\n');

    PushASTNodeToList(stmt_list, stmt);
  }
  ASTCompStmt *comp_stmt = AllocASTCompStmt();
  comp_stmt->stmt_list = stmt_list;
  if (!ConsumeToken(stream, "}")) {
    Error("Expected } but got %s", PeekToken(stream)->str);
  }
  return comp_stmt;
}

ASTIdent *ParseIdent(TokenStream *stream) {
  const Token *token;
  ASTIdent *ident;
  token = PeekToken(stream);
  if (token->type != kIdentifier) {
    return NULL;
  }
  PopToken(stream);
  ident = AllocASTIdent();
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
  if (!IsNextToken(stream, "...")) {
    Error("Token '...' expected, got %s", PeekToken(stream)->str);
  }
  PushASTNodeToList(list, ToASTNode(AllocAndInitASTKeyword(PopToken(stream))));
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
    ASTIdent *ident = AllocASTIdent();
    ident->token = token;
    ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
    direct_decltor->direct_decltor = last_direct_decltor;
    direct_decltor->data = ToASTNode(ident);
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
      if (!ConsumeToken(stream, ")"))
        Error("expected ) after ParseParamTypeList but got %s",
              PeekToken(stream)->str);
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
  if (!ConsumeToken(stream, ";")) {
    Error("Expected ; after decl");
  }
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
