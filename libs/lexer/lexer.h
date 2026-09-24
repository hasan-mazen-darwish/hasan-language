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
  Token **tokens; // an array of arrays of tokens. Every line is going to be an array of tokens.
} Lexer;
