#include "compilium.h"

static struct Node *CreateNextToken(const char *p, const char *src) {
  while (*p <= ' ') {
    if (!*p) return NULL;
    p++;
  }
  if ('1' <= *p && *p <= '9') {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '9') {
      length++;
    }
    return AllocToken(src, p, length, kTokenDecimalNumber);
  } else if ('0' == *p) {
    int length = 0;
    while ('0' <= p[length] && p[length] <= '7') {
      length++;
    }
    return AllocToken(src, p, length, kTokenDecimalNumber);
  } else if (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') ||
             *p == '_') {
    int length = 0;
    while (('A' <= p[length] && p[length] <= 'Z') ||
           ('a' <= p[length] && p[length] <= 'z') || p[length] == '_' ||
           ('0' <= p[length] && p[length] <= '9')) {
      length++;
    }
    struct Node *t = AllocToken(src, p, length, kTokenIdent);
    if (IsEqualTokenWithCStr(t, "char")) t->type = kTokenKwChar;
    if (IsEqualTokenWithCStr(t, "for")) t->type = kTokenKwFor;
    if (IsEqualTokenWithCStr(t, "if")) t->type = kTokenKwIf;
    if (IsEqualTokenWithCStr(t, "int")) t->type = kTokenKwInt;
    if (IsEqualTokenWithCStr(t, "return")) t->type = kTokenKwReturn;
    if (IsEqualTokenWithCStr(t, "sizeof")) t->type = kTokenKwSizeof;
    if (IsEqualTokenWithCStr(t, "struct")) t->type = kTokenKwStruct;
    if (IsEqualTokenWithCStr(t, "void")) t->type = kTokenKwVoid;
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
    return AllocToken(src, p, length, kTokenCharLiteral);
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
    return AllocToken(src, p, length, kTokenStringLiteral);
  } else if ('&' == *p) {
    if (p[1] == '&') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('|' == *p) {
    if (p[1] == '|') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('<' == *p) {
    if (p[1] == '<') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    } else if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('>' == *p) {
    if (p[1] == '>') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    } else if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('=' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('!' == *p) {
    if (p[1] == '=') {
      return AllocToken(src, p, 2, kTokenPunctuator);
    }
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('^' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('+' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('-' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('*' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('/' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('%' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('~' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('?' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (':' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (',' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (';' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('{' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('}' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if ('(' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  } else if (')' == *p) {
    return AllocToken(src, p, 1, kTokenPunctuator);
  }
  Error("Unexpected char %c", *p);
}

struct Node *CreateToken(const char *input) {
  return CreateNextToken(input, input);
}

struct Node *Tokenize(const char *input) {
  const char *p = input;
  struct Node *t;
  struct Node *tokens = AllocList();
  while ((t = CreateNextToken(p, input))) {
    PushToList(tokens, t);
    p = t->begin + t->length;
  }
  return tokens;
}
