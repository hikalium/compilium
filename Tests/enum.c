typedef enum {                                              
  kIdentifier,                                              
  kStringLiteral,                             
  kCharacterLiteral,                                             
  kInteger,                                       
  kPunctuator,                                     
  kKeyword,                                            
} TokenType;

int main(){
  printf("kIdentifier = %d\n", kIdentifier);
  printf("kInteger = %d\n", kInteger);
  printf("kKeyword = %d\n", kKeyword);

  TokenType type;
  type = kPunctuator;
  printf("type = %d\n", type);

  return 0;
}
