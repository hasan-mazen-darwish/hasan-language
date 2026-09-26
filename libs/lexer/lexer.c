#include "./lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lexer *lexer_tokenify(Lexer *lexer) {
  // Fixing the lexer src to not get into any problem
  lexer->src[lexer->srcLength] = '\0';

  size_t lLength = 0; // A shortcut for lexemeLength
  size_t initialLCapacity = 100;
  size_t lCapacity =
      initialLCapacity; // For detecting the allocation capacity of the lBuffer.
  char *lBuffer =
      malloc(lCapacity * sizeof(char)); // A shortcut for lexemeBuffer

  size_t currentLine = 0; // 0-indexed, just like then tokens arrays pointer
  size_t currentLineCursor = 0;
  size_t numberOfLinesAllocations = 4;
  int isString = 0;

  lexer->tokens = malloc(numberOfLinesAllocations * sizeof(Token *));
  if (lexer->tokens == NULL) {
    printf("Error allocating memory for the tokens addresses!\n");
    free(lBuffer);
    return NULL;
  }

  size_t tokensInitialAllocationSize = 5;
  size_t tokensNumberInCurrentLine = 0;
  size_t tokensAllocationCapacity = tokensInitialAllocationSize;

  lexer->tokens[currentLine] =
      malloc(tokensInitialAllocationSize * sizeof(Token));
  if (lexer->tokens[currentLine] == NULL) {
    printf("Failed to allocate memory for the tokens of the initial line!\n");
    return NULL;
  }
  char *p = lexer->src;
  while (*p) {
    currentLineCursor++;
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
      // The logic of tokenizing after hitting a new line:

      // Now, we will be adding the END_OF_LINE token after we finish the last
      // token, then we will reset the lBuffer and lLength if we weren't in a
      // string. Note that we are using currentLine-1 since we styarted this if
      // statement with adding one to the currentLine

      if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
        tokensAllocationCapacity += 1; // Adding only one just because we are
                                       // adding one and only one token.
        Token *temp = realloc(lexer->tokens[currentLine - 1],
                              tokensAllocationCapacity * sizeof(Token));
        if (temp == NULL) {
          printf("Failed reallocating memory for the END_OF_LINE token!\n");
          return NULL;
        }
        lexer->tokens[currentLine - 1] = temp;
      }
      lexer->tokens[currentLine - 1][tokensNumberInCurrentLine].line =
          currentLine; // This is for user readability, not 0-indexed.
      lexer->tokens[currentLine - 1][tokensNumberInCurrentLine].lexeme =
          strdup(lBuffer);
      lexer->tokens[currentLine - 1][tokensNumberInCurrentLine].start =
          currentLineCursor;
      lexer->tokens[currentLine - 1][tokensNumberInCurrentLine].type =
          END_OF_LINE;

      // Cleaning the lexeme info after the tokenizing
      lLength = 0;
      lCapacity = initialLCapacity;
      char *temp = realloc(lBuffer, lCapacity * sizeof(char));
      if (temp == NULL) {
        printf("Error reallocating the buffer in the resetting process in the "
               "new line logic!\n");
        return NULL;
      }
      lBuffer = temp;

      // Finally, initializing a new Tokens array for the next line (which is
      // now currentLine)
      tokensAllocationCapacity = tokensInitialAllocationSize;
      lexer->tokens[currentLine] =
          malloc(tokensAllocationCapacity * sizeof(Token));
      if (lexer->tokens[currentLine] == NULL) {
        printf("Failed allocating memory for the new lines tokens!\n");
        return NULL;
      }

      // Continuing properly in the loops
      tokensNumberInCurrentLine = 0;
      currentLineCursor = 0;
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

  // Freeing the buffer after being useless
  free(lBuffer);

  return lexer;
}
