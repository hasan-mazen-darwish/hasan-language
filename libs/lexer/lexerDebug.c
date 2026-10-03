#include "./lexerDebug.h"
#include "lexer.h"
#include <stdio.h>

#define COLORS_LINE_NUMBERING "\033[0;1;4;33m" // Bold, underlined, yellow
#define COLORS_TOKEN "\033[0;1;31m"            // Bold, red
#define COLORS_TOKEN_LEXEME "\033[0;3;2;37m"   // Italic, dim, white
#define COLORS_RESET "\033[0m"                 // Resetting text state

static char *getTokenText(TokensTypes type) {
  switch (type) {
  // Variable types:
  case VARIABLE_NUMBER:
    return "VARIABLE_NUMBER";
  case VARIABLE_NUMBER_KEYWORD:
    return "VARIABLE_NUMBER_KEYWORD";
  case VARIABLE_STRING:
    return "VARIABLE_STRING";
  case VARIABLE_STRING_KEYWORD:
    return "VARIABLE_STRING_KEYWORD";

  // Data Types
  case DATA_NUMBER:
    return "DATA_NUMBER";
  case DATA_STRING:
    return "DATA_STRING";

  // Built-in function into the language:
  case FUNCTION_PRINT:
    return "FUNCTION_PRINT";

  // Ends
  case END_OF_LINE:
    return "END_OF_LINE";

  // Symbols
  case SYMBOL_EQUALS:
    return "SYMBOL_EQUALS";
  case SYMBOL_PLUS:
    return "SYMBOL_PLUS";
  case SYMBOL_LEFT_PARENTHESIS:
    return "SYMBOL_LEFT_PARENTHESIS";
  case SYMBOL_RIGHT_PARENTHESIS:
    return "SYMBOL_RIGHT_PARENTHESIS";
  case SYMBOL_MINUS:
    return "SYMBOL_MINUS";
  case SYMBOL_NOT:
    return "SYMBOL_NOT";
  case SYMBOL_ASTERISK:
    return "SYMBOL_ASTERISK";
  case SYMBOL_SLASH:
    return "SYMBOL_SLASH";
  case SYMBOL_RIGHT_CURLY_BRACKET:
    return "SYMBOL_RIGHT_CURLY_BRACKET";
  case SYMBOL_LEFT_CURLY_BRACKET:
    return "SYMBOL_RIGHT_CURLY_BRACKET";
  case SYMBOL_DOUBLE_QUOTES:
    return "SYMBOL_DOUBLE_QUOTES";

    // Multi-symbols operations
  case OPERATION_IS_EQUALS:
    return "OPERATION_IS_EQUALS";
  case OPERATION_ISNT_EQUALS:
    return "OPERATION_ISNT_EQUALS";
  case OPERATION_GREATER_THAN:
    return "OPERATION_GREATER_THAN";
  case OPERATION_SMALLER_THAN:
    return "OPERATION_SMALLER_THAN";
  case OPERATION_GREATER_OR_EQUALS_THAN:
    return "OPERATION_GREATER_OR_EQUALS_THAN";
  case OPERATION_SMALLER_OR_EQUALS_THAN:
    return "OPERATION_SMALLER_OR_EQUALS_THAN";
  case OPERATION_PLUS_PLUS:
    return "OPERATION_PLUS_PLUS";
  case OPERATION_MINUS_MINUS:
    return "OPERATION_MINUS_MINUS";
  case OPERATION_MINUS_EQUALS:
    return "OPERATION_MINUS_EQUALS";
  case OPERATION_PLUS_EQUALS:
    return "OPERATION_PLUS_EQUALS";
  case OPERATION_MULTIPLIES_EQUALS:
    return "OPERATION_MULTIPLIES_EQUALS";
  case OPERATION_DIVIDES_EQUALS:
    return "OPERATION_DIVIDES_EQUALS";

  // Keywords
  case KEYWORD_WITH:
    return "KEYWORD_WITH";

  // Generals
  case VARIABLE:
    return "VARIABLE";
  case COMMENT:
    return "COMMENT";
  case UNKNOWN:
    return "UNKNOWN";
  }
}

void lexer_debug_tokens(Lexer *lexer) {
  for (int i = 0; i < lexer->lines; i++) {
    if (lexer->tokens[i] == NULL) {
      printf("!!ERROR!! lexer->tokens[%d] is NULL.\n", i);
      break;
    }
    printf(COLORS_LINE_NUMBERING "%d." COLORS_RESET " ", i);
    TokensTypes type;
    Token *p = lexer->tokens[i];
    type = (*p).type;
    while (type != END_OF_LINE) {
      if (p == NULL) {
        printf("!!ERROR!! a token at line %d is NULL.\n", i);
        break;
      }
      if ((*p).type == END_OF_LINE)
        break;
      type = (*p).type;
      printf(COLORS_TOKEN "%s" COLORS_TOKEN_LEXEME "(%s)" COLORS_RESET ", ",
             getTokenText((*p).type), (*p).lexeme);
      p++;
    }
    printf(COLORS_TOKEN "%s" COLORS_TOKEN_LEXEME "(%s)" COLORS_RESET,
           getTokenText(END_OF_LINE), "");
    printf("\n");
  }
}
