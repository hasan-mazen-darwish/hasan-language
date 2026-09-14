#include <stdio.h>

typedef enum {
  TOKEN_LET,
  TOKEN_TYPE_FUNCTION,
  TOKEN_LEFT_PARENTHESES,
  TOKEN_RIGHT_PARANTHESES,
  TOKEN_TYPE_VARIABLE,
  TOKEN_FUNCTION_PRINT
} TokensTypes;


typedef struct Token {
  TokensTypes type;
} Token;

typedef struct Lexer {
  char *src;
  size_t srcLength;
} Lexer;
