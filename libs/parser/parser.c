#include "./parser.h"
#include <stdlib.h>

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
  if (node.firstSideType == NODE) {
    node_cleanup(*node.firstSide.node);
    free(node.firstSide.node);
  }

  if (node.secondSideType == NODE) {
    node_cleanup(*node.secondSide.node);
    free(node.secondSide.node);
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
  free(parser);
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

      cursor += 2; // We are now in the token that's after the variable.
      if (cursor->type == END_OF_LINE) {
      }
    }
  }

  return parser;
}
