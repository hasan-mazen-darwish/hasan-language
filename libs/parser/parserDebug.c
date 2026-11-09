#include "./parserDebug.h"
#include "../utils/debug-tree.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DebugTree *tree;

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

// Helper functions for the print_number_variable function:
static void number_operation_print(ParserNode operation_node,
                                   int current_branches_number);

static void print_number_variable(ParserNode node,
                                  int initial_branches_number) {
  char text[1025];
  snprintf(text, 1025,
           "%sDefining a number variable called %s"
           "\"%s\"%s. The value attached to it is:\n",
           COLORS_YELLOW_BOLD, COLORS_RED_BOLD,
           node.content.numberVariable.identifier, COLORS_YELLOW_BOLD);

  unsigned int branches_number = initial_branches_number;
  dt_add_branch(tree, branches_number, text);

  // This loop will check all the nodes and then print them all together:
  ParserNode *n = malloc(sizeof(ParserNode));
  if (n == NULL) {
    printf("Error allocating memory for the initial temporary parser node!\n");
    exit(1);
  }
  printf(""); // Getting rid of the annoying C23 extension warning
  char *debug_text = "(No text defined for this node)";

  ParserNode cn = *node.content.numberVariable.value; // Stands for current node

  switch (cn.type) {
  case NODE_NUMBER_VALUE:
    debug_text = get_node_number_value(cn.content.number);
    break;
  case NODE_NUMBER_VARIABLE:
    print_number_variable(cn, branches_number + 1);
    branches_number++;
    return;
  case NODE_OPERATION:
    number_operation_print(cn, branches_number);
    break;

  default:
    break;
  }

  snprintf(text, 1025, "%s%s%s\n", COLORS_YELLOW_BOLD, debug_text,
           COLORS_RESET);
  dt_add_branch(tree, branches_number, text);
}

static void number_operation_print_switch_logic(ParserNode n,
                                                int current_branches_number) {
  char text[1025];
  switch (n.type) {
  case NODE_NUMBER_VARIABLE:
    print_number_variable(n, current_branches_number);
    break;
  case NODE_NUMBER_VALUE:
    printf(""); // Getting rid of the annoying warning
    snprintf(text, 1025, "%s%s\n", get_node_number_value(n.content.number),
             COLORS_RESET);
    dt_add_branch(tree, current_branches_number, text);
    break;
  case NODE_OPERATION:
    number_operation_print(n, current_branches_number);
    break;
  default:
    printf("");
    snprintf(text, 1025, "%sUnknown node type given! (not listed)\n%s",
             COLORS_YELLOW_BOLD, COLORS_RESET);
    dt_add_branch(tree, current_branches_number, text);
    break;
  }
}

static void number_operation_print(ParserNode operation_node,
                                   int current_branches_number) {
  char text[1025];
  char *operation =
      operation_node.content.operation.type == PARSER_OPERATION_ADDITION
          ? "addition"
          : (operation_node.content.operation.type == PARSER_OPERATION_DIVISION
                 ? "division"
                 : (operation_node.content.operation.type ==
                            PARSER_OPERATION_SUBTRACTION
                        ? "subtraction"
                        : (operation_node.content.operation.type ==
                                   PARSER_OPERATION_MULTIPLICATION
                               ? "multiplication"
                               : "unknown")));
  snprintf(text, 1025, "%sOperation of type %s\"%s\"%s. Sides are:\n%s",
           COLORS_YELLOW_BOLD, COLORS_RED_BOLD, COLORS_YELLOW_BOLD,
           COLORS_RESET, operation);
  dt_add_branch(tree, current_branches_number, text);

  snprintf(text, 1025, "%sOn the left side of the operation, we have: \n%s",
           COLORS_YELLOW_BOLD, COLORS_RESET);
  dt_add_branch(tree, current_branches_number, text);
  ParserNode n = *operation_node.content.operation.node1;
  number_operation_print_switch_logic(n, current_branches_number + 1);

  snprintf(text, 1025, "%sOn the right side of the operation, we have: \n%s",
           COLORS_YELLOW_BOLD, COLORS_RESET);
  dt_add_branch(tree, current_branches_number, text);
  n = *operation_node.content.operation.node2;
  number_operation_print_switch_logic(n, current_branches_number + 1);
}

void parser_debug(Parser *parser) {
  tree = malloc(sizeof(DebugTree));
  if (tree == NULL) {
    printf("Error allocating memory for the debugging tree of the parser!");
    parser_clean(parser);
    exit(1);
  }
  *tree = dt_get_default();
  for (size_t i = 0; i < parser->nodes_used; i++) {
    ParserNode node = parser->nodes[i];
    switch (node.type) {
    case NODE_NUMBER_VARIABLE:
      print_number_variable(node, 1);
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
