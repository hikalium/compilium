#include "compilium.h"

static struct Node *CreateNextToken(const char *p, const char *src, int *line) {
  assert(line);
  while (*p <= ' ') {
    if (!*p) return NULL;
    if (*p == '\n') (*line)++;
    p++;
  }
  if ('1' <= *p && *p <= '9') {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '9') {
      length++;
    }
    return AllocToken(src, *line, p, length, kTokenDecimalNumber);
  } else if ('0' == *p) {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '7') {
      length++;
    }
    return AllocToken(src, *line, p, length, kTokenOctalNumber);
  } else if (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') ||
             *p == '_') {
    int length = 0;
    while (('A' <= p[length] && p[length] <= 'Z') ||
           ('a' <= p[length] && p[length] <= 'z') || p[length] == '_' ||
           ('0' <= p[length] && p[length] <= '9')) {
      length++;
    }
    struct Node *t = AllocToken(src, *line, p, length, kTokenIdent);
    if (IsEqualTokenWithCStr(t, "char")) t->token_type = kTokenKwChar;
    if (IsEqualTokenWithCStr(t, "else")) t->token_type = kTokenKwElse;
    if (IsEqualTokenWithCStr(t, "for")) t->token_type = kTokenKwFor;
    if (IsEqualTokenWithCStr(t, "if")) t->token_type = kTokenKwIf;
    if (IsEqualTokenWithCStr(t, "int")) t->token_type = kTokenKwInt;
    if (IsEqualTokenWithCStr(t, "return")) t->token_type = kTokenKwReturn;
    if (IsEqualTokenWithCStr(t, "sizeof")) t->token_type = kTokenKwSizeof;
    if (IsEqualTokenWithCStr(t, "struct")) t->token_type = kTokenKwStruct;
    if (IsEqualTokenWithCStr(t, "void")) t->token_type = kTokenKwVoid;
    return t;
  } else if ('\'' == *p) {
    int length = 1;
    while (p[length] && p[length] != '\'') {
      if (p[length] == '\\' && p[length + 1]) {
        length++;
      }
      length++;
    }
    if (p[length] != '\'') {
      Error("Expected end of char literal (')");
    }
    length++;
    return AllocToken(src, *line, p, length, kTokenCharLiteral);
  } else if ('"' == *p) {
    int length = 1;
    while (p[length] && p[length] != '"') {
      if (p[length] == '\\' && p[length + 1]) {
        length++;
      }
      length++;
    }
    if (p[length] != '"') {
      Error("Expected end of string literal (\")");
    }
    length++;
    return AllocToken(src, *line, p, length, kTokenStringLiteral);
  } else if ('&' == *p) {
    if (p[1] == '&') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('|' == *p) {
    if (p[1] == '|') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('<' == *p) {
    if (p[1] == '<') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    } else if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('>' == *p) {
    if (p[1] == '>') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    } else if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('=' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('!' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('^' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('+' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('-' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('*' == *p) {
    if (p[1] == '/') {
      return AllocToken(src, *line, p, 2, kTokenBlockCommentEnd);
    }
    if (p[1] == '=') {
      return AllocToken(src, *line, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('/' == *p) {
    if (p[1] == '/') {
      return AllocToken(src, *line, p, 2, kTokenLineComment);
    }
    if (p[1] == '*') {
      return AllocToken(src, *line, p, 2, kTokenBlockCommentBegin);
    }
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('%' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('~' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('?' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if (':' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if (',' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if (';' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('{' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('}' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if ('(' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  } else if (')' == *p) {
    return AllocToken(src, *line, p, 1, kTokenPunctuator);
  }
  Error("Unexpected char %c", *p);
}

struct Node *CreateToken(const char *input) {
  int line = 1;
  return CreateNextToken(input, input, &line);
}

struct Node *Tokenize(const char *input) {
  // returns head of tokens.
  struct Node *token_head = NULL;
  struct Node **last_next_token = &token_head;
  const char *p = input;
  struct Node *t;
  int line = 1;
  while ((t = CreateNextToken(p, input, &line))) {
    *last_next_token = t;
    last_next_token = &t->next_token;
    p = t->begin + t->length;
  }
  return token_head;
}
