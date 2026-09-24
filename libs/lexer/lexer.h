#include <stdio.h>

typedef enum {
  // Variable Types
  VARIABLE_NUMBER,
  VARIABLE_NUMBER_KEYWORD
} TokensTypes;

typedef struct Token {
  TokensTypes type;
  char *lexeme;
  size_t start;
  size_t line;
} Token;

typedef struct Lexer {
  char *src;
  size_t srcLength;
  Token **tokens; // an array of arrays of tokens. Every line is going to be an
                  // array of tokens.
} Lexer;

Lexer *lexer_tokenify(Lexer *lexer);
