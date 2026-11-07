#define _GNU_SOURCE
#include "./pratt.h"
#include "parser.h"
#include <bits/types/locale_t.h>
#include <locale.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

// Helper functions - Numbers
static double my_strtod(const char *text) {
  locale_t c_locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
  return strtod_l(text, NULL, c_locale);
}

static ParserNode expr_number(int gbp);

Token *current;

static ParserNode nud_number() {
  Token c = *current;
  current++;
  switch (c.type) {
  case SYMBOL_PLUS:
    printf(""); // Getting rid of the annoying warning
    ParserNode *zero_ = malloc(sizeof(ParserNode));
    if (zero_ == NULL) {
      printf("Unable to allocate memory for the zero number!\n");
      exit(1);
    }
    zero_[0] = (ParserNode){NODE_NUMBER_VALUE, {.number = 0}};

    ParserNode *expr_ = malloc(sizeof(ParserNode));
    if (expr_ == NULL) {
      printf("Unable to allocate memory for the expression after the unary "
             "minus!\n");
      exit(1);
    }
    expr_[0] = expr_number(10);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_ADDITION, zero_, expr_}}};
  case SYMBOL_MINUS:
    printf(""); // Getting rid of the annoying warning
    ParserNode *zero = malloc(sizeof(ParserNode));
    if (zero == NULL) {
      printf("Unable to allocate memory for the zero number!\n");
      exit(1);
    }
    zero[0] = (ParserNode){NODE_NUMBER_VALUE, {.number = 0}};

    ParserNode *expr = malloc(sizeof(ParserNode));
    if (expr == NULL) {
      printf("Unable to allocate memory for the expression after the unary "
             "minus!\n");
      exit(1);
    }
    expr[0] = expr_number(10);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_SUBTRACTION, zero, expr}}};
  case DATA_NUMBER:
    return (ParserNode){NODE_NUMBER_VALUE, {.number = my_strtod(c.lexeme)}};
  case VARIABLE_NUMBER:
    return (ParserNode){NODE_NUMBER_VARIABLE,
                        {.numberVariable = {c.lexeme, NULL}}};
  case SYMBOL_LEFT_PARENTHESIS:
    printf(""); // To get rid of the annoying warning
    ParserNode node = expr_number(0);
    if (current->type != SYMBOL_RIGHT_PARENTHESIS) {
      printf("Error parsing the source code: At %zu:%zu, it's expected to get "
             "\")\", but got \"%s\".\n",
             current->line, current->start, current->lexeme);
      exit(1);
    }
    current++;
    return node;
  default:
    printf("Error parsing the source code: Unexpected token "
           "\"%s\" at %zu:%zu.\n",
           current->lexeme, current->line, current->start);
    exit(1);
    break;
  }
}

static ParserNode led_number(ParserNode left) {
  TokensTypes currType = current->type;
  current++;
  ParserNode *node1 = malloc(sizeof(ParserNode));
  if (node1 == NULL) {
    printf("Unable to allocate memory for the left side of the subtraction "
           "operation!\n");
    exit(1);
  }
  node1[0] = left;
  ParserNode *node2 = malloc(sizeof(ParserNode));
  if (node2 == NULL) {
    printf("Unable to allocate memor for the right side of the subtraction "
           "operation!\n");
    exit(1);
  }

  switch (currType) {
  case SYMBOL_MINUS:
    node2[0] = expr_number(10);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_SUBTRACTION, node1, node2}}};
  case SYMBOL_PLUS:
    node2[0] = expr_number(10);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_ADDITION, node1, node2}}};
  case SYMBOL_ASTERISK:
    node2[0] = expr_number(20);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_MULTIPLICATION, node1, node2}}};
  case SYMBOL_SLASH:
    node2[0] = expr_number(20);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_DIVISION, node1, node2}}};
  default:
    printf("Error parsing the source code: Expected an operation at %zu:%zu, "
           "found \"%s\".\n",
           current->line, current->start, current->lexeme);
    exit(1);
  }
}

static int number_bp() {
  switch (current->type) {
  case SYMBOL_RIGHT_PARENTHESIS:
  case END_OF_LINE:
    return 0;
  case SYMBOL_MINUS:
  case SYMBOL_PLUS:
    return 10;
  case SYMBOL_ASTERISK:
  case SYMBOL_SLASH:
    return 20;
  default:
    printf(
        "Error parsing the source code: Unexpected token \"%s\" at %zu:%zu.\n",
        current->lexeme, current->line, current->start);
    exit(1);
  }
}

// gbp stands for Given Binding Power
static ParserNode expr_number(int gbp) {
  ParserNode left = nud_number();
  while (gbp < number_bp())
    left = led_number(left);
  return left;
}

ParserNode pratt_parsing_numbers(Token *cursor) {
  current = cursor;
  return expr_number(0);
}

Token *current_cursor = NULL;
PrattParameter *current_parameters = NULL;
int current_parameters_number = 0;

static ParserNode nud();
static ParserNode led(ParserNode left);
static int bp();
static ParserNode expr(int gbp);

ErrorFunction nud_e;
ErrorFunction led_e;
ErrorFunction bp_e;

ParserNode pratt_parsing(Token *cursor, PrattParameter *parameters,
                         int parameters_number, ErrorFunction nud_error_func,
                         ErrorFunction led_error_func,
                         ErrorFunction bp_error_func) {
  current_cursor = cursor;
  current_parameters = parameters;
  current_parameters_number = parameters_number;
  nud_e = nud_error_func;
  led_e = led_error_func;
  bp_e = bp_error_func;
  ParserNode node = expr(0);
  if (current_parameters != NULL) {
    free(current_parameters);
    current_parameters = NULL;
  }
  return node;
}

static ParserNode nud() {
  if (current_cursor == NULL) {
    printf("Error parsing the source code: No token given to parse! (parsing "
           "expressions)\n");
    exit(1);
  } else if (current_parameters == NULL) {
    printf("Error parsing the source code: The pratt parser has no rules to "
           "parse accoding to!\n");
    exit(1);
  }

  for (int i = 0; i < current_parameters_number; i++) {
    if (current_parameters[i].token_type == current_cursor->type &&
        (current_parameters[i].type == PRATT_PARAMETER_TYPE_UNARY ||
         current_parameters[i].type == PRATT_PARAMETER_TYPE_ALLOWED)) {
      Token c = *current_cursor;
      current_cursor++;
      return current_parameters[i].nud(c);
    }

    else if (current_parameters[i].token_type == current_cursor->type &&
             current_parameters[i].type ==
                 PRATT_PARAMETER_TYPE_EXPRESSION_STARTER) {
      current_cursor++;
      return expr(0);
    }
  }

  nud_e(*current_cursor);
  exit(1);
}

static ParserNode led(ParserNode left) {
  if (current_cursor == NULL) {
    printf("Error parsing the source code: No token given to parse! (parsing "
           "expressions)\n");
    exit(1);
  } else if (current_parameters == NULL) {
    printf("Error parsing the source code: The pratt parser has no rules to "
           "parse accoding to!\n");
    exit(1);
  }

  for (int i = 0; i < current_parameters_number; i++) {
    if (current_parameters[i].token_type == current_cursor->type &&
        current_parameters[i].type == PRATT_PARAMETER_TYPE_OPERATION) {
      Token c = *current_cursor;
      current_cursor++;
      ParserNode right = expr(current_parameters[i].bp);
      return current_parameters[i].led(left, right, c);
    }
  }

  led_e(*current_cursor);
  exit(1);
}

static int bp() {
  if (current_cursor == NULL) {
    printf("Error parsing the source code: No token given to parse! (parsing "
           "expressions)\n");
    exit(1);
  } else if (current_parameters == NULL) {
    printf("Error parsing the source code: The pratt parser has no rules to "
           "parse accoding to!\n");
    exit(1);
  }

  if (current_cursor->type == END_OF_LINE)
    return 0;

  for (int i = 0; i < current_parameters_number; i++) {
    if (current_cursor->type == current_parameters[i].token_type) {
      if (current_parameters[i].type ==
              PRATT_PARAMETER_TYPE_EXPRESSION_STARTER ||
          current_parameters[i].type == PRATT_PARAMETER_TYPE_TERMINATOR)
        return 0;
      else if (current_parameters[i].type == PRATT_PARAMETER_TYPE_OPERATION)
        return current_parameters[i].bp;
    }
  }

  bp_e(*current_cursor);
  exit(1);
}

static ParserNode expr(int gbp) {
  ParserNode left = nud();
  while (gbp < bp())
    left = led(left);
  return left;
}

// Parsers:

// Numbers Pratt Parser:
// --- Error Functions ---
void number_nud_error_function(Token token) {
  printf("Error parsing the source code: Unexpected token "
         "\"%s\" at %zu:%zu.\n",
         token.lexeme, token.line, token.start);
}

void number_led_error_function(Token token) {
  printf("Error parsing the source code: Expected an operation at %zu:%zu, "
         "found \"%s\".\n",
         token.line, token.start, token.lexeme);
}

void number_bp_error_function(Token token) {
  printf("Error parsing the source code: Unexpected token \"%s\" at %zu:%zu.\n",
         token.lexeme, token.line, token.start);
}

// --- Nudders ---
ParserNode n_number_nud(Token token) {
  return (ParserNode){NODE_NUMBER_VALUE, {.number = my_strtod(token.lexeme)}};
}

ParserNode n_number_variable_nud(Token token) {
  return (ParserNode){NODE_NUMBER_VARIABLE,
                      {.numberVariable = {token.lexeme, NULL}}};
}

ParserNode n_unary_nud(Token token) {
  ParserNode *zero = malloc(sizeof(ParserNode));
  if (zero == NULL) {
    printf("Error allocating memory for the zero node!\n");
    exit(1);
  }
  *zero = (ParserNode){NODE_NUMBER_VALUE, {.number = 0}};

  ParserNode *expression = malloc(sizeof(ParserNode));
  if (expression == NULL) {
    printf("Error allocating memory for the expression node!\n");
    exit(1);
  }
  *expression = expr(10);
  return (ParserNode){
      NODE_OPERATION,
      {.operation = {token.type == SYMBOL_MINUS ? PARSER_OPERATION_SUBTRACTION
                                                : PARSER_OPERATION_ADDITION,
                     zero, expression}}};
}

// --- Ledders ---
ParserNode n_operations_led(ParserNode left, ParserNode right, Token token) {
  ParserNode *l = malloc(sizeof(ParserNode));
  if (l == NULL) {
    printf("Error allocating memory for the left side of the operation %s !",
           token.lexeme);
    exit(1);
  }
  *l = left;

  ParserNode *r = malloc(sizeof(ParserNode));
  if (r == NULL) {
    printf("Error allocating memory for the right side of the operation %s !",
           token.lexeme);
    exit(1);
  }
  *r = right;

  return (ParserNode){
      NODE_OPERATION,
      {.operation = {token.type == SYMBOL_MINUS
                         ? PARSER_OPERATION_SUBTRACTION
                         : (token.type == SYMBOL_PLUS
                                ? PARSER_OPERATION_ADDITION
                                : (token.type == SYMBOL_ASTERISK
                                       ? PARSER_OPERATION_MULTIPLICATION
                                       : PARSER_OPERATION_DIVISION)),
                     l, r}}};
}

// --- Main Function ---
ParserNode pratt_parsing_numbers_generalized(Token *cursor) {
  int parameters_number = 10;
  PrattParameter *parameters =
      malloc(parameters_number * sizeof(PrattParameter));
  if (parameters == NULL) {
    printf("Failed to allocate memory for the Pratt parser's parameters! (for "
           "number expressions)\n");
    exit(1);
  }

  // The brackets:
  parameters[0] = (PrattParameter){PRATT_PARAMETER_TYPE_EXPRESSION_STARTER,
                                   SYMBOL_LEFT_PARENTHESIS, NULL, NULL, 0};
  parameters[1] = (PrattParameter){PRATT_PARAMETER_TYPE_TERMINATOR,
                                   SYMBOL_RIGHT_PARENTHESIS, NULL, NULL, 0};

  // The numbers:
  parameters[2] = (PrattParameter){PRATT_PARAMETER_TYPE_ALLOWED, DATA_NUMBER,
                                   &n_number_nud, NULL, 0};
  parameters[3] =
      (PrattParameter){PRATT_PARAMETER_TYPE_ALLOWED, VARIABLE_NUMBER,
                       &n_number_variable_nud, NULL, 0};

  // Unary:
  parameters[4] = (PrattParameter){PRATT_PARAMETER_TYPE_UNARY, SYMBOL_MINUS,
                                   &n_unary_nud, NULL, 10};
  parameters[5] = (PrattParameter){PRATT_PARAMETER_TYPE_UNARY, SYMBOL_PLUS,
                                   &n_unary_nud, NULL, 10};

  // Operations
  parameters[6] = (PrattParameter){PRATT_PARAMETER_TYPE_OPERATION, SYMBOL_MINUS,
                                   NULL, &n_operations_led, 10};
  parameters[7] = (PrattParameter){PRATT_PARAMETER_TYPE_OPERATION, SYMBOL_PLUS,
                                   NULL, &n_operations_led, 10};
  parameters[8] =
      (PrattParameter){PRATT_PARAMETER_TYPE_OPERATION, SYMBOL_ASTERISK, NULL,
                       &n_operations_led, 20};
  parameters[9] = (PrattParameter){PRATT_PARAMETER_TYPE_OPERATION, SYMBOL_SLASH,
                                   NULL, &n_operations_led, 20};

  return pratt_parsing(cursor, parameters, parameters_number,
                       &number_nud_error_function, &number_led_error_function,
                       &number_bp_error_function);
}
