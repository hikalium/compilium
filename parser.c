#include "compilium.h"

ASTExprStmt *ParseExprStmt(TokenList *tokens, int index, int *after_index);
ASTList *ParseDeclSpecs(TokenList *tokens, int index, int *after_index);
ASTDecltor *ParseDecltor(TokenList *tokens, int index, int *after_index);
ASTDecl *ParseDecl(TokenList *tokens, int index, int *after_index);
ASTNode *ParseAssignExpr(TokenList *tokens, int index, int *after_index);

#define MAX_NUM_OF_NODES_IN_COMMA_SEPARATED_LIST 8
ASTList *ParseCommaSeparatedList(TokenList *tokens, int index, int *after_index,
                                 ASTNode *(elem_parser)(TokenList *tokens,
                                                        int index,
                                                        int *after_index)) {
  ASTList *list = AllocASTList(MAX_NUM_OF_NODES_IN_COMMA_SEPARATED_LIST);
  ASTNode *node;
  const Token *token;
  for (;;) {
    node = elem_parser(tokens, index, &index);
    if (!node) break;
    *after_index = index;
    PushASTNodeToList(list, node);

    token = GetTokenAt(tokens, index++);
    if (!IsEqualToken(token, ",")) break;
  }
  return list;
}

ASTNode *ParsePrimaryExpr(TokenList *tokens, int index, int *after_index) {
  const Token *token = GetTokenAt(tokens, index++);
  if (token->type == kInteger || token->type == kStringLiteral) {
    *after_index = index;
    return AllocAndInitASTConstant(token);
  } else if (token->type == kIdentifier) {
    *after_index = index;
    return ToASTNode(AllocAndInitASTIdent(token));
  }
  return NULL;
}

ASTNode *ParsePostExpr(TokenList *tokens, int index, int *after_index) {
  // postfix-expression
  ASTNode *last = NULL;
  const Token *op = NULL;
  last = ParsePrimaryExpr(tokens, index, &index);
  *after_index = index;
  if (!last) return NULL;
  for (;;) {
    op = GetTokenAt(tokens, index++);
    if (IsEqualToken(op, "(")) {
      ASTList *arg_expr_list =
          ParseCommaSeparatedList(tokens, index, &index, ParseAssignExpr);
      if (!IsEqualToken(GetTokenAt(tokens, index++), ")")) break;
      last = AllocAndInitASTExprBinOp(op, last, ToASTNode(arg_expr_list));
      *after_index = index;
      continue;
    }
    break;
  }
  return last;
}

ASTNode *ParseUnaryExpr(TokenList *tokens, int index, int *after_index) {
  ASTNode *node = ParsePostExpr(tokens, index, after_index);
  if (!node) {
  }
  return ParsePostExpr(tokens, index, after_index);
}

ASTNode *ParseCastExpr(TokenList *tokens, int index, int *after_index) {
  return ParseUnaryExpr(tokens, index, after_index);
}

ASTNode *ParseMultiplicativeExpr(TokenList *tokens, int index,
                                 int *after_index) {
  // multiplicative-expression
  ASTNode *last = NULL;
  ASTNode *node = NULL;
  const Token *op = NULL;
  while ((node = ParseCastExpr(tokens, index, &index))) {
    if (!last) {
      last = node;
    } else {
      last = AllocAndInitASTExprBinOp(op, last, node);
    }
    op = GetTokenAt(tokens, index);
    if (!IsEqualToken(op, "*") && !IsEqualToken(op, "/") &&
        !IsEqualToken(op, "%")) {
      break;
    }
    index++;
  }
  *after_index = index;
  return last;
}

ASTNode *ParseAdditiveExpr(TokenList *tokens, int index, int *after_index) {
  // additive-expression
  ASTNode *last = NULL;
  ASTNode *node = NULL;
  const Token *op = NULL;
  while ((node = ParseMultiplicativeExpr(tokens, index, &index))) {
    if (!last) {
      last = node;
    } else {
      last = AllocAndInitASTExprBinOp(op, last, node);
    }
    op = GetTokenAt(tokens, index);
    if (!IsEqualToken(op, "+") && !IsEqualToken(op, "-")) {
      break;
    }
    index++;
  }
  *after_index = index;
  return last;
}

ASTNode *ParseAssignExpr(TokenList *tokens, int index, int *after_index) {
  // assignment-expression
  ASTNode *last = NULL;
  ASTNode *node = NULL;
  const Token *op = NULL;
  // TODO: ParseAdditiveExpr -> ParseCondExpr
  while ((node = ParseAdditiveExpr(tokens, index, &index))) {
    if (!last) {
      last = node;
    } else {
      last = AllocAndInitASTExprBinOp(op, last, node);
    }
    op = GetTokenAt(tokens, index);
    if (!IsEqualToken(op, "=") && !IsEqualToken(op, "*=")) {
      break;
    }
    index++;
  }
  *after_index = index;
  return last;
}

#define MAX_NODES_IN_EXPR 64
ASTNode *ParseExpression(TokenList *tokens, int index, int *after_index) {
  // expression
  ASTNode *last = NULL;
  ASTNode *node = NULL;
  const Token *op = NULL;
  while ((node = ParseAssignExpr(tokens, index, &index))) {
    if (!last) {
      last = node;
    } else {
      last = AllocAndInitASTExprBinOp(op, last, node);
    }
    op = GetTokenAt(tokens, index);
    if (!IsEqualToken(op, ",")) {
      break;
    }
    index++;
  }
  *after_index = index;
  return last;
}

/*
ASTNode *TryReadForStatement(TokenList *tokens, int index, int *after_index) {
  // 6.8.5.3
  // for ( expression(opt) ; expression(opt) ; expression(opt) ) statement:

  if (!IsEqualToken(GetTokenAt(tokens, index++), "for")) return NULL;
  if (!IsEqualToken(GetTokenAt(tokens, index++), "(")) return NULL;
  ASTNode *init_expr = ReadExpression(tokens, index, &index);
  if (!IsEqualToken(GetTokenAt(tokens, index++), ";")) return NULL;
  ASTNode *cond_expr = ReadExpression(tokens, index, &index);
  if (!IsEqualToken(GetTokenAt(tokens, index++), ";")) return NULL;
  ASTNode *updt_expr = ReadExpression(tokens, index, &index);
  if (!IsEqualToken(GetTokenAt(tokens, index++), ")")) return NULL;
  ASTNode *body_comp_stmt = TryReadCompoundStatement(tokens, index, &index);
  if (!body_comp_stmt) {
    Error("TryReadForStatement: body_comp_stmt is null");
  }
  ASTForStmt *for_stmt = AllocASTForStmt();
  for_stmt->init_expr = init_expr;
  for_stmt->cond_expr = cond_expr;
  for_stmt->updt_expr = updt_expr;
  for_stmt->body_comp_stmt = body_comp_stmt;
  *after_index = index;
  return ToASTNode(for_stmt);
}
*/
ASTNode *ParseJumpStmt(TokenList *tokens, int index, int *after_index) {
  const Token *token;
  token = GetTokenAt(tokens, index);
  if (IsEqualToken(token, "return")) {
    // jump-statement(return)
    ASTNode *expr_stmt =
        ToASTNode(ParseExprStmt(tokens, index + 1, after_index));
    if (!expr_stmt) return NULL;
    ASTKeyword *kw = AllocASTKeyword();
    kw->token = token;
    ASTJumpStmt *return_stmt = AllocASTJumpStmt();
    return_stmt->kw = kw;
    return_stmt->param = expr_stmt;
    return ToASTNode(return_stmt);
  }
  return NULL;
}

ASTNode *ParseStmt(TokenList *tokens, int index, int *after_index) {
  // 6.8
  // statement:
  //   labeled-statement
  //   compound-statement
  //   expression-statement
  //   selection-statement
  //   iteration-statement
  //   jump-statement
  ASTNode *statement;
  if ((statement = ParseJumpStmt(tokens, index, after_index)) ||
      //(statement = TryReadForStatement(tokens, index, after_index)) ||
      (statement = ToASTNode(ParseExprStmt(tokens, index, after_index))))
    return statement;
  return NULL;
}

ASTExprStmt *ParseExprStmt(TokenList *tokens, int index, int *after_index) {
  // expression-statement:
  //   expression ;
  ASTNode *expr = ParseExpression(tokens, index, &index);
  if (!IsEqualToken(GetTokenAt(tokens, index++), ";")) return NULL;
  ASTExprStmt *expr_stmt = AllocASTExprStmt();
  expr_stmt->expr = expr;
  *after_index = index;
  return expr_stmt;
}

#define MAX_NUM_OF_STATEMENTS_IN_BLOCK 64
ASTCompStmt *ParseCompStmt(TokenList *tokens, int index, int *after_index) {
  // 6.8.2
  // compound-statement:
  //   { block-item-list(opt) }
  //
  // block-item:
  //   declaration
  //   statement
  if (!IsEqualToken(GetTokenAt(tokens, index++), "{")) return NULL;
  //
  ASTList *stmt_list = AllocASTList(MAX_NUM_OF_STATEMENTS_IN_BLOCK);
  ASTNode *stmt;
  while (!IsEqualToken(GetTokenAt(tokens, index), "}")) {
    stmt = ToASTNode(ParseDecl(tokens, index, &index));
    if (!stmt) stmt = ParseStmt(tokens, index, &index);
    if (!stmt) break;
    PushASTNodeToList(stmt_list, stmt);
  }
  ASTCompStmt *comp_stmt = AllocASTCompStmt();
  comp_stmt->stmt_list = stmt_list;
  //
  if (!IsEqualToken(GetTokenAt(tokens, index), "}")) {
    Error("Expected } but got %s", GetTokenAt(tokens, index)->str);
  }
  index++;
  //
  *after_index = index;
  return comp_stmt;
}

ASTIdent *ParseIdent(TokenList *tokens, int index, int *after_index) {
  const Token *token;
  ASTIdent *ident;
  token = GetTokenAt(tokens, index++);
  if (token->type != kIdentifier) {
    return NULL;
  }
  ident = AllocASTIdent();
  ident->token = token;
  *after_index = index;
  return ident;
}

ASTNode *ParseIdentNode(TokenList *tokens, int index, int *after_index) {
  return ToASTNode(ParseIdent(tokens, index, after_index));
}

ASTList *ParseIdentList(TokenList *tokens, int index, int *after_index) {
  return ParseCommaSeparatedList(tokens, index, after_index, ParseIdentNode);
}

ASTParamDecl *ParseParamDecl(TokenList *tokens, int index, int *after_index) {
  // parameter-declaration
  // TODO: Impl abstract case.
  ASTList *decl_specs = ParseDeclSpecs(tokens, index, &index);
  if (!decl_specs) return NULL;
  ASTDecltor *decltor = ParseDecltor(tokens, index, &index);
  if (!decltor) return NULL;

  ASTParamDecl *param_decl = AllocASTParamDecl();
  param_decl->decl_specs = decl_specs;
  param_decl->decltor = ToASTNode(decltor);

  *after_index = index;
  return param_decl;
}

ASTNode *ParseParamDeclNode(TokenList *tokens, int index, int *after_index) {
  return ToASTNode(ParseParamDecl(tokens, index, after_index));
}

ASTList *ParseParamList(TokenList *tokens, int index, int *after_index) {
  // parameter-list
  return ParseCommaSeparatedList(tokens, index, after_index,
                                 ParseParamDeclNode);
}

ASTList *ParseParamTypeList(TokenList *tokens, int index, int *after_index) {
  // parameter-type-list
  // TODO: Impl ", ..." case
  ASTList *list = ParseParamList(tokens, index, after_index);
  index = *after_index;
  //
  const Token *token;
  token = GetTokenAt(tokens, index++);
  PrintToken(token);
  if (!IsEqualToken(token, ",")) return list;
  token = GetTokenAt(tokens, index++);
  PrintToken(token);
  if (!IsEqualToken(token, "...")) return list;
  PushASTNodeToList(list, ToASTNode(AllocAndInitASTKeyword(token)));
  *after_index = index;
  return list;
}

ASTDirectDecltor *ParseDirectDecltor(TokenList *tokens, int index,
                                     int *after_index) {
  // direct-declarator
  ASTDirectDecltor *last_direct_decltor = NULL;
  for (;;) {
    const Token *token = GetTokenAt(tokens, index);
    if (!token) break;
    if (token->type == kIdentifier) {
      if (last_direct_decltor) break;
      //
      ASTIdent *ident = AllocASTIdent();
      ident->token = token;
      //
      ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
      direct_decltor->direct_decltor = last_direct_decltor;
      direct_decltor->data = ToASTNode(ident);
      last_direct_decltor = direct_decltor;
      //
      index++;
      continue;
    } else if (token->type == kPunctuator) {
      if (IsEqualToken(token, "(")) {
        index++;
        //
        ASTList *list;
        list = ParseParamTypeList(tokens, index, &index);
        if (!list) list = ParseIdentList(tokens, index, &index);
        // Identlist can be null
        token = GetTokenAt(tokens, index);
        if (IsEqualToken(token, ")")) {
          if (!last_direct_decltor) break;
          index++;
          //
          ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
          direct_decltor->direct_decltor = last_direct_decltor;
          direct_decltor->data = ToASTNode(list);
          last_direct_decltor = direct_decltor;
          continue;
        }
      }
    }
    break;
  }
  *after_index = index;
  return last_direct_decltor;
}

ASTPointer *ParsePointer(TokenList *tokens, int index, int *after_index) {
  const Token *token = GetTokenAt(tokens, index++);
  if (!IsEqualToken(token, "*")) return NULL;
  // TODO: impl type-qual-list(opt)
  ASTPointer *pointer = AllocASTPointer();
  pointer->pointer = ParsePointer(tokens, index, &index);
  *after_index = index;
  return pointer;
}

ASTDecltor *ParseDecltor(TokenList *tokens, int index, int *after_index) {
  // declarator
  ASTPointer *pointer = ParsePointer(tokens, index, &index);  // optional
  ASTDirectDecltor *direct_decltor = ParseDirectDecltor(tokens, index, &index);
  if (!direct_decltor) {
    return NULL;
  }
  ASTDecltor *decltor = AllocASTDecltor();
  decltor->pointer = pointer;
  decltor->direct_decltor = direct_decltor;
  *after_index = index;
  return decltor;
}

ASTNode *ParseDecltorNode(TokenList *tokens, int index, int *after_index) {
  return ToASTNode(ParseDecltor(tokens, index, after_index));
}

ASTNode *ParseTypeSpec(TokenList *tokens, int index, int *after_index) {
  // type-specifier
  // ASTKeyword | ASTSpec
  // TODO: Impl struct cases (ASTSpec)
  const Token *token = GetTokenAt(tokens, index++);
  if (IsEqualToken(token, "int") || IsEqualToken(token, "char")) {
    ASTKeyword *kw = AllocASTKeyword();
    kw->token = token;

    *after_index = index;
    return ToASTNode(kw);
  }
  return NULL;
}

ASTKeyword *ParseTypeQual(TokenList *tokens, int index, int *after_index) {
  // type-qualifier
  // ASTKeyword
  const Token *token = GetTokenAt(tokens, index++);
  if (IsEqualToken(token, "const")) {
    ASTKeyword *kw = AllocASTKeyword();
    kw->token = token;

    *after_index = index;
    return kw;
  }
  return NULL;
}

#define MAX_NODES_IN_DECL_SPECS 4
ASTList *ParseDeclSpecs(TokenList *tokens, int index, int *after_index) {
  // declaration-specifiers
  // ASTList<ASTKeyword>
  ASTList *list = AllocASTList(MAX_NODES_IN_DECL_SPECS);
  ASTNode *node;
  for (;;) {
    node = ParseTypeSpec(tokens, index, &index);
    if (!node) node = ToASTNode(ParseTypeQual(tokens, index, &index));
    if (!node) break;
    PushASTNodeToList(list, node);
  }
  *after_index = index;
  return list;
}

ASTNode *ParseFuncDef(TokenList *tokens, int index, int *after_index) {
  // function-definition
  ASTList *decl_specs = ParseDeclSpecs(tokens, index, &index);
  if (!decl_specs) {
    return NULL;
  }

  ASTDecltor *decltor = ParseDecltor(tokens, index, &index);
  if (!decltor) {
    return NULL;
  }

  ASTCompStmt *comp_stmt = ParseCompStmt(tokens, index, &index);
  if (!comp_stmt) {
    return NULL;
  }

  ASTFuncDef *func_def = AllocASTFuncDef();
  func_def->decl_specs = decl_specs;
  func_def->decltor = decltor;
  func_def->comp_stmt = comp_stmt;
  *after_index = index;
  return ToASTNode(func_def);
}

#define MAX_NODES_IN_INIT_DECLTORS 8
ASTList *ParseInitDecltors(TokenList *tokens, int index, int *after_index) {
  // declaration-specifiers
  // ASTList<ASTKeyword>
  return ParseCommaSeparatedList(tokens, index, after_index, ParseDecltorNode);
}

ASTDecl *ParseDecl(TokenList *tokens, int index, int *after_index) {
  ASTList *decl_specs = ParseDeclSpecs(tokens, index, &index);
  if (!decl_specs) {
    return NULL;
  }
  ASTList *init_decltors = ParseInitDecltors(tokens, index, &index);
  // init_decltors is optional
  if (!IsEqualToken(GetTokenAt(tokens, index++), ";")) {
    return NULL;
  }
  //
  *after_index = index;
  ASTDecl *decl = AllocASTDecl();
  decl->decl_specs = decl_specs;
  decl->init_decltors = init_decltors;
  return decl;
}

#define MAX_NODES_IN_TRANSLATION_UNIT 64
ASTNode *ParseTranslationUnit(TokenList *tokens, int index, int *after_index) {
  // ASTList<ASTFuncDef | ASTDecl>
  ASTList *list = AllocASTList(MAX_NODES_IN_TRANSLATION_UNIT);
  ASTNode *node;
  for (;;) {
    node = ParseFuncDef(tokens, index, &index);
    if (!node) node = ToASTNode(ParseDecl(tokens, index, &index));
    if (node) {
      printf("Read in TopLevel: ");
      PrintASTNode(node, 0);
      PushASTNodeToList(list, node);
      continue;
    }
    if (index != GetSizeOfTokenList(tokens)) {
      const Token *token = GetTokenAt(tokens, index);
      Error("Unexpected Token %s (%s:%d)", token->str, token->filename,
            token->line);
    }
    break;
  }
  *after_index = index;
  return ToASTNode(list);
}

#define MAX_ROOT_NODES 64
ASTNode *Parse(TokenList *tokens) {
  int index = 0;
  return ParseTranslationUnit(tokens, index, &index);
}
