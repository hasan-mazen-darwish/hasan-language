#include "./parser.h"
#include <stdlib.h>

Parser parser_new(Lexer *lexer) {
  Parser parser = {.lexer = lexer, .nodes = NULL};
  return parser;
}

void parser_attach_lexer(Parser *parser, Lexer *lexer) {
  lexer_clean(parser->lexer);
  parser->lexer = lexer;
}

void parser_clean(Parser *parser) {
  lexer_clean(parser->lexer);
  free(parser->nodes);
  free(parser);
}
