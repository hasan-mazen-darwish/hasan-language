#ifndef _PRATT_PARSING_H
#define _PRATT_PARSING_H

#include "./parser.h"

typedef enum {
  PRATT_PARAMETER_TYPE_ALLOWED,            // Nudded
  PRATT_PARAMETER_TYPE_UNARY,              // Nudded
  PRATT_PARAMETER_TYPE_EXPRESSION_STARTER, // Nudded and binding power evaluated
  PRATT_PARAMETER_TYPE_OPERATION,          // Ledded and binding power evaluated
  PRATT_PARAMETER_TYPE_TERMINATOR,         // Binding power evaluated
} PrattParameterType;

typedef ParserNode (*NudFunction)(Token token);
typedef ParserNode (*LedFunction)(ParserNode left, ParserNode right,
                                  Token token);
typedef ParserNode (*BindingPowerEvaluationFunction)(Token token);
typedef void (*ErrorFunction)(Token token);

typedef struct PrattParameter {
  PrattParameterType type;
  TokensTypes token_type;
  NudFunction nud;
  LedFunction led;
  int bp;
} PrattParameter;

ParserNode pratt_parsing_numbers(Token *cursor);
ParserNode pratt_parsing(Token *cursor, PrattParameter *parameters,
                         int parameters_number,
                         ErrorFunction nud_error_function,
                         ErrorFunction led_error_function,
                         ErrorFunction bp_error_func);

// Testing the general purposed pratt parsing:
ParserNode pratt_parsing_numbers_generalized(Token *cursor);

#endif // !_PRATT_PARSING_H
