#ifndef _PARSER_H
#define _PARSER_H

#include "../lexer/lexerLibs.h"

typedef struct Parser {
  Lexer *lexer;
} Parser;

Parser parser_new(Lexer *lexer);
void parser_attach_lexer(Parser *parser, Lexer *lexer);
void parser_clean(Parser *parser);
Parser *parser_parse(Parser *parser);

#endif // !_PARSER_H
