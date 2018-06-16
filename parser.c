#include "compilium.h"

ASTNode *TryReadAsVarDef(TokenList *tokens, int index, int *after_index) {
  ASTVarDef *var_def = AllocASTVarDef();
  const Token *token;
  TokenList *token_list = AllocateTokenList();
  var_def->type_tokens = token_list;

  token = GetTokenAt(tokens, index);
  if (IsEqualToken(token, "const")) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(tokens, ++index);
  }
  if (IsTypeToken(token)) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(tokens, ++index);
  } else {
    return NULL;
  }
  while (IsEqualToken(token, "*")) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(tokens, ++index);
  }
  var_def->name = GetTokenAt(tokens, index++);
  *after_index = index;
  return ToASTNode(var_def);
}

void ReduceExprOp(ASTList *expr_stack, ASTList *op_stack) {
  ASTNode *last_op = PopASTNodeFromList(op_stack);
  if (GetSizeOfASTList(expr_stack) < 2) {
    Error("expr_stack too short (%d < 2)", GetSizeOfASTList(expr_stack));
  }
  ASTNode *right = PopASTNodeFromList(expr_stack);
  ASTNode *left = PopASTNodeFromList(expr_stack);
  SetOperandOfExprBinOp(ToASTExprBinOp(last_op), left, right);
  PushASTNodeToList(expr_stack, last_op);
}

int ExprBinOpPriority[kNumOfExprBinOp];

int IsExprBinOpPriorTo(ASTNode *left, ASTNode *right) {
  ASTExprBinOp *left_op = ToASTExprBinOp(left);
  ASTExprBinOp *right_op = ToASTExprBinOp(right);

  if (!left_op || !right_op) {
    Error("IsExprBinOpPriorTo: node is not an ExprBinOp");
  }

  if (!ExprBinOpPriority[kNumOfExprBinOp - 1]) {
    // priority 0 means undefined.
    ExprBinOpPriority[kOpAdd] = 2;
    ExprBinOpPriority[kOpSub] = 2;
    ExprBinOpPriority[kOpMul] = 4;
  }

  int left_priority = ExprBinOpPriority[left_op->op_type];
  int right_priority = ExprBinOpPriority[right_op->op_type];
  if (!left_priority || !right_priority) {
    Error("IsExprBinOpPriorTo: Undefined operator priority");
  }

  return left_priority >= right_priority;
}

ASTExprBinOpType GetExprBinOpTypeFromToken(const Token *token) {
  if (IsEqualToken(token, "+")) {
    return kOpAdd;
  } else if (IsEqualToken(token, "-")) {
    return kOpSub;
  } else if (IsEqualToken(token, "*")) {
    return kOpMul;
  }
  return kOpUndefined;
}

#define MAX_NODES_IN_EXPR 64
ASTNode *ReadExpression(TokenList *tokens, int index, int *after_index) {
  // 6.5
  // a sequence of operators and operands
  // ... or that designates an object or a function
  ASTList *expr_stack = AllocASTList(MAX_NODES_IN_EXPR);
  ASTList *op_stack = AllocASTList(MAX_NODES_IN_EXPR);
  const Token *token;
  while ((token = GetTokenAt(tokens, index))) {
    if (token->type == kInteger) {
      ASTNode *vnode = AllocateASTNodeAsExprVal(token);
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
}

ASTNode *TryReadExpressionStatement(TokenList *tokens, int index,
                                    int *after_index) {
  // expression-statement:
  //   expression ;
  ASTNode *expr = ReadExpression(tokens, index, &index);
  if (!expr || !IsEqualToken(GetTokenAt(tokens, index++), ";")) return NULL;
  ASTExprStmt *expr_stmt = AllocASTExprStmt();
  expr_stmt->expr = expr;
  *after_index = index;
  return ToASTNode(expr_stmt);
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
ASTNode *TryReadReturnStmt(TokenList *tokens, int index, int *after_index) {
  const Token *token;
  token = GetTokenAt(tokens, index);
  if (IsEqualToken(token, "return")) {
    // jump-statement(return)
    ASTNode *expr_stmt =
        TryReadExpressionStatement(tokens, index + 1, after_index);
    if (!expr_stmt) return NULL;
    ASTReturnStmt *return_stmt = AllocASTReturnStmt();
    return_stmt->expr_stmt = expr_stmt;
    return ToASTNode(return_stmt);
  }
  return NULL;
}

ASTNode *TryReadStatement(TokenList *tokens, int index, int *after_index) {
  // 6.8
  // statement:
  //   labeled-statement
  //   compound-statement
  //   expression-statement
  //   selection-statement
  //   iteration-statement
  //   jump-statement
  ASTNode *statement;
  if ((statement = TryReadReturnStmt(tokens, index, after_index)) ||
      //(statement = TryReadForStatement(tokens, index, after_index)) ||
      (statement = TryReadExpressionStatement(tokens, index, after_index)))
    return statement;
  return NULL;
}

#define MAX_NUM_OF_STATEMENTS_IN_BLOCK 64
ASTNode *ParseCompStmt(TokenList *tokens, int index, int *after_index) {
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
         (stmt = TryReadStatement(tokens, index, &index))) {
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
  return ToASTNode(comp_stmt);
}

#define MAX_ARGS 16
ASTNode *ParseFuncDef(TokenList *tokens, int index, int *after_index) {
  // function-definition
  const Token *token;
  ASTNode *tmp_node;
  if ((tmp_node = TryReadAsVarDef(tokens, index, &index))) {
    // 6.7.6.3
    // T D( parameter-type-list )
    // T D( identifier-list_opt )
    // func decl / def
    ASTNode *type_and_name = tmp_node;
    //
    token = GetTokenAt(tokens, index++);
    if (!IsEqualToken(token, "(")) {
      return NULL;
    }
    // args
    ASTList *arg_list = AllocASTList(MAX_ARGS);
    while ((tmp_node = TryReadAsVarDef(tokens, index, &index))) {
      PushASTNodeToList(arg_list, tmp_node);
      if (!IsEqualToken(GetTokenAt(tokens, index), ",")) {
        break;
      }
      index++;
    }
    //
    token = GetTokenAt(tokens, index++);
    if (!IsEqualToken(token, ")")) {
      return NULL;
    }
    //
    ASTFuncDecl *func_decl = AllocASTFuncDecl();
    func_decl->type_and_name = type_and_name;
    func_decl->arg_list = arg_list;

    ASTNode *comp_stmt = ParseCompStmt(tokens, index, &index);
    if (!comp_stmt) {
      return NULL;
    }
    ASTFuncDef *func_def = AllocASTFuncDef();
    func_def->func_decl = ToASTNode(func_decl);
    func_def->comp_stmt = comp_stmt;
    *after_index = index;
    return ToASTNode(func_def);
  }
  return NULL;
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
