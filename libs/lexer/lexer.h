#ifndef _LEXER_H
#define _LEXER_H
#include <stdio.h>

typedef enum {
  // Variable Types
  VARIABLE_NUMBER,
  VARIABLE_NUMBER_KEYWORD,
  VARIABLE_STRING,
  VARIABLE_STRING_KEYWORD,
  VARIABLE_BOOLEAN,
  VARIABLE_BOOLEAN_KEYWORD,

  // Data Types
  DATA_NUMBER,
  DATA_STRING,

  // Built-in functions into the language
  FUNCTION_PRINT,

  // Ends
  END_OF_LINE,

  // Symbols
  SYMBOL_EQUALS,
  SYMBOL_PLUS,
  SYMBOL_LEFT_PARENTHESIS,
  SYMBOL_RIGHT_PARENTHESIS,
  SYMBOL_MINUS,
  SYMBOL_NOT,
  SYMBOL_ASTERISK,
  SYMBOL_SLASH,
  SYMBOL_RIGHT_CURLY_BRACKET,
  SYMBOL_LEFT_CURLY_BRACKET,
  SYMBOL_DOUBLE_QUOTES,

  // Multi-symbols operations
  OPERATION_IS_EQUALS,
  OPERATION_ISNT_EQUALS,
  OPERATION_GREATER_THAN,
  OPERATION_SMALLER_THAN,
  OPERATION_GREATER_OR_EQUALS_THAN,
  OPERATION_SMALLER_OR_EQUALS_THAN,
  OPERATION_PLUS_PLUS,
  OPERATION_MINUS_MINUS,
  OPERATION_MINUS_EQUALS,
  OPERATION_PLUS_EQUALS,
  OPERATION_MULTIPLIES_EQUALS,
  OPERATION_DIVIDES_EQUALS,
  OPERATION_AND,
  OPERATION_BITWISE_AND,
  OPERATION_OR,
  OPERATION_BITWISE_OR,

  // Keywords
  KEYWORD_WITH,
  KEYWORD_AND,
  KEYWORD_IF,
  KEYWORD_ELSE,
  KEYWORD_SWITCH,
  KEYWORD_CASE,
  KEYWORD_DEFAULT,
  KEYWORD_TRUE,
  KEYWORD_FALSE,

  // Generals
  VARIABLE,
  COMMENT,
  UNKNOWN
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
void lexer_clean(Lexer *lexer);
#endif // _LEXER_H
