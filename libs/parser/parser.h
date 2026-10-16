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
  NODE_FALSE_VALUE
} ParserNodeTypes;

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
      double value;
    } numberVariable;

    struct stringVariable {
      char *identifier;
      char *value;
    } stringVariable;

    struct booleanVariable {
      char *identifier;
      int value;
    } booleanVariable;
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
