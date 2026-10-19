#ifndef _PARSER_H
#define _PARSER_H

#include "../lexer/lexerLibs.h"
#include <stdio.h>

//
// --- Enums ---
//
typedef enum {
  NODE_NUMBER_VARIABLE,
  NODE_STRING_VARIABLE,
  NODE_BOOLEAN_VARIABLE,

  NODE_NUMBER_VALUE,
  NODE_STRING_VALUE,
  NODE_TRUE_VALUE,
  NODE_FALSE_VALUE,
  NODE_OPERATION,
} ParserNodeTypes;

typedef enum {
  PARSER_OPERATION_ADDITION,
  PARSER_OPERATION_SUBTRACTION,
  PARSER_OPERATION_MULTIPLICATION,
  PARSER_OPERATION_DIVISION
} ParserOperation;

//
// --- Structs of the parser ---
//
typedef struct ParserNode ParserNode;

struct ParserNode {
  ParserNodeTypes type;

  union content {
    double number;
    char *string;
    int boolean;

    struct numberVariable {
      char *identifier;
      ParserNode *value;
    } numberVariable;

    struct stringVariable {
      char *identifier;
      ParserNode *value;
    } stringVariable;

    struct booleanVariable {
      char *identifier;
      ParserNode *value;
    } booleanVariable;

    struct operation {
      ParserOperation type;
      ParserNode *node1;
      ParserNode *node2;
    } operation;
  } content;
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
