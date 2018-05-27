#include "compilium.h"

ASTNode *TryReadAsVarDef(TokenList *tokens, int index, int *after_index) {
  ASTNode *var_def = AllocateASTNode(kVarDef);
  const Token *token;
  TokenList *token_list = AllocateTokenList();
  var_def->data.var_def.type_tokens = token_list;

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
  var_def->data.var_def.name = GetTokenAt(tokens, index++);
  *after_index = index;
  return var_def;
}

void ReduceExprOp(ASTNodeList *expr_stack, ASTNodeList *op_stack) {
  ASTNode *last_op = PopASTNodeFromList(op_stack);
  if (GetSizeOfASTNodeList(expr_stack) < 2) {
    Error("expr_stack too short (%d < 2)", GetSizeOfASTNodeList(expr_stack));
  }
  ASTNode *right = PopASTNodeFromList(expr_stack);
  ASTNode *left = PopASTNodeFromList(expr_stack);
  SetOperandOfExprBinOp(last_op, left, right);
  PushASTNodeToList(expr_stack, last_op);
}

#define MAX_NODES_IN_EXPR 64
ASTNode *ReadExpression(TokenList *tokens, int index, int *after_index) {
  // 6.5
  // a sequence of operators and operands
  // ... or that designates an object or a function
  ASTNodeList *expr_stack = AllocateASTNodeList(MAX_NODES_IN_EXPR);
  ASTNodeList *op_stack = AllocateASTNodeList(MAX_NODES_IN_EXPR);
  const Token *token;
  while ((token = GetTokenAt(tokens, index))) {
    if (token->type == kInteger) {
      ASTNode *vnode = AllocateASTNodeAsExprVal(token);
      PushASTNodeToList(expr_stack, vnode);
      index++;
    } else if (token->type == kPunctuator) {
      if (IsEqualToken(token, "+")) {
        ASTNode *opnode = AllocateASTNodeAsExprBinOp(kOpAdd);
        if (GetSizeOfASTNodeList(op_stack) > 0) {
          ReduceExprOp(expr_stack, op_stack);
        }
        PushASTNodeToList(op_stack, opnode);
        index++;
      } else {
        break;
      }
    } else {
      break;
    }
  }

  while (GetSizeOfASTNodeList(op_stack) != 0) {
    ReduceExprOp(expr_stack, op_stack);
  }

  if (GetSizeOfASTNodeList(expr_stack) == 0) {
    return NULL;
  }

  if (GetSizeOfASTNodeList(expr_stack) != 1) {
    PrintASTNodeList(expr_stack, 0);
    Error("parse expr failed.");
  }

  *after_index = index;
  return GetASTNodeAt(expr_stack, 0);
}

ASTNode *TryReadCompoundStatement(TokenList *tokens, int index,
                                  int *after_index);

ASTNode *TryReadExpressionStatement(TokenList *tokens, int index,
                                    int *after_index) {
  // expression-statement:
  //   expression ;
  ASTNode *expr = ReadExpression(tokens, index, &index);
  if (!expr || !IsEqualToken(GetTokenAt(tokens, index++), ";")) return NULL;
  ASTNode *expr_stmt = AllocateASTNode(kExprStmt);
  expr_stmt->data.expr_stmt.expr = expr;
  *after_index = index;
  return expr_stmt;
}

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
  ASTNode *for_stmt = AllocateASTNode(kForStatement);
  for_stmt->data.for_stmt.init_expr = init_expr;
  for_stmt->data.for_stmt.cond_expr = cond_expr;
  for_stmt->data.for_stmt.updt_expr = updt_expr;
  for_stmt->data.for_stmt.body_comp_stmt = body_comp_stmt;
  *after_index = index;
  return for_stmt;
}

ASTNode *TryReadReturnStmt(TokenList *tokens, int index, int *after_index) {
  const Token *token;
  token = GetTokenAt(tokens, index++);
  if (IsEqualToken(token, "return")) {
    // jump-statement(return)
    ASTNode *return_stmt = AllocateASTNode(kReturnStmt);
    ASTNode *expr_stmt = TryReadExpressionStatement(tokens, index, &index);
    return_stmt->data.return_stmt.expr_stmt = expr_stmt;
    *after_index = index;
    return return_stmt;
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
      (statement = TryReadForStatement(tokens, index, after_index)) ||
      (statement = TryReadExpressionStatement(tokens, index, after_index)))
    return statement;
  return NULL;
}

#define MAX_NUM_OF_STATEMENTS_IN_BLOCK 64
ASTNode *TryReadCompoundStatement(TokenList *tokens, int index,
                                  int *after_index) {
  // 6.8.2
  // compound-statement:
  //   { block-item-list(opt) }
  //
  // block-item:
  //   declaration
  //   statement
  ASTNode *comp_stmt = AllocateASTNode(kCompStmt);
  if (!IsEqualToken(GetTokenAt(tokens, index++), "{")) return NULL;
  //
  ASTNodeList *stmt_list = AllocateASTNodeList(MAX_NUM_OF_STATEMENTS_IN_BLOCK);
  ASTNode *stmt;
  while ((stmt = TryReadStatement(tokens, index, &index))) {
    PushASTNodeToList(stmt_list, stmt);
    putchar('\n');
  }
  comp_stmt->data.comp_stmt.stmt_list = stmt_list;
  //
  if (!IsEqualToken(GetTokenAt(tokens, index), "}")) {
    Error("Expected } but got %s", GetTokenAt(tokens, index)->str);
  }
  index++;
  //
  *after_index = index;
  return comp_stmt;
}

#define MAX_ROOT_NODES 64
#define MAX_ARGS 16
ASTNode *Parse(TokenList *tokens) {
  ASTNode *root = AllocateASTNode(kRoot);
  ASTNodeList *root_list = AllocateASTNodeList(MAX_ROOT_NODES);
  root->data.root.root_list = root_list;
  int index = 0;
  const Token *token;
  ASTNode *tmp_node;
  for (;;) {
    token = GetTokenAt(tokens, index);
    if (!token) break;
    if ((tmp_node = TryReadAsVarDef(tokens, index, &index))) {
      // 6.7.6.3
      // T D( parameter-type-list )
      // T D( identifier-list_opt )
      // func decl / def
      ASTNode *type_and_name = tmp_node;
      //
      token = GetTokenAt(tokens, index++);
      if (!IsEqualToken(token, "(")) {
        Error("Expected ( but got %s", token->str);
      }
      // args
      ASTNodeList *arg_list = AllocateASTNodeList(MAX_ARGS);
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
        Error("Expected ) but got %s", token->str);
      }
      //
      ASTNode *func_decl = AllocateASTNode(kFuncDecl);
      func_decl->data.func_decl.type_and_name = type_and_name;
      func_decl->data.func_decl.arg_list = arg_list;
      // ; if decl, { if def
      token = GetTokenAt(tokens, index);
      if (IsEqualToken(token, ";")) {
        // decl
        index++;
        PushASTNodeToList(root_list, func_decl);
      } else if (IsEqualToken(token, "{")) {
        // def
        ASTNode *func_def = AllocateASTNode(kFuncDef);
        ASTNode *comp_stmt = TryReadCompoundStatement(tokens, index, &index);
        if (!comp_stmt) {
          Error("comp_stmt is null");
        }
        func_def->data.func_def.func_decl = func_decl;
        func_def->data.func_def.comp_stmt = comp_stmt;
        PushASTNodeToList(root_list, func_def);

      } else {
        Error("Expected ; or { but got %s", token->str);
      }
    } else {
      Error("Unexpected token: %s", token->str);
    }
  }
  return root;
}
