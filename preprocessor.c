#include "compilium.h"

static struct Node *SkipDelimiterTokensInLogicalLine(struct Node *t) {
  while (t && t->token_type == kTokenDelimiter &&
         !IsEqualTokenWithCStr(t, "\n"))
    t = t->next_token;
  return t;
}

static const char *CreateStrFromTokenRange(struct Node *begin,
                                           struct Node *end) {
  assert(begin);
  int len = 0;
  for (struct Node *t = begin; t && t != end; t = t->next_token) {
    len += t->length;
  }
  return strndup(begin->begin, len);
}

static void PreprocessRemoveBlock(void) {
  for (struct Node *t = PeekToken(); t; t = t->next_token) {
    if (!IsEqualTokenWithCStr(t, "#")) {
      continue;
    }
    t = SkipDelimiterTokensInLogicalLine(t->next_token);
    if (!IsEqualTokenWithCStr(t, "endif") && !IsEqualTokenWithCStr(t, "else")) {
      continue;
    }
    RemoveTokensTo(t);
    break;
  }
}

static struct Node *TryReadIdentListWrappedByParens(struct Node **tp) {
  // If ( ident_list ) is read, this function returns cloned tokens of
  // ident_list without commas and tp is advanced to next token.
  // If not, this function returns NULL and tp is unchanged.
  struct Node *t = *tp;
  if (!IsEqualTokenWithCStr(t, "(")) {
    return NULL;
  }
  struct Node *ident_list_head = NULL;
  struct Node **ident_list_last_holder = &ident_list_head;
  for (t = SkipDelimiterTokensInLogicalLine(t->next_token); t;
       t = SkipDelimiterTokensInLogicalLine(t->next_token)) {
    if (IsEqualTokenWithCStr(t, ")")) break;
    *ident_list_last_holder = DuplicateToken(t);
    ident_list_last_holder = &(*ident_list_last_holder)->next_token;
    t = SkipDelimiterTokensInLogicalLine(t->next_token);
    if (!IsEqualTokenWithCStr(t, ",")) break;
  }
  if (!IsEqualTokenWithCStr(t, ")")) {
    return NULL;
  }
  // To distinguish function-like macro with zero args and
  // token level replacement macro, add ) at the end of args
  // to ensure args is not NULL
  *ident_list_last_holder = DuplicateToken(t);
  *tp = SkipDelimiterTokensInLogicalLine(t->next_token);
  return ident_list_head;
}

static char *CreateJoinedString(const char *s1, const char *s2) {
  assert(s1 && s2);
  char *s = malloc(strlen(s1) + strlen(s2) + 1);
  assert(s);
  strcpy(s, s1);
  strcat(s, s2);
  return s;
}

static void PreprocessBlock(struct Node *replacement_list, int level) {
  struct Node *t;
  while (PeekToken()) {
    if ((t = ConsumeTokenStr("__LINE__"))) {
      char s[32];
      snprintf(s, sizeof(s), "%d", t->line);
      t->token_type = kTokenIntegerConstant;
      t->begin = t->src_str = strdup(s);
      t->length = strlen(t->begin);
      continue;
    }
    if ((t = ReadToken(kTokenLineComment))) {
      while (t && !IsEqualTokenWithCStr(t, "\n")) t = t->next_token;
      RemoveTokensTo(t);
      continue;
    }
    if ((t = ReadToken(kTokenBlockCommentBegin))) {
      while (t && !IsTokenWithType(t, kTokenBlockCommentEnd)) t = t->next_token;
      if (IsTokenWithType(t, kTokenBlockCommentEnd)) t = t->next_token;
      RemoveTokensTo(t);
      continue;
    }
    if (IsEqualTokenWithCStr((t = PeekToken()), "#")) {
      assert(t);
      t = SkipDelimiterTokensInLogicalLine(t->next_token);
      if (IsEqualTokenWithCStr(t, "define")) {
        assert(t);
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        struct Node *from = t;
        t = t->next_token;
        struct Node *ident_list = TryReadIdentListWrappedByParens(&t);
        t = SkipDelimiterTokensInLogicalLine(t);
        assert(t);
        struct Node *to_token_head = NULL;
        struct Node **to_token_last_holder = &to_token_head;
        while (t && !IsEqualTokenWithCStr(t, "\n")) {
          *to_token_last_holder = DuplicateToken(t);
          to_token_last_holder = &(*to_token_last_holder)->next_token;
          t = t->next_token;
        }
        assert(IsEqualTokenWithCStr(t, "\n"));
        RemoveTokensTo(t->next_token);
        PushKeyValueToList(replacement_list, CreateTokenStr(from),
                           CreateMacroReplacement(ident_list, to_token_head));
        continue;
      }
      if (IsEqualTokenWithCStr(t, "include")) {
        struct Node *token_include = t;
        const char *fname = NULL;
        const char *path = NULL;
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        if (IsTokenWithType(t, kTokenStringLiteral)) {
          char *tmp_fname = CreateTokenStr(t);
          tmp_fname++;  // Remove open "
          tmp_fname[strlen(tmp_fname) - 1] = 0;  // Remove close "
          fname = tmp_fname;
          RemoveTokensTo(t->next_token);
          path = CreateJoinedString(
              "./", fname);  // TODO: Make this relative to source, not cwd.
        } else if (IsEqualTokenWithCStr(t, "<")) {
          struct Node *markL = t;
          t = t->next_token;
          struct Node *begin = t;
          while (t && !IsEqualTokenWithCStr(t, ">")) {
            t = t->next_token;
          }
          if (!t) {
            ErrorWithToken(markL,
                           "Unexpected EOF. > is expected to match with this.");
          }
          struct Node *end = t;
          fname = CreateStrFromTokenRange(begin, end);
          RemoveTokensTo(end->next_token);
          if (!include_path) {
            ErrorWithToken(token_include,
                           "Include path is not provided in compiler args");
          }
          path = CreateJoinedString(include_path, fname);
        } else {
          ErrorWithToken(t, "Expected < or \" here");
        }
        assert(path);
        fprintf(stderr, "Include from: %s\n", path);
        FILE *fp = fopen(path, "rb");
        if (!fp) {
          ErrorWithToken(token_include, "File not found: %s", path);
        }
        const char *include_input = ReadFile(fp);
        InsertTokens(Tokenize(include_input));
        fclose(fp);
        continue;
      }
      if (IsEqualTokenWithCStr(t, "ifdef")) {
        struct Node *ifdef_token = t;
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        struct Node *e;
        bool cond = (e = GetNodeByTokenKey(replacement_list, t));
        // defined
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        RemoveTokensTo(t);
        if (cond) {
          PreprocessBlock(replacement_list, level + 1);
          if (IsEqualTokenWithCStr(PeekToken(), "else")) {
            RemoveCurrentToken();
            PreprocessRemoveBlock();
          }
        } else {
          PreprocessRemoveBlock();
          if (IsEqualTokenWithCStr(PeekToken(), "else")) {
            RemoveCurrentToken();
            PreprocessBlock(replacement_list, level + 1);
          }
        }
        t = PeekToken();
        if (!IsEqualTokenWithCStr(t, "endif")) {
          ErrorWithToken(ifdef_token,
                         "Unexpected eof. Expected #endif to match with this.");
        }
        t = SkipDelimiterTokensInLogicalLine(t->next_token);
        RemoveTokensTo(t);
        continue;
      }
      if (IsEqualTokenWithCStr(t, "endif")) {
        if (level == 0) {
          ErrorWithToken(t, "Unexpected endif here");
        }
        RemoveTokensTo(t);
        return;
      }
      if (IsEqualTokenWithCStr(t, "else")) {
        if (level == 0) {
          ErrorWithToken(t, "Unexpected else here");
        }
        RemoveTokensTo(t);
        return;
      }
      ErrorWithToken(NextToken(), "Not a valid macro");
    }
    struct Node *e;
    if ((e = GetNodeByTokenKey(replacement_list, (t = PeekToken())))) {
      assert(e->type == kNodeMacroReplacement);
      struct Node *rep = DuplicateTokenSequence(e->value);
      RemoveCurrentToken();
      if (!e->arg_expr_list) {
        // ident replace macro case
        InsertTokens(rep);
        continue;
      }
      // function-like macro case
      t = SkipDelimiterTokensInLogicalLine(t->next_token);
      if (!IsEqualTokenWithCStr(t, "(")) ErrorWithToken(t, "Expected ( here");
      t = t->next_token;
      struct Node *it;
      struct Node *arg_rep_list = AllocList();
      for (it = e->arg_expr_list; it; it = it->next_token) {
        if (IsEqualTokenWithCStr(it, ")")) break;
        struct Node *arg_token_head = NULL;
        struct Node **arg_token_last_holder = &arg_token_head;
        t = SkipDelimiterTokensInLogicalLine(t);
        for (; t; t = t->next_token) {
          if (IsEqualTokenWithCStr(t, ")") || IsEqualTokenWithCStr(t, ","))
            break;
          *arg_token_last_holder = DuplicateToken(t);
          arg_token_last_holder = &(*arg_token_last_holder)->next_token;
        }
        PushKeyValueToList(arg_rep_list, CreateTokenStr(it),
                           CreateMacroReplacement(NULL, arg_token_head));
        if (IsEqualTokenWithCStr(t, ")")) break;
        t = t->next_token;
      }
      if (!IsEqualTokenWithCStr(t, ")")) ErrorWithToken(t, "Expected ) here");
      RemoveTokensTo(t->next_token);
      // Insert & replace args
      InsertTokensWithIdentReplace(rep, arg_rep_list);
      continue;
    }
    NextToken();
  }
}

void Preprocess(struct Node **head_holder, struct Node *replacement_list) {
  InitTokenStream(head_holder);
  PreprocessBlock(replacement_list, 0);
}
