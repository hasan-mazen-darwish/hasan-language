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

static ParserNode expr_number(int gbp, Token *cursor);

static ParserNode nud_number(Token *cursor) {
  Token c = *cursor;
  switch (c.type) {
  case DATA_NUMBER:
    cursor++;
    return (ParserNode){NODE_NUMBER_VALUE, {.number = my_strtod(c.lexeme)}};
  case VARIABLE_NUMBER:
    cursor++;
    return (ParserNode){NODE_NUMBER_VARIABLE,
                        {.numberVariable = {c.lexeme, NULL}}};
  case SYMBOL_LEFT_PARENTHESIS:
    cursor++;
    ParserNode node = expr_number(0, cursor);
    if (cursor->type != SYMBOL_RIGHT_PARENTHESIS) {
      printf("Error parsing the source code: At %zu:%zu, it's expected to get "
             "\")\", but got \"%s\".\n",
             cursor->line, cursor->start, cursor->lexeme);
      exit(1);
    }
    cursor++;
    return node;
  default:
    printf("Error parsing the source code: Unable to null denotate the token "
           "\"%s\" at %zu:%zu.\n",
           cursor->lexeme, cursor->line, cursor->start);
    exit(1);
    break;
  }
}

static ParserNode led_number(ParserNode left, Token *cursor) {
  TokensTypes currType = cursor->type;
  cursor++;
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
    node2[0] = expr_number(10, cursor);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_SUBTRACTION, node1, node2}}};
  case SYMBOL_PLUS:
    node2[0] = expr_number(10, cursor);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_ADDITION, node1, node2}}};
  case SYMBOL_ASTERISK:
    node2[0] = expr_number(20, cursor);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_MULTIPLICATION, node1, node2}}};
  case SYMBOL_SLASH:
    node2[0] = expr_number(20, cursor);
    return (ParserNode){
        NODE_OPERATION,
        {.operation = {PARSER_OPERATION_DIVISION, node1, node2}}};
  default:
    printf("Error parsing the source code: Expected an operation at %zu:%zu, "
           "found \"%s\".\n",
           cursor->line, cursor->start, cursor->lexeme);
    exit(1);
  }
}

static int number_bp(Token *cursor) {
  switch (cursor->type) {
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
    printf("Error parsing the source code: Cannot evaluate the binding power "
           "(in number pratt parsing) of the token \"%s\" at %zu:%zu.\n",
           cursor->lexeme, cursor->line, cursor->start);
    exit(1);
  }
}

// gbp stands for Given Binding Power
static ParserNode expr_number(int gbp, Token *cursor) {
  ParserNode left = nud_number(cursor);
  while (gbp < number_bp(cursor))
    left = led_number(left, cursor);
  return left;
}

ParserNode pratt_parsing_numbers(Token *cursor) {
  return expr_number(0, cursor);
}
