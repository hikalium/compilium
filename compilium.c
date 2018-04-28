#include "compilium.h"

const char *ReadFile(const char *file_name) {
  // file_buf is allocated here.
  FILE *fp = fopen(file_name, "rb");
  if(!fp){
    Error("Failed to open: %s", file_name);
  }
  char *file_buf = malloc(MAX_INPUT_SIZE);
  int file_buf_size = fread(file_buf, 1, MAX_INPUT_SIZE, fp);
  if (file_buf_size >= MAX_INPUT_SIZE) {
    Error("Too large input");
  }
  file_buf[file_buf_size] = 0;
  printf("Input(path: %s, size: %d)\n", file_name, file_buf_size);
  fclose(fp);
  return file_buf;
}

const char *Preprocess(const char *p);
#define IS_IDENT_NODIGIT(c) \
  ((c) == '_' || ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z'))
#define IS_IDENT_DIGIT(c) (('0' <= (c) && (c) <= '9'))
const char *CommonTokenizer(const char *p)
{
  static const char *single_char_punctuators = "[](){}~?:;,\\";
  const char *begin = NULL;
  if (IS_IDENT_NODIGIT(*p)) {
    begin = p++;
    while (IS_IDENT_NODIGIT(*p) || IS_IDENT_DIGIT(*p)) {
      p++;
    }
    AddToken(begin, p, kIdentifier);
  } else if (IS_IDENT_DIGIT(*p)) {
    begin = p++;
    while (IS_IDENT_DIGIT(*p)) {
      p++;
    }
    AddToken(begin, p, kInteger);
  } else if (*p == '"' || *p == '\'') {
    begin = p++;
    while (*p && *p != *begin) {
      if (*p == '\\') p++;
      p++;
    }
    if (*(p++) != *begin) {
      Error("Expected %c but got char 0x%02X", *begin, *p);
    }
    TokenType type = (*begin == '"' ? kStringLiteral : kCharacterLiteral);
    AddToken(begin + 1, p - 1, type);
  } else if (strchr(single_char_punctuators, *p)) {
    // single character punctuator
    begin = p++;
    AddToken(begin, p, kPunctuator);
  } else if (*p == '#') {
    p++;
    p = Preprocess(p);
  } else if (*p == '|' || *p == '&' || *p == '+' || *p == '/') {
    // | || |=
    // & && &=
    // + ++ +=
    // / // /=
    begin = p++;
    if (*p == *begin || *p == '=') {
      p++;
    }
    AddToken(begin, p, kPunctuator);
  } else if (*p == '-') {
    // - -- -= ->
    begin = p++;
    if (*p == *begin || *p == '-' || *p == '>') {
      p++;
    }
    AddToken(begin, p, kPunctuator);
  } else if (*p == '=' || *p == '!' || *p == '*') {
    // = ==
    // ! !=
    // * *=
    begin = p++;
    if (*p == '=') {
      p++;
    }
    AddToken(begin, p, kPunctuator);
  } else if (*p == '<' || *p == '>') {
    // < << <= <<=
    // > >> >= >>=
    begin = p++;
    if (*p == *begin) {
      p++;
      if (*p == '=') {
        p++;
      }
    } else if (*p == '=') {
      p++;
    }
    AddToken(begin, p, kPunctuator);
  } else if (*p == '.') {
    // .
    // ...
    begin = p++;
    if (p[0] == '.' && p[1] == '.') {
      p += 2;
    }
    AddToken(begin, p, kPunctuator);
  } else {
    Error("Unexpected char '%c'\n", *p);
  }
  return p;
}

void Tokenize(const char *file_name);
const char *Preprocess(const char *p)
{
  int org_num_of_token = GetNumOfTokens();
  do {
    if (*p == ' ') {
      p++;
    } else if (*p == '\n') {
      p++;
      break;
    } else if(*p == '\\') {
      // if "\\\n", continue parsing beyond the lines.
      // otherwise, raise Error.
      p++;
      if(*p != '\n'){
        Error("Unexpected char '%c'\n", *p);
      }
      p++;
    } else {
      p = CommonTokenizer(p);
    }
  } while(*p);
  const Token *directive = GetTokenAt(org_num_of_token);
  if(IsEqualToken(directive, "include")){
    const Token *file_name = GetTokenAt(org_num_of_token + 1);
    if(!file_name || file_name->type != kStringLiteral){
      Error("Expected string literal but got %s", file_name ? file_name->str : "(null)");
    }
    SetNumOfTokens(org_num_of_token);
    Tokenize(file_name->str);
  } else{
    Error("Unknown preprocessor directive '%s'", directive ? directive->str : "(null)");
  }
  return p;
}

void Tokenize(const char *file_name) {
  const char *p = ReadFile(file_name);
  do {
    if (*p == ' ') {
      p++;
    } else if (*p == '\n') {
      p++;
      putchar('\n');
    } else {
      p = CommonTokenizer(p);
    }
  } while (*p);
}

ASTNode *AllocateASTNode(ASTType type) {
  ASTNode *node = malloc(sizeof(ASTNode));
  node->type = type;
  return node;
}

void PrintASTNodePadding(int depth) {
  putchar('\n');
  for (int i = 0; i < depth; i++) putchar(' ');
}

void PrintASTNodeList(ASTNodeList *list, int depth);
void PrintASTNode(const ASTNode *node, int depth) {
  if (node->type == kInclude) {
    printf("(Include:");
    PrintASTNodePadding(depth);
    PrintTokenList(node->data.directive_include.file_name_tokens);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kVarDef) {
    printf("(VarDef:");
    PrintASTNodePadding(depth);
    printf("type=");
    PrintTokenList(node->data.var_def.type_tokens);
    PrintASTNodePadding(depth);
    printf("name=%s", node->data.var_def.name->str);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDecl) {
    printf("(FuncDecl:");
    PrintASTNodePadding(depth);
    printf("type_and_name=");
    PrintASTNode(node->data.func_decl.type_and_name, depth + 1);
    PrintASTNodePadding(depth);
    printf("arg_list=");
    PrintASTNodeList(node->data.func_decl.arg_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kFuncDef) {
    printf("(FuncDef:");
    PrintASTNodePadding(depth);
    printf("func_decl=");
    PrintASTNode(node->data.func_def.func_decl, depth + 1);
    PrintASTNodePadding(depth);
    printf("comp_stmt=");
    PrintASTNode(node->data.func_def.comp_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kCompStatement) {
    printf("(CompStatement:");
    PrintASTNodePadding(depth);
    printf("(body=");
    PrintASTNodeList(node->data.comp_stmt.stmt_list, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kExpressionStatement) {
    printf("(ExpressionStatement:");
    PrintASTNodePadding(depth);
    printf("expression=");
    PrintTokenList(node->data.expression_stmt.expression);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kReturnStatement) {
    printf("(ReturnStatement:");
    PrintASTNodePadding(depth);
    printf("expression_stmt=");
    PrintASTNode(node->data.return_stmt.expression_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else if (node->type == kForStatement) {
    printf("(ForStatement:");
    PrintASTNodePadding(depth);
    printf("init_expression=");
    PrintTokenList(node->data.for_stmt.init_expression);
    PrintASTNodePadding(depth);
    printf("cond_expression=");
    PrintTokenList(node->data.for_stmt.cond_expression);
    PrintASTNodePadding(depth);
    printf("updt_expression=");
    PrintTokenList(node->data.for_stmt.updt_expression);
    PrintASTNodePadding(depth);
    printf("body_comp_stmt=");
    PrintASTNode(node->data.for_stmt.body_comp_stmt, depth + 1);
    PrintASTNodePadding(depth);
    printf(")");
  } else {
    Error("PrintASTNode not implemented for type %d", node->type);
  }
}

ASTNodeList *AllocateASTNodeList() {
  ASTNodeList *list = malloc(sizeof(ASTNodeList));
  list->used = 0;
  return list;
}

void AppendASTNodeToList(ASTNodeList *list, ASTNode *node) {
  if (list->used >= AST_NODE_LIST_SIZE) {
    Error("No more space in ASTNodeList");
  }
  list->nodes[list->used++] = node;
}

void PrintASTNodeList(ASTNodeList *list, int depth) {
  putchar('[');
  PrintASTNodePadding(depth);
  for (int i = 0; i < list->used; i++) {
    PrintASTNode(list->nodes[i], depth + 1);
    PrintASTNodePadding(depth);
  }
  putchar(']');
}

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
  ASTNode *expression_stmt = AllocateASTNode(kExpressionStatement);
  expression_stmt->data.expression_stmt.expression = expression;
  *after_index = index;
  return expression_stmt;
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
  ASTNode *for_stmt = AllocateASTNode(kForStatement);
  for_stmt->data.for_stmt.init_expression = init_expression;
  for_stmt->data.for_stmt.cond_expression = cond_expression;
  for_stmt->data.for_stmt.updt_expression = updt_expression;
  for_stmt->data.for_stmt.body_comp_stmt = body_comp_stmt;
  *after_index = index;
  return for_stmt;
}

ASTNode *TryReadReturnStatement(int index, int *after_index) {
  const Token *token;
  token = GetTokenAt(index++);
  if (IsEqualToken(token, "return")) {
    // jump-statement(return)
    ASTNode *return_stmt = AllocateASTNode(kReturnStatement);
    ASTNode *expression_stmt = TryReadExpressionStatement(index, &index);
    return_stmt->data.return_stmt.expression_stmt = expression_stmt;
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
  if ((statement = TryReadReturnStatement(index, after_index)) ||
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
  ASTNode *comp_stmt = AllocateASTNode(kCompStatement);
  if (!IsEqualToken(GetTokenAt(index++), "{")) return NULL;
  //
  ASTNodeList *stmt_list = AllocateASTNodeList();
  ASTNode *stmt;
  while ((stmt = TryReadStatement(index, &index))) {
    AppendASTNodeToList(stmt_list, stmt);
  }
  comp_stmt->data.comp_stmt.stmt_list = stmt_list;
  //
  if (!IsEqualToken(GetTokenAt(index++), "}")) return NULL;
  //
  *after_index = index;
  return comp_stmt;
}

ASTNodeList *Parse() {
  ASTNodeList *root = AllocateASTNodeList();
  int index = 0;
  const Token *token;
  ASTNode *tmp_node;
  for (;;) {
    token = GetTokenAt(index);
    if (!token) break;
    if (IsEqualToken(token, "\n")) {
      // skip
      index++;
    } else if (IsEqualToken(token, "#")) {
      index++;
      token = GetTokenAt(index++);
      if (IsEqualToken(token, "include")) {
        token = GetTokenAt(index++);
        ASTNode *node = AllocateASTNode(kInclude);
        if (IsEqualToken(token, "<")) {
          TokenList *tlist = AllocateTokenList();
          node->data.directive_include.file_name_tokens = tlist;
          while ((token = GetTokenAt(index++))) {
            if (IsEqualToken(token, ">")) break;
            AppendTokenToList(tlist, token);
          }
          if (!token) {
            Error("Expected > but got EOF");
          }
        } else {
          Error("Expected < but got %s", token->str);
        }
        AppendASTNodeToList(root, node);
      } else if (IsEqualToken(token, "define")) {
        ASTNode *node = AllocateASTNode(kInclude);
        TokenList *tlist = AllocateTokenList();
        node->data.directive_include.file_name_tokens = tlist;
        while ((token = GetTokenAt(index++))) {
          if (IsEqualToken(token, ">")) break;
          AppendTokenToList(tlist, token);
        }
        if (!token) {
          Error("Expected > but got EOF");
        }
      } else {
        Error("Unknown preprocessor directive: %s", token->str);
      }
    } else if ((tmp_node = TryReadAsVarDef(index, &index))) {
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
        AppendASTNodeToList(root, func_decl);
      } else if (IsEqualToken(token, "{")) {
        // def
        ASTNode *func_def = AllocateASTNode(kFuncDef);
        ASTNode *comp_stmt = TryReadCompoundStatement(index, &index);
        func_def->data.func_def.func_decl = func_decl;
        func_def->data.func_def.comp_stmt = comp_stmt;
        AppendASTNodeToList(root, func_def);

      } else {
        Error("Expected ; or { but got %s", token->str);
      }
    } else {
      Error("Unexpected token: %s", token->str);
    }
  }
  return root;
}

int main(int argc, char *argv[]) {
  if (argc < 2) return 1;

  Tokenize(argv[1]);
  ASTNodeList *ast = Parse();

  printf("ASTRoot: ");
  PrintASTNodeList(ast, 0);
  putchar('\n');

  return 0;
}
