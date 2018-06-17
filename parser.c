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
  return ParseAdditiveExpr(tokens, index, after_index);
  /*
  ASTList *expr_stack = AllocASTList(MAX_NODES_IN_EXPR);
  ASTList *op_stack = AllocASTList(MAX_NODES_IN_EXPR);
  const Token *token;
  while ((token = GetTokenAt(tokens, index))) {
    if (token->type == kInteger) {
      ASTNode *vnode = AllocAndInitASTConstant(token);
      PushASTNodeToList(expr_stack, vnode);
      index++;
    } else if (token->type == kPunctuator) {
      ASTExprBinOpType op_type = GetExprBinOpTypeFromToken(token);
      if (op_type != kOpUndefined) {
        ASTNode *opnode = AllocateASTNodeAsExprBinOp(op_type);
        while (GetSizeOfASTList(op_stack) > 0 &&
               IsExprBinOpPriorTo(GetLastASTNode(op_stack), opnode)) {
          ReduceExprOp(expr_stack, op_stack);
        }
        PushASTNodeToList(op_stack, opnode);
        index++;
      } else if (IsEqualToken(token, ";")) {
        break;
      } else {
        Error("Unexpected punctuator %s", token->str);
      }
    } else if (token->type == kIdentifier) {
      Error("Unexpected identifier %s", token->str);
    } else {
      Error("Unexpected token %s", token->str);
    }
  }

  while (GetSizeOfASTList(op_stack) != 0) {
    ReduceExprOp(expr_stack, op_stack);
  }

  if (GetSizeOfASTList(expr_stack) == 0) {
    return NULL;
  }

  if (GetSizeOfASTList(expr_stack) != 1) {
    PrintASTNode(ToASTNode(expr_stack), 0);
    Error("parse expr failed.");
  }

  *after_index = index;
  return GetASTNodeAt(expr_stack, 0);
  */
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
        token = GetTokenAt(tokens, index + 1);
        if (IsEqualToken(token, ")")) {
          if (!last_direct_decltor) break;
          //
          ASTList *list = AllocASTList(0);
          //
          ASTDirectDecltor *direct_decltor = AllocASTDirectDecltor();
          direct_decltor->direct_decltor = last_direct_decltor;
          direct_decltor->data = ToASTNode(list);
          last_direct_decltor = direct_decltor;
          //
          index += 2;
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

#define MAX_ARGS 16
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

#define MAX_NODES_IN_TRANSLATION_UNIT 64
ASTNode *ParseTranslationUnit(TokenList *tokens, int index, int *after_index) {
  // ASTList<ASTFuncDef | ASTDecl>
  ASTList *list = AllocASTList(MAX_NODES_IN_TRANSLATION_UNIT);
  for (;;) {
    ASTNode *node = ParseFuncDef(tokens, index, &index);
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
