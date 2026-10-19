#include "./parserDebug.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLORS_YELLOW_BOLD "\033[0;1;33m"
#define COLORS_RED_BOLD "\033[0;1;31m"
#define COLORS_RESET "\033[0m" // Colors resetting

static char *get_node_number_value(double number) {
  const char *format =
      COLORS_YELLOW_BOLD "A number value of " COLORS_RED_BOLD
                         "%f" COLORS_YELLOW_BOLD "." COLORS_RESET;
  size_t size = strlen(format) + 20;
  char *result = malloc(size * sizeof(char));
  if (result == NULL) {
    printf(
        COLORS_RESET
        "\n\nError allocating memory for the node number value debugging!\n\n");
    exit(1);
  }
  snprintf(result, size, format, number);
  return result;
}

static char *get_node_string_value(char *string) {
  const char *format =
      COLORS_YELLOW_BOLD "A number value of " COLORS_RED_BOLD
                         "%s" COLORS_YELLOW_BOLD "." COLORS_RESET;
  size_t size = strlen(format) + 20;
  char *result = malloc(size * sizeof(char));
  if (result == NULL) {
    printf(
        COLORS_RESET
        "\n\nError allocating memory for the node number value debugging!\n\n");
    exit(1);
  }
  snprintf(result, size, format, string);
  return result;
}

const char *get_node_true_value = COLORS_YELLOW_BOLD
    "A " COLORS_RED_BOLD "true" COLORS_YELLOW_BOLD " value." COLORS_RESET;
const char *get_node_false_value = COLORS_YELLOW_BOLD
    " " COLORS_RED_BOLD "false" COLORS_YELLOW_BOLD " value." COLORS_RESET;

//
//
// Variables debuggers:
//
//

void parser_debug(Parser *parser) {
  for (int i = 0; i < parser->nodes_used; i++) {
    ParserNode node = parser->nodes[i];
    switch (node.type) {
    case NODE_NUMBER_VARIABLE:
      printf(COLORS_YELLOW_BOLD
             "Defining a number variable named " COLORS_RED_BOLD
             "%s" COLORS_YELLOW_BOLD " with the value " COLORS_RED_BOLD
             "%f" COLORS_YELLOW_BOLD "." COLORS_RESET,
             node.content.numberVariable.identifier,
             // TODO: change this temporary glue
             node.content.numberVariable.value->content.number);
      break;

    case NODE_STRING_VARIABLE:
      printf(COLORS_YELLOW_BOLD
             "Defining a string variable named " COLORS_RED_BOLD
             "%s" COLORS_YELLOW_BOLD " with the value " COLORS_RED_BOLD
             "\"%s\"" COLORS_YELLOW_BOLD "." COLORS_RESET,
             node.content.stringVariable.identifier,
             // TODO: change this temporary glue
             node.content.stringVariable.value->content.string);
      break;

    case NODE_BOOLEAN_VARIABLE:
      printf(COLORS_YELLOW_BOLD
             "Defining a truth (boolean) variable named " COLORS_RED_BOLD
             "%s" COLORS_YELLOW_BOLD " with the value " COLORS_RED_BOLD
             "%s" COLORS_YELLOW_BOLD "." COLORS_RESET,
             node.content.booleanVariable.identifier,
             // TODO: change this temporary glue
             node.content.booleanVariable.value->content.boolean <= 0 ? "false"
                                                                      : "true");
      break;

    case NODE_NUMBER_VALUE:
      printf("%s\n", get_node_number_value(node.content.number));
      break;

    case NODE_STRING_VALUE:
      printf("%s\n", get_node_string_value(node.content.string));
      break;

    case NODE_TRUE_VALUE:
      printf("%s\n", get_node_true_value);
      break;

    case NODE_FALSE_VALUE:
      printf("%s\n", get_node_false_value);
      break;

    case NODE_OPERATION:
      printf("An operation got detected! (still debugging not handled)\n");
      break;
    }
    printf("\n");
  }
}
