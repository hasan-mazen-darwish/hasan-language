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
  switch (c.type) {
  case DATA_NUMBER:
    current++;
    return (ParserNode){NODE_NUMBER_VALUE, {.number = my_strtod(c.lexeme)}};
  case VARIABLE_NUMBER:
    current++;
    return (ParserNode){NODE_NUMBER_VARIABLE,
                        {.numberVariable = {c.lexeme, NULL}}};
  case SYMBOL_LEFT_PARENTHESIS:
    current++;
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
