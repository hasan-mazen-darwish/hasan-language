#ifndef _PARSER_H
#define _PARSER_H

#include "../lexer/lexerLibs.h"
#include <stdio.h>

//
// --- Enums ---
//
typedef enum { VARIABLE_ASSIGNING } ParserNodeTypes;

//
// --- Structs of the parser ---
//
typedef struct ParserNode ParserNode;

struct ParserNode {
  ParserNodeTypes type;
};

typedef struct Parser {
  Lexer *lexer;
  ParserNode *nodes;
  size_t nodes_size;
  size_t nodes_used;
} Parser;

//
// --- Functions ---
//
Parser parser_new(Lexer *lexer);
void parser_attach_lexer(Parser *parser, Lexer *lexer);
void parser_clean(Parser *parser);
Parser *parser_parse(Parser *parser);

#endif // !_PARSER_H
