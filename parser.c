#include "compilium.h"

ASTExprStmt *ParseExprStmt(TokenList *tokens, int index, int *after_index);

ASTNode *ParsePrimaryExpr(TokenList *tokens, int index, int *after_index) {
  const Token *token = GetTokenAt(tokens, index++);
  if (token->type == kInteger) {
    *after_index = index;
    return AllocAndInitASTConstant(token);
  }
  return NULL;
}

ASTNode *ParsePostExpr(TokenList *tokens, int index, int *after_index) {
  return ParsePrimaryExpr(tokens, index, after_index);
}

ASTNode *ParseUnaryExpr(TokenList *tokens, int index, int *after_index) {
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

#define MAX_NODES_IN_EXPR 64
ASTNode *ParseExpression(TokenList *tokens, int index, int *after_index) {
  // expression
  ASTNode *last = NULL;
  ASTNode *node = NULL;
  const Token *op = NULL;
  while ((node = ParseAdditiveExpr(tokens, index, &index))) {
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
  while (!IsEqualToken(GetTokenAt(tokens, index), "}") &&
         (stmt = ParseStmt(tokens, index, &index))) {
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

#define MAX_NUM_OF_NODES_IN_PARAM_TYPE_LIST 8
ASTList *ParseParamTypeList(TokenList *tokens, int index, int *after_index)
{
  ASTList *list = AllocASTList(MAX_NUM_OF_NODES_IN_PARAM_TYPE_LIST);
  const Token *token;
  ASTIdent *node;
  for(;;){
    token = GetTokenAt(tokens, index);
    if(token->type != kIdentifier){
      return NULL;
    }
    node = AllocASTIdent();
    node->token = token;
    PushASTNodeToList(list, ToASTNode(node));
    index++;
    //
    token = GetTokenAt(tokens, index);
    if(token->type != kPunctuator || !IsEqualToken(token, ",")){
      break;
    }
    index++;
  }
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
        // node can be null
        token = GetTokenAt(tokens, index);
        if (IsEqualToken(token, ")")) {
          index++;
          if (!last_direct_decltor) break;
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

ASTDecltor *ParseDecltor(TokenList *tokens, int index, int *after_index) {
  // declarator
  // TODO: Impl pointer(opt)
  ASTDirectDecltor *direct_decltor = ParseDirectDecltor(tokens, index, &index);
  if (!direct_decltor) {
    return NULL;
  }
  ASTDecltor *decltor = AllocASTDecltor();
  decltor->direct_decltor = direct_decltor;
  *after_index = index;
  return decltor;
}

#define MAX_NODES_IN_DECL_SPECS 4
ASTList *ParseDeclSpecs(TokenList *tokens, int index, int *after_index) {
  // declaration-specifiers
  // ASTList<ASTKeyword>
  ASTList *list = AllocASTList(MAX_NODES_IN_DECL_SPECS);
  for (;;) {
    const Token *token = GetTokenAt(tokens, index);
    if (IsEqualToken(token, "int")) {
      ASTKeyword *kw = AllocASTKeyword();
      kw->token = token;
      PushASTNodeToList(list, ToASTNode(kw));
      index++;
      continue;
    }
    break;
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
    printf("comp_stmt is null!");
    return NULL;
  }

  ASTFuncDef *func_def = AllocASTFuncDef();
  func_def->decl_specs = decl_specs;
  func_def->decltor = decltor;
  func_def->comp_stmt = comp_stmt;
  *after_index = index;
  return ToASTNode(func_def);
}

#define MAX_NODES_IN_INIT_DECLTORS  8
ASTList *ParseInitDecltors(TokenList *tokens, int index, int *after_index) {
  // declaration-specifiers
  // ASTList<ASTKeyword>
  ASTList *list = AllocASTList(MAX_NODES_IN_INIT_DECLTORS);
  ASTNode *node;
  const Token *token;
  for (;;) {
    node = ToASTNode(ParseDecltor(tokens, index, &index));
    // TODO: Add InitDecltor case
    if(!node) return NULL;
    PushASTNodeToList(list, node);
    token = GetTokenAt(tokens, index + 1);
    if (!IsEqualToken(token, ",")) break;
    index++;
  }
  *after_index = index;
  return list;
}

ASTDecl *ParseDecl(TokenList *tokens, int index, int *after_index) {
  ASTList *decl_specs = ParseDeclSpecs(tokens, index, &index);
  if(!decl_specs) return NULL;
  ASTList *init_decltors = ParseInitDecltors(tokens, index, &index);
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
    if (node) {
      PushASTNodeToList(list, node);
      continue;
    }
    if (index != GetSizeOfTokenList(tokens)) {
      Error("Unexpected Token %s", GetTokenAt(tokens, index)->str);
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
