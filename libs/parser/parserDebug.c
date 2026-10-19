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

void print_number_variable(ParserNode node) {
  printf(COLORS_YELLOW_BOLD "Defining a number variable called " COLORS_RED_BOLD
                            "\"%s\"" COLORS_YELLOW_BOLD
                            ". The value attached to it is:\n",
         node.content.numberVariable.identifier);

  unsigned int branches_number = 1, old_branches_number = 1;
  char *branches = malloc((branches_number + 1) * sizeof(char));
  if (branches == NULL) {
    printf("Error allocating memory for the parser debugger's first branch "
           "char!\n");
    exit(1);
  }
  strcpy(branches, "-");

  // This loop will check all the nodes and then print them all together:
  ParserNode *n = malloc(sizeof(ParserNode));
  if (n == NULL) {
    printf("Error allocating memory for the initial temporary parser node!\n");
    exit(1);
  }
  int need_to_loop_back = 0;

loop:
  if (branches_number != old_branches_number) {
    char *temp = realloc(branches, (branches_number + 1) * sizeof(char));
    if (temp == NULL) {
      printf("Error reallocating branches characters for a new branch!\n");
      free(branches);
      exit(1);
    }
    branches = temp;
    temp[branches_number - 1] = '-';
    temp[branches_number] = '\0';
    old_branches_number = branches_number;
  }

  char *debug_text = "(No text defined for this node)";

  ParserNode cn = *node.content.numberVariable.value; // Stands for current node

  switch (cn.type) {
  case NODE_NUMBER_VALUE:
    debug_text = get_node_number_value(cn.content.number);
    break;
  case NODE_OPERATION:
    printf(""); // To get rid from that annoying "Declaration after label is a
                // C23 extension" warning
    ParserOperation o = cn.content.operation.type;
    char *operation =
        o == PARSER_OPERATION_SUBTRACTION
            ? "subtraction"
            : (o == PARSER_OPERATION_ADDITION
                   ? "additrion"
                   : (o == PARSER_OPERATION_DIVISION ? "division"
                                                     : "multiplication"));
    snprintf(debug_text, 10240,
             "An operation of " COLORS_RED_BOLD "%s" COLORS_YELLOW_BOLD
             " that has these two nodes:",
             operation);
    branches_number++;
    need_to_loop_back = 1;
    break;

  default:
    break;
  }

  printf("\033[0;1;32m%s " COLORS_YELLOW_BOLD "%s" COLORS_RESET "\n", branches,
         debug_text);
  if (need_to_loop_back == 1)
    goto loop;
  free(branches);
}

void parser_debug(Parser *parser) {
  for (int i = 0; i < parser->nodes_used; i++) {
    ParserNode node = parser->nodes[i];
    switch (node.type) {
    case NODE_NUMBER_VARIABLE:
      print_number_variable(node);
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
