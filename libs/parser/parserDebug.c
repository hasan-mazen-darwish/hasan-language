#include "./parserDebug.h"
#include <stdio.h>

#define COLORS_YELLOW_BOLD "\033[0;1;33m"
#define COLORS_RED_BOLD "\033[0;1;31m"
#define COLORS_RESET "\033[0m" // Colors resetting

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
    }
    printf("\n");
  }
}
