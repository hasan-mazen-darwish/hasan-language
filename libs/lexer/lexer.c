#include "./lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lexer *lexer_tokenify(Lexer *lexer) {
  size_t lLength = 0; // A shortcut for lexemeLength
  size_t lCapacity =
      100; // For detecting the allocation capacity of the lBuffer.
  char *lBuffer =
      malloc(lCapacity * sizeof(char)); // A shortcut for lexemeBuffer

  size_t currentLine = 0; // 0-indexed, just like then tokens arrays pointer
  size_t numberOfLinesAllocations = 4;
  int isString = 0;

  lexer->tokens = malloc(numberOfLinesAllocations * sizeof(Token *));
  if (lexer->tokens == NULL) {
    printf("Error allocating memory for the tokens addresses!\n");
    free(lBuffer);
    return NULL;
  }

  size_t tokensInitialAllocationSize = 5;
  size_t currentTokensAllocationSize = tokensInitialAllocationSize;

  lexer->tokens[currentLine] =
      malloc(tokensInitialAllocationSize * sizeof(Token));
  if (lexer->tokens[currentLine] == NULL) {
    printf("Failed to allocate memory for the tokens of the initial line!\n");
    return NULL;
  }
  char *p = lexer->src;
  while (*p) {
    if (*p == '\n') {
      currentLine++;
      if (currentLine + 1 >= numberOfLinesAllocations) {
        numberOfLinesAllocations *= 2;
        Token **temp =
            realloc(lexer->tokens, numberOfLinesAllocations * sizeof(Token *));
        if (temp == NULL) {
          printf("Error reallocating memory for tokenizing new lines of the "
                 "file!\n");
          free(lBuffer);
          return NULL;
        }
        lexer->tokens = temp;
      }

      lexer->tokens[currentLine] =
          malloc(tokensInitialAllocationSize * sizeof(Token));
      if (lexer->tokens[currentLine] == NULL) {
        printf("Failed to allocate memory for the tokens of the new line!\n");
        return NULL;
      }

      if (lLength >= lCapacity) {
        lCapacity *= 2;
        char *temp = realloc(lBuffer, lCapacity * sizeof(char));
        if (temp == NULL) {
          printf("Failed reallocating memory for the lexeme buffer!\n");
          free(lexer->tokens);
          return NULL;
        }
        lBuffer = temp;
      }
      // TODO: when hitting a new line, consider also doing the exact same
      // thing as when a whitespace is hit after a keyword
      p++;
      continue;
    }

    if (isspace((unsigned char)*p) != 0 && (lLength == 0 || isString == 0)) {
      // This either detects whitespaces in the beginning of the text or the
      // whitespaces after the non-string tokens

      if (lLength > 0) {
      }

      p++;
      continue;
    }

    // No whitespace detected. Therefore, we will write to the buffer.
    lLength++;
    if (lLength >= lCapacity) {
      lCapacity *= 2;
      char *temp = realloc(lBuffer, lCapacity * sizeof(char));
      if (temp == NULL) {
        printf("Failed reallocating memory for the lexeme buffer!\n");
        free(lexer->tokens);
        return NULL;
      }
      lBuffer = temp;
    }
    // A note for my future self:
    // Here, we increased the length before using it. See ~11 lines upwards.
    // That means, if we have the buffer "pr" for example, then we have a
    // previous length of 2. We pre-increased it, and it became 3 now. And, we
    // need to add a new character, which is in the index of the previous
    // length, or lLength-1
    lBuffer[lLength - 1] = *p;
    lBuffer[lLength] = '\0';

    p++;
  }

  return lexer;
}
