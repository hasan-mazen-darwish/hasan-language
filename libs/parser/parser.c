#define _GNU_SOURCE
#include "./parser.h"
#include <stdlib.h>
#include <string.h>

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
  if (node.type == NODE_STRING_VARIABLE)
    free(node.content.stringVariable.value);
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

int add_node_to_parser(Parser *parser, ParserNode node) {
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

int add_number_variable(Parser *parser, char *name, double value) {
  ParserNode node = {NODE_NUMBER_VARIABLE, {.numberVariable = {name, value}}};
  return add_node_to_parser(parser, node);
}

int add_string_variable(Parser *parser, char *name, char *value) {
  ParserNode node = {NODE_STRING_VARIABLE,
                     {.stringVariable = {name, strdup(value)}}};
  return add_node_to_parser(parser, node);
}

int add_boolean_variable(Parser *parser, char *name, int value) {
  if (value != 0 && value != 1)
    return 0;
  ParserNode node = {NODE_BOOLEAN_VARIABLE, {.booleanVariable = {name, value}}};
  return add_node_to_parser(parser, node);
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
        // The variable was assigned to the default value!
        int isVariableAdded = 0;
        if (varType == VARIABLE_NUMBER)
          isVariableAdded = add_number_variable(parser, varName, 0.0);
        else if (varType == VARIABLE_STRING)
          isVariableAdded = add_string_variable(parser, varName, "");
        else if (varType == VARIABLE_BOOLEAN)
          isVariableAdded = add_boolean_variable(parser, varName, 0);

        // Checking if the variable got added successfully or not:
        if (isVariableAdded == 0) {
          printf("There was an error declaring the variable %s at %zu:%zu to "
                 "the default value!\n",
                 varName, (cursor - 1)->line, (cursor - 1)->start);
          parser_clean(parser);
          return NULL;
        }
      }
    }
  }

  return parser;
}
