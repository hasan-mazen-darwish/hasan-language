#include "./parser.h"
#include <stdlib.h>

Parser parser_new(Lexer *lexer) {
  Parser parser = {.lexer = lexer, .nodes = NULL, .nodes_size = 0};
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
  lexer_clean(parser->lexer);
  if (parser->nodes != NULL) {
    for (size_t i = 0; i < parser->nodes_size; i++) {
      node_cleanup(parser->nodes[i]);
    }
    free(parser->nodes);
    parser->nodes = NULL;
  }
  free(parser);
}
