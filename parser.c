#include "compilium.h"

int IsTypeToken(const Token *token) {
  return IsEqualToken(token, "int") || IsEqualToken(token, "char");
}

ASTNode *TryReadAsVarDef(int index, int *after_index) {
  ASTNode *var_def = AllocateASTNode(kVarDef);
  const Token *token;
  TokenList *token_list = AllocateTokenList();
  var_def->data.var_def.type_tokens = token_list;

  token = GetTokenAt(index);
  if (IsEqualToken(token, "const")) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(++index);
  }
  if (IsTypeToken(token)) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(++index);
  } else {
    return NULL;
  }
  while (IsEqualToken(token, "*")) {
    AppendTokenToList(token_list, token);
    token = GetTokenAt(++index);
  }
  var_def->data.var_def.name = GetTokenAt(index++);
  *after_index = index;
  return var_def;
}

TokenList *ReadExpression(int index, int *after_index) {
  // 6.5
  // a sequence of operators and operands
  // ... or that designates an object or a function
  const Token *token;
  TokenList *expression = AllocateTokenList();
  int paren_count = 0;
  if (IsKeyword(GetTokenAt(index))) return NULL;
  while ((token = GetTokenAt(index))) {
    if (IsEqualToken(token, "{") || IsEqualToken(token, "}")) return NULL;
    if (IsEqualToken(token, ";")) {
      break;
    } else {
      if (IsEqualToken(token, "(")) {
        paren_count++;
      } else if (IsEqualToken(token, ")")) {
        if (paren_count == 0) break;
        paren_count--;
      }
      AppendTokenToList(expression, token);
      index++;
    }
  }
  if (paren_count != 0) {
    Error("More ) needed before %s.", token ? token->str : "EOF");
  }
  *after_index = index;
  return expression;
}

ASTNode *TryReadCompoundStatement(int index, int *after_index);

ASTNode *TryReadExpressionStatement(int index, int *after_index) {
  // expression-statement:
  //   expression ;
  TokenList *expression = ReadExpression(index, &index);
  if (!expression || !IsEqualToken(GetTokenAt(index++), ";")) return NULL;
  ASTNode *expr_stmt = AllocateASTNode(kExprStmt);
  expr_stmt->data.expr_stmt.expr = expression;
  *after_index = index;
  return expr_stmt;
}

ASTNode *TryReadForStatement(int index, int *after_index) {
  // 6.8.5.3
  // for ( expression(opt) ; expression(opt) ; expression(opt) ) statement:

  if (!IsEqualToken(GetTokenAt(index++), "for")) return NULL;
  if (!IsEqualToken(GetTokenAt(index++), "(")) return NULL;
  TokenList *init_expression = ReadExpression(index, &index);
  if (!IsEqualToken(GetTokenAt(index++), ";")) return NULL;
  TokenList *cond_expression = ReadExpression(index, &index);
  if (!IsEqualToken(GetTokenAt(index++), ";")) return NULL;
  TokenList *updt_expression = ReadExpression(index, &index);
  if (!IsEqualToken(GetTokenAt(index++), ")")) return NULL;
  ASTNode *body_comp_stmt = TryReadCompoundStatement(index, &index);
  if(!body_comp_stmt){
    Error("TryReadForStatement: body_comp_stmt is null");
  }
  ASTNode *for_stmt = AllocateASTNode(kForStatement);
  for_stmt->data.for_stmt.init_expression = init_expression;
  for_stmt->data.for_stmt.cond_expression = cond_expression;
  for_stmt->data.for_stmt.updt_expression = updt_expression;
  for_stmt->data.for_stmt.body_comp_stmt = body_comp_stmt;
  *after_index = index;
  return for_stmt;
}

ASTNode *TryReadReturnStmt(int index, int *after_index) {
  const Token *token;
  token = GetTokenAt(index++);
  if (IsEqualToken(token, "return")) {
    // jump-statement(return)
    ASTNode *return_stmt = AllocateASTNode(kReturnStmt);
    ASTNode *expr_stmt = TryReadExpressionStatement(index, &index);
    return_stmt->data.return_stmt.expr_stmt = expr_stmt;
    *after_index = index;
    return return_stmt;
  }
  return NULL;
}

ASTNode *TryReadStatement(int index, int *after_index) {
  // 6.8
  // statement:
  //   labeled-statement
  //   compound-statement
  //   expression-statement
  //   selection-statement
  //   iteration-statement
  //   jump-statement
  ASTNode *statement;
  if ((statement = TryReadReturnStmt(index, after_index)) ||
      (statement = TryReadForStatement(index, after_index)) ||
      (statement = TryReadExpressionStatement(index, after_index)))
    return statement;
  return NULL;
}

ASTNode *TryReadCompoundStatement(int index, int *after_index) {
  // 6.8.2
  // compound-statement:
  //   { block-item-list(opt) }
  //
  // block-item:
  //   declaration
  //   statement
  ASTNode *comp_stmt = AllocateASTNode(kCompStmt);
  if (!IsEqualToken(GetTokenAt(index++), "{")) return NULL;
  //
  ASTNodeList *stmt_list = AllocateASTNodeList();
  ASTNode *stmt;
  while ((stmt = TryReadStatement(index, &index))) {
    AppendASTNodeToList(stmt_list, stmt);
    PrintASTNode(stmt, 0); putchar('\n');
  }
  comp_stmt->data.comp_stmt.stmt_list = stmt_list;
  //
  if (!IsEqualToken(GetTokenAt(index), "}")){
    Error("Expected } but got %s", GetTokenAt(index)->str);
  }
  index++;
  //
  *after_index = index;
  return comp_stmt;
}

ASTNode *Parse() {
  ASTNode *root = AllocateASTNode(kRoot);
  ASTNodeList *root_list = AllocateASTNodeList();
  root->data.root.root_list = root_list;
  int index = 0;
  const Token *token;
  ASTNode *tmp_node;
  for (;;) {
    token = GetTokenAt(index);
    if (!token) break;
    if ((tmp_node = TryReadAsVarDef(index, &index))) {
      // 6.7.6.3
      // T D( parameter-type-list )
      // T D( identifier-list_opt )
      // func decl / def
      ASTNode *type_and_name = tmp_node;
      //
      token = GetTokenAt(index++);
      if (!IsEqualToken(token, "(")) {
        Error("Expected ( but got %s", token->str);
      }
      // args
      ASTNodeList *arg_list = AllocateASTNodeList();
      while ((tmp_node = TryReadAsVarDef(index, &index))) {
        AppendASTNodeToList(arg_list, tmp_node);
        if (!IsEqualToken(GetTokenAt(index), ",")) {
          break;
        }
        index++;
      }
      //
      token = GetTokenAt(index++);
      if (!IsEqualToken(token, ")")) {
        Error("Expected ) but got %s", token->str);
      }
      //
      ASTNode *func_decl = AllocateASTNode(kFuncDecl);
      func_decl->data.func_decl.type_and_name = type_and_name;
      func_decl->data.func_decl.arg_list = arg_list;
      // ; if decl, { if def
      token = GetTokenAt(index);
      if (IsEqualToken(token, ";")) {
        // decl
        index++;
        AppendASTNodeToList(root_list, func_decl);
      } else if (IsEqualToken(token, "{")) {
        // def
        ASTNode *func_def = AllocateASTNode(kFuncDef);
        ASTNode *comp_stmt = TryReadCompoundStatement(index, &index);
        if(!comp_stmt){
          Error("comp_stmt is null");
        }
        func_def->data.func_def.func_decl = func_decl;
        func_def->data.func_def.comp_stmt = comp_stmt;
        AppendASTNodeToList(root_list, func_def);

      } else {
        Error("Expected ; or { but got %s", token->str);
      }
    } else {
      Error("Unexpected token: %s", token->str);
    }
  }
  return root;
}

