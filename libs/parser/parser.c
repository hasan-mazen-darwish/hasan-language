#define _GNU_SOURCE
#include "./parser.h"
#include <stdlib.h>
#include <string.h>

#include <locale.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

Parser parser_new(Lexer *lexer) {
  Parser parser = {.lexer = lexer,
                   .nodes = NULL,
                   .nodes_size = lexer->lines,
                   .nodes_used = 0};

  parser.nodes = malloc(parser.nodes_size * sizeof(ParserNode));
  if (parser.nodes == NULL) {
    printf("Error allocating memory for the parser's nodes!\n");
    lexer_clean(lexer);
    exit(1);
    return parser;
  }

  return parser;
}

void parser_attach_lexer(Parser *parser, Lexer *lexer) {
  lexer_clean(parser->lexer);
  parser->lexer = lexer;
}

static void node_cleanup(ParserNode node) {
  switch (node.type) {
  case NODE_NUMBER_VARIABLE:
    node_cleanup(*node.content.numberVariable.value);
    free(node.content.numberVariable.value);
    break;
  case NODE_STRING_VARIABLE:
    node_cleanup(*node.content.stringVariable.value);
    free(node.content.stringVariable.value);
    break;
  case NODE_BOOLEAN_VARIABLE:
    node_cleanup(*node.content.booleanVariable.value);
    free(node.content.booleanVariable.value);
    break;
  case NODE_STRING_VALUE:
    free(node.content.string);
    break;
  default:
    break;
  }
}

void parser_clean(Parser *parser) {
  if (parser->lexer != NULL) {
    lexer_clean(parser->lexer);
    parser->lexer = NULL;
  }
  if (parser->nodes != NULL) {
    for (size_t i = 0; i < parser->nodes_used; i++) {
      node_cleanup(parser->nodes[i]);
    }
    free(parser->nodes);
    parser->nodes = NULL;
  }
}

//
//
//
// ==== HELPER FUNCTIONS ====
//
//
//

static int is_variable_keyword(Token *cursor) {
  return cursor->type == VARIABLE_NUMBER_KEYWORD ||
         cursor->type == VARIABLE_STRING_KEYWORD ||
         cursor->type == VARIABLE_BOOLEAN_KEYWORD;
}

static int add_node_to_parser(Parser *parser, ParserNode node) {
  // Returns 1 for success and 0 for failure.
  if (parser->nodes_used >= parser->nodes_size) {
    parser->nodes_size *= 2;
    ParserNode *temp =
        realloc(parser->nodes, parser->nodes_size * sizeof(ParserNode));
    if (temp == NULL) {
      printf("Error reallocating memory for new nodes in the parser!\n");
      parser_clean(parser);
      return 0;
    }
    parser->nodes = temp;
  }

  parser->nodes[parser->nodes_used] = node;
  parser->nodes_used++;

  return 1;
}

static int add_number_variable_value(Parser *parser, char *name, double value) {
  ParserNode boxedValueNode = {NODE_NUMBER_VALUE, {.number = value}};
  ParserNode *boxedValue = malloc(sizeof(ParserNode));
  if (boxedValue == NULL) {
    printf("Error allocating memory for a new number variable! (variable %s; "
           "value %f)\n",
           name, value);
    return 0;
  }
  boxedValue[0] = boxedValueNode;

  ParserNode node = {NODE_NUMBER_VARIABLE,
                     {.numberVariable = {name, boxedValue}}};
  return add_node_to_parser(parser, node);
}

static int add_string_variable_value(Parser *parser, char *name, char *value) {
  ParserNode boxedValueNode = {NODE_STRING_VALUE, {.string = strdup(value)}};
  ParserNode *boxedValue = malloc(sizeof(ParserNode));
  if (boxedValue == NULL) {
    printf("Error allocating memory for a new string variable! (variable %s; "
           "value %s)\n",
           name, value);
    return 0;
  }
  boxedValue[0] = boxedValueNode;

  ParserNode node = {NODE_STRING_VARIABLE,
                     {.stringVariable = {name, boxedValue}}};
  return add_node_to_parser(parser, node);
}

static int add_boolean_variable_value(Parser *parser, char *name, int value) {
  if (value != 0 && value != 1)
    return 0;
  ParserNode boxedValueNode = {value == 0 ? NODE_FALSE_VALUE : NODE_TRUE_VALUE,
                               {.boolean = value}};
  ParserNode *boxedValue = malloc(sizeof(ParserNode));
  if (boxedValue == NULL) {
    printf("Error allocating memory for a new boolean variable! (variable %s; "
           "value %s)\n",
           name, value <= 0 ? "false" : "true");
    return 0;
  }
  boxedValue[0] = boxedValueNode;

  ParserNode node = {NODE_BOOLEAN_VARIABLE,
                     {.booleanVariable = {name, boxedValue}}};
  return add_node_to_parser(parser, node);
}

static double my_strtod(const char *text) {
  locale_t c_locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
  return strtod_l(text, NULL, c_locale);
}

static int check_variable_type_to_data_type(TokensTypes variable_type,
                                            TokensTypes data_type) {
  return (variable_type == VARIABLE_NUMBER && data_type == DATA_NUMBER) ||
         (variable_type == VARIABLE_STRING && data_type == DATA_STRING) ||
         (variable_type == VARIABLE_BOOLEAN &&
          (data_type == DATA_NUMBER || data_type == KEYWORD_TRUE ||
           data_type == KEYWORD_FALSE));
}

// Handling other variables values, like operations and arithmetics and even
// other variables' values. Note: errors are going to be printed directly from
// this function. This fucntion only returns the status code.
static int add_number_variable_from_cursor(Parser *parser, char *varName,
                                           Token *cursor) {
  // Checking for empty assigning
  if (cursor->type == END_OF_LINE && (cursor - 1)->type == SYMBOL_EQUALS) {
    printf("Error parsing the source code: Attempting to assign no value to "
           "the number variable \"%s\" at %zu:%zu\n",
           varName, cursor->line, cursor->start);
    return 0;
  }

  ParserNode node = {NODE_NUMBER_VARIABLE};
  ParserNode *currentNode = NULL;

  // Additional flags:
  int is_negative = 0;
  int used_sign = 0;

  cursor--;
check_begin:
  cursor++;
  if (used_sign &&
      (cursor->type == SYMBOL_PLUS || cursor->type == SYMBOL_MINUS)) {
    printf("Error parsing the source code: Another symbol got used directly "
           "(cannot use %s and %s symbols like the way got used in the source "
           "code) at %zu:%zu.",
           (cursor - 1)->lexeme, cursor->lexeme, cursor->line, cursor->start);
    return 0;
  }
  used_sign = cursor->type == SYMBOL_MINUS || cursor->type == SYMBOL_PLUS;

  if (cursor->type == SYMBOL_MINUS) {
    is_negative = 1;
    goto check_begin;
  }

  if (cursor->type == DATA_NUMBER) {
    double value = my_strtod(cursor->lexeme);
    if (is_negative)
      value *= -1;
    ParserNode *n = malloc(sizeof(ParserNode));
    if (n == NULL) {
      printf("Error allocating memory for the new parser node for the number "
             "at %zu:%zu!\n",
             cursor->line, cursor->start);
      return 0;
    }
    n[0] = (ParserNode){NODE_NUMBER_VALUE, {.number = value}};
    currentNode = n;
  }

  // The end of the check: the current cursor got hit with the END_OF_LINE
  if (cursor->type != END_OF_LINE) {
    goto check_begin;
  }

  int returnValue = 0;
  if (currentNode != NULL) {
    node.content.numberVariable.identifier = varName;
    node.content.numberVariable.value = currentNode;
  }
  returnValue = add_node_to_parser(parser, node);
  return returnValue;
}

//
//
//

Parser *parser_parse(Parser *parser) {
  if (parser == NULL || parser->lexer == NULL ||
      parser->lexer->tokens == NULL) {
    printf("Error happened in the parsing: null source.\n");
    return NULL;
  }

  // Allocating memory for the nodes:
  if (parser->nodes == NULL || parser->nodes_size < parser->lexer->lines) {
    parser->nodes_size = parser->lexer->lines;
    parser->nodes = malloc(parser->nodes_size * sizeof(ParserNode));
    if (parser->nodes == NULL) {
      printf("Error allocating memory for the parser's nodes!\n");
      parser_clean(parser);
      return NULL;
    }
  }

  // Let's start with looping through the tokens:
  for (size_t i = 0; i < parser->lexer->lines; i++) {
    Token *line = parser->lexer->tokens[i];
    if (line == NULL)
      continue;
    Token *cursor = line;

    // Checking for the variables:
    if (is_variable_keyword(cursor)) {
      TokensTypes varType;
      switch (cursor->type) {
      case VARIABLE_NUMBER_KEYWORD:
        varType = VARIABLE_NUMBER;
        break;
      case VARIABLE_STRING_KEYWORD:
        varType = VARIABLE_STRING;
        break;
      case VARIABLE_BOOLEAN_KEYWORD:
        varType = VARIABLE_BOOLEAN;
        break;
      default:
        varType = UNKNOWN;
        break;
      }
      if (varType == UNKNOWN) {
        printf("Could not detect the variable type of the token \"%s\" at "
               "%zu:%zu.\n",
               (cursor + 1)->lexeme, cursor->line, cursor->start);
        parser_clean(parser);
        return NULL;
      }

      // We will deny any non-variable after the variable declaring keyword:
      if ((cursor + 1)->type != varType) {
        printf("The token that comes after \"%s\" should be a valid variable "
               "name%s. At %zu:%zu.\n",
               cursor->lexeme,
               (cursor + 1)->type == DATA_NUMBER
                   ? " (The variable should not begin with a number)"
                   : "",
               (cursor + 1)->line, (cursor + 1)->start);
        parser_clean(parser);
        return NULL;
      }

      char *varName = (cursor + 1)->lexeme;
      cursor += 2; // We are now in the token that's after the variable.
      if (cursor->type == END_OF_LINE) {
        // The variable will be assigned to the default value!
        int isVariableAdded = 0;
        if (varType == VARIABLE_NUMBER)
          isVariableAdded = add_number_variable_value(parser, varName, 0.0);
        else if (varType == VARIABLE_STRING)
          isVariableAdded = add_string_variable_value(parser, varName, "");
        else if (varType == VARIABLE_BOOLEAN)
          isVariableAdded = add_boolean_variable_value(parser, varName, 0);

        // Checking if the variable got added successfully or not:
        if (isVariableAdded == 0) {
          printf("There was an error declaring the variable %s at %zu:%zu to "
                 "the default value!\n",
                 varName, (cursor - 1)->line, (cursor - 1)->start);
          parser_clean(parser);
          return NULL;
        }
      }

      else if (cursor->type == SYMBOL_EQUALS) {
        cursor++;
        int i = 1;
        switch (varType) {
        case VARIABLE_NUMBER:
          i -= add_number_variable_from_cursor(parser, varName, cursor);
          break;
        case VARIABLE_STRING:
          break;
        case VARIABLE_BOOLEAN:
          break;

        default:
          printf("Fatal error parsing the source code: unknown variable \"%s\" "
                 "type at %zu:%zu.\n",
                 varName, cursor->line, cursor->start);
          parser_clean(parser);
          return NULL;
        }
        if (i == 1) {
          parser_clean(parser);
          return NULL;
        }
      }
    }
  }

  return parser;
}
