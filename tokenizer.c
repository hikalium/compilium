#include "compilium.h"

const char *Preprocess(TokenList *tokens, const char *p, const char *filename,
                       int *line);

typedef struct {
  const Token *left;
  const Token *right;
} TokenPair;

TokenPair replace_table[64];
int replace_table_used;

const Token *find_replacement(const Token *from) {
  for (int i = 0; i < replace_table_used; i++) {
    if (IsEqualToken(replace_table[i].left, from->str))
      return replace_table[i].right;
  }
  return from;
}

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

static const char *keyword_list[] = {
    "auto",       "break",    "case",     "char",   "const",   "continue",
    "default",    "do",       "double",   "else",   "enum",    "extern",
    "float",      "for",      "goto",     "if",     "inline",  "int",
    "long",       "register", "restrict", "return", "short",   "signed",
    "sizeof",     "static",   "struct",   "switch", "typedef", "union",
    "unsigned",   "void",     "volatile", "while",  "_Bool",   "_Complex",
    "_Imaginary", NULL};

int IsKeywordStr(const char *str) {
  for (int i = 0; keyword_list[i]; i++) {
    if (strcmp(str, keyword_list[i]) == 0) return 1;
  }
  return 0;
}

#define IS_IDENT_NODIGIT(c) \
  ((c) == '_' || ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z'))
#define IS_IDENT_DIGIT(c) (('0' <= (c) && (c) <= '9'))
const char *CommonTokenizer(TokenList *tokens, const char *p,
                            const char *filename, int *line) {
  static const char *single_char_punctuators = "[](){}~?:;,\\";
  const char *begin = NULL;
  if (IS_IDENT_NODIGIT(*p)) {
    begin = p++;
    while (IS_IDENT_NODIGIT(*p) || IS_IDENT_DIGIT(*p)) {
      p++;
    }
    Token *token =
        AllocTokenWithSubstring(begin, p, kIdentifier, filename, *line);
    if (IsKeywordStr(token->str)) token->type = kKeyword;
    AppendTokenToList(tokens, find_replacement(token));
    return p;
  }
  if (IS_IDENT_DIGIT(*p)) {
    begin = p++;
    while (IS_IDENT_DIGIT(*p)) {
      p++;
    }
    AppendTokenToList(
        tokens, AllocTokenWithSubstring(begin, p, kInteger, filename, *line));
    return p;
  }
  if (*p == '"' || *p == '\'') {
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
                                                      filename, *line));
    return p;
  }
  if (strchr(single_char_punctuators, *p)) {
    begin = p++;
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin, p, kPunctuator,
                                                      filename, *line));
    return p;
  }
  if (*p == '#') {
    p++;
    return Preprocess(tokens, p, filename, line);
  }
  if (*p == '/' && *(p + 1) == '*') {
    while (*p) {
      if (*p == '*' && *(p + 1) == '/') return p + 2;
      p++;
    }
    Error("*/ expected but got EOF");
  }
  if (*p == '|' || *p == '&' || *p == '+' || *p == '/') {
    // | || |=
    // & && &=
    // + ++ +=
    // / // /=
    begin = p++;
    if (*p == *begin || *p == '=') {
      p++;
      if (*begin == '/' && *begin == *(begin + 1)) {
        while (*p && *p != '\n') p++;
        return p;
      }
    }
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin, p, kPunctuator,
                                                      filename, *line));
    return p;
  }
  if (*p == '-') {
    // - -- -= ->
    begin = p++;
    if (*p == *begin || *p == '-' || *p == '=' || *p == '>') {
      p++;
    }
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin, p, kPunctuator,
                                                      filename, *line));
    return p;
  }
  if (*p == '=' || *p == '!' || *p == '*' || *p == '%' || *p == '^') {
    // = ==
    // ! !=
    // * *=
    // % %=
    // ^ ^=
    begin = p++;
    if (*p == '=') {
      p++;
    }
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin, p, kPunctuator,
                                                      filename, *line));
    return p;
  }
  if (*p == '<' || *p == '>') {
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
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin, p, kPunctuator,
                                                      filename, *line));
    return p;
  }
  if (*p == '.') {
    // .
    // ...
    begin = p++;
    if (p[0] == '.' && p[1] == '.') {
      p += 2;
    }
    AppendTokenToList(tokens, AllocTokenWithSubstring(begin, p, kPunctuator,
                                                      filename, *line));
    return p;
  }
  Error("Unexpected char '%c'\n", *p);
}

const char *Preprocess(TokenList *tokens, const char *p, const char *filename,
                       int *line) {
  int org_num_of_token = GetSizeOfTokenList(tokens);
  int dummy_line;
  do {
    if (*p == ' ' || *p == '\t') {
      p++;
    } else if (*p == '\n') {
      p++;
      (*line)++;
      break;
    } else if (*p == '\\') {
      // if "\\\n", continue parsing beyond the lines.
      // otherwise, raise Error.
      p++;
      if (*p != '\n') {
        Error("Unexpected char '%c'\n", *p);
      }
      p++;
      (*line)++;
    } else if (*p == '<') {
      const char *begin = p++;
      while (*p && *p != '>') {
        if (*p == '\\') p++;
        p++;
      }
      if (*(p++) != '>') {
        Error("Expected > but got char 0x%02X", *p);
      }
      AppendTokenToList(tokens,
                        AllocTokenWithSubstring(begin + 1, p - 1, kHeaderName,
                                                filename, *line));
    } else {
      p = CommonTokenizer(tokens, p, "(preprocess)", &dummy_line);
    }
  } while (*p);
  const Token *directive = GetTokenAt(tokens, org_num_of_token);
  if (IsEqualToken(directive, "include")) {
    const Token *file_name = GetTokenAt(tokens, org_num_of_token + 1);
    const char *file_name_str = file_name->str;
    if(file_name->type == kHeaderName){
      if(!include_path){
        Error("Include path not specified");
      }
      char *buf = calloc(1, 128);
      strncat(buf, include_path, 128);
      strncat(buf, "/", 128);
      strncat(buf, file_name->str, 128);
      file_name_str = buf;
    }
    char *input = ReadFile(file_name_str);
    SetSizeOfTokenList(tokens, org_num_of_token);
    Tokenize(tokens, input, file_name_str);
    return p;
  } else if (IsEqualToken(directive, "define")) {
    replace_table[replace_table_used].left =
        GetTokenAt(tokens, org_num_of_token + 1);
    replace_table[replace_table_used].right =
        GetTokenAt(tokens, org_num_of_token + 2);
    SetSizeOfTokenList(tokens, org_num_of_token);
    replace_table_used++;
    return p;
  }
  Error("Unknown preprocessor directive '%s'",
        directive ? directive->str : "(null)");
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
      p = CommonTokenizer(tokens, p, filename, &line);
    }
  } while (*p);
}

void TokenizeFile(TokenList *tokens, const char *filename) {
  Tokenize(tokens, ReadFile(filename), filename);
}
