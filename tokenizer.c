#include "compilium.h"

const char *Preprocess(TokenList *tokens, const char *p);

char *ReadFile(const char *file_name) {
  // file_buf is allocated here.
  FILE *fp = fopen(file_name, "rb");
  if (!fp && strcmp(file_name, "-") == 0) {
    fp = stdin;
  }
  if (!fp) {
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

#define IS_IDENT_NODIGIT(c) \
  ((c) == '_' || ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z'))
#define IS_IDENT_DIGIT(c) (('0' <= (c) && (c) <= '9'))
const char *CommonTokenizer(TokenList *tokens, const char *p,
                            const char *filename, int line) {
  static const char *single_char_punctuators = "[](){}~?:;,%\\";
  const char *begin = NULL;
  if (IS_IDENT_NODIGIT(*p)) {
    begin = p++;
    while (IS_IDENT_NODIGIT(*p) || IS_IDENT_DIGIT(*p)) {
      p++;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kIdentifier, filename, line));
  } else if (IS_IDENT_DIGIT(*p)) {
    begin = p++;
    while (IS_IDENT_DIGIT(*p)) {
      p++;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kInteger, filename, line));
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
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin + 1, p - 1, type,
                                                      filename, line));
  } else if (strchr(single_char_punctuators, *p)) {
    // single character punctuator
    begin = p++;
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kPunctuator, filename, line));
  } else if (*p == '#') {
    p++;
    p = Preprocess(tokens, p);
  } else if (*p == '|' || *p == '&' || *p == '+' || *p == '/') {
    // | || |=
    // & && &=
    // + ++ +=
    // / // /=
    begin = p++;
    if (*p == *begin || *p == '=') {
      p++;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kPunctuator, filename, line));
  } else if (*p == '-') {
    // - -- -= ->
    begin = p++;
    if (*p == *begin || *p == '-' || *p == '>') {
      p++;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kPunctuator, filename, line));
  } else if (*p == '=' || *p == '!' || *p == '*') {
    // = ==
    // ! !=
    // * *=
    begin = p++;
    if (*p == '=') {
      p++;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kPunctuator, filename, line));
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
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kPunctuator, filename, line));
  } else if (*p == '.') {
    // .
    // ...
    begin = p++;
    if (p[0] == '.' && p[1] == '.') {
      p += 2;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kPunctuator, filename, line));
  } else {
    Error("Unexpected char '%c'\n", *p);
  }
  return p;
}

const char *Preprocess(TokenList *tokens, const char *p) {
  int org_num_of_token = GetSizeOfTokenList(tokens);
  do {
    if (*p == ' ' || *p == '\t') {
      p++;
    } else if (*p == '\n') {
      p++;
      break;
    } else if (*p == '\\') {
      // if "\\\n", continue parsing beyond the lines.
      // otherwise, raise Error.
      p++;
      if (*p != '\n') {
        Error("Unexpected char '%c'\n", *p);
      }
      p++;
    } else {
      p = CommonTokenizer(tokens, p, "(preprocess)", 1);
    }
  } while (*p);
  const Token *directive = GetTokenAt(tokens, org_num_of_token);
  if (IsEqualToken(directive, "include")) {
    Error("#include not implemented");
  } else {
    Error("Unknown preprocessor directive '%s'",
          directive ? directive->str : "(null)");
  }
  return p;
}

void Tokenize(TokenList *tokens, const char *p, const char *filename) {
  int line = 1;
  do {
    if (*p == ' ' || *p == '\t') {
      p++;
    } else if (*p == '\n') {
      p++;
      line++;
    } else {
      p = CommonTokenizer(tokens, p, filename, line);
    }
  } while (*p);
}
