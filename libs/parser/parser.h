#ifndef _PARSER_H
#define _PARSER_H

#include "../lexer/lexerLibs.h"
#include <stdio.h>

//
// --- Enums ---
//
typedef enum { VARIABLE_ASSIGNING } ParserNodeTypes;

typedef enum {
  TEXT, // For like variable names
  NUMBER,
  ARITHMETIC_OPERATION,
  NODE
} ParserExpressionTypes;

//
// --- Structs of the parser ---
//
typedef struct ParserNode ParserNode;

union ParserNodeSide {
  ParserNode *node;
};

struct ParserNode {
  ParserNodeTypes type;

  ParserExpressionTypes firstSideType;
  union ParserNodeSide firstSide;

  ParserExpressionTypes secondSideType;
  union ParserNodeSide secondSide;
};

typedef struct Parser {
  Lexer *lexer;
  ParserNode *nodes;
  size_t nodes_size;
} Parser;

//
// --- Functions ---
//
Parser parser_new(Lexer *lexer);
void parser_attach_lexer(Parser *parser, Lexer *lexer);
void parser_clean(Parser *parser);
Parser *parser_parse(Parser *parser);

#endif // !_PARSER_H
