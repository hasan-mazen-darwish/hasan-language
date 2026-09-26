#include <stdio.h>

typedef enum {
  // Variable Types
  VARIABLE_NUMBER,
  VARIABLE_NUMBER_KEYWORD,
  VARIABLE_STRING,
  VARIABLE_STRING_KEYWORD,

  // Built-in functions into the language
  FUNCTION_PRINT,

  // Ends
  END_OF_LINE,

  // Symbols
  SYMBOL_EQUALS,
  SYMBOL_PLUS,
  SYMBOL_LEFT_PARENTHESIS,
  SYMBOL_RIGHT_PARENTHESIS,

  // Keywords
  KEYWORD_WITH,

  // Generals
  VARIABLE
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
  size_t lines;
} Lexer;

Lexer *lexer_tokenify(Lexer *lexer);
