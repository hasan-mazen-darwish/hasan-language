#ifndef _PARSER_H
#define _PARSER_H

#include "../lexer/lexerLibs.h"
#include <stdio.h>

//
// --- Enums ---
//
typedef enum { VARIABLE_ASSIGNING } ParserNodeTypes;

typedef enum { NODE, EXPRESSION, OPERATION } ParserNodeExpressionTypes;

typedef enum {
  EXPRESSION_VALUE_NUMBER,
  EXPRESSION_VALUE_STRING,
  EXPRESSION_VALUE,
  EXPRESSION_VARIABLE,
  EXPRESSION_VARIABLE_STRING,
  EXPRESSION_VARIABLE_NUMBER
} ParserExpressions;

//
// --- Structs of the parser ---
//
typedef struct ParserNode ParserNode;

typedef struct ParserExpression {
  ParserExpressions type;
} ParserExpression;

typedef struct ParserOperation {
} ParserOperation;

union ParserNodeSide {
  ParserNode *node;
  ParserExpression expression;
  ParserOperation operation;
};

struct ParserNode {
  ParserNodeTypes type;

  ParserNodeExpressionTypes firstSideType;
  union ParserNodeSide firstSide;

  ParserNodeExpressionTypes secondSideType;
  union ParserNodeSide secondSide;
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
