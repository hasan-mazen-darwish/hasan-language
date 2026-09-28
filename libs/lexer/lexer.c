#include "./lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TokensTypes classify_token(char **lexeme, size_t *lexemeLength,
                                  int *isString) {
  if (*isString == 1)
    return VARIABLE_STRING;
  else if (strcmp(*lexeme, "print") == 0)
    return FUNCTION_PRINT;
  else if (strcmp(*lexeme, "(") == 0)
    return SYMBOL_LEFT_PARENTHESIS;
  else if (strcmp(*lexeme, ")") == 0)
    return SYMBOL_RIGHT_PARENTHESIS;
  else if (strcmp(*lexeme, "=") == 0)
    return SYMBOL_EQUALS;
  else if (strcmp(*lexeme, "+") == 0)
    return SYMBOL_PLUS;
  else if (strcmp(*lexeme, "with") == 0)
    return KEYWORD_WITH;

  // For unspecified tokens:
  return UNKNOWN;
}

void lexer_clean(Lexer *lexer) {
  for (size_t i = 0; i < lexer->lines; i++) {
    free(lexer->tokens[i]);
  }
  free(lexer->tokens);
  free(lexer->src);

  // Unnecessary. Just nice-to-haves
  lexer->srcLength = 0;
  lexer->lines = 0;
}

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
    lexer_clean(lexer);
    return NULL;
  }

  size_t tokensInitialAllocationSize = 5;
  size_t tokensNumberInCurrentLine = 0;
  size_t tokensAllocationCapacity = tokensInitialAllocationSize;

  lexer->tokens[currentLine] =
      malloc(tokensInitialAllocationSize * sizeof(Token));
  if (lexer->tokens[currentLine] == NULL) {
    printf("Failed to allocate memory for the tokens of the initial line!\n");
    free(lBuffer);
    lexer_clean(lexer);
    return NULL;
  }
  char *p = lexer->src;
  lexer->lines = 1;
  while (*p) {
    currentLineCursor++;
    if (*p == '\n') {
      size_t previousLine = currentLine;
      currentLine++;
      lexer->lines++;
      if (currentLine + 1 >= numberOfLinesAllocations) {
        numberOfLinesAllocations *= 2;
        Token **temp =
            realloc(lexer->tokens, numberOfLinesAllocations * sizeof(Token *));
        if (temp == NULL) {
          printf("Error reallocating memory for tokenizing new lines of the "
                 "file!\n");
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        lexer->tokens = temp;
      }
      // The logic of tokenizing after hitting a new line:
      // Here, we will tokenize the lBuffer if it's not null
      if (lLength > 0) {
        TokensTypes classifiedToken =
            classify_token(&lBuffer, &lLength, &isString);
        if (classifiedToken == UNKNOWN) {
          printf(
              "Error tokenizing the source code: unknown token at %zu:%zu.\n",
              previousLine + 1, currentLineCursor - lLength);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

        if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
          // We will only add 2 slots: one for this token, and another for the
          // END_OF_LINE token.
          tokensAllocationCapacity += 2;
          Token *temp = realloc(lexer->tokens[previousLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf("Failed reallocating memory for the last token of the %zu "
                   "line!\n",
                   previousLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[previousLine] = temp;
        }

        lexer->tokens[previousLine][tokensNumberInCurrentLine].line =
            previousLine + 1; // Not 0-indexed for readability.
        lexer->tokens[previousLine][tokensNumberInCurrentLine].lexeme =
            strdup(lBuffer);
        lexer->tokens[previousLine][tokensNumberInCurrentLine].start =
            currentLineCursor - lLength;
        lexer->tokens[previousLine][tokensNumberInCurrentLine].type =
            classifiedToken;
        tokensNumberInCurrentLine++;

        lLength = 0;
        lCapacity = initialLCapacity;
        char *temp = realloc(lBuffer, lCapacity * sizeof(char));
        if (temp == NULL) {
          printf("Error reallocating the buffer in the resetting process in "
                 "the new line logic! (The old lBuffer is being tokenized)\n");
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
      }

      // Now, we will be adding the END_OF_LINE token after we finish the last
      // token, then we will reset the lBuffer and lLength if we weren't in a
      // string. Note that we are using currentLine-1 since we styarted this if
      // statement with adding one to the currentLine

      if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
        tokensAllocationCapacity += 1; // Adding only one just because we are
                                       // adding one and only one token.
        Token *temp = realloc(lexer->tokens[previousLine],
                              tokensAllocationCapacity * sizeof(Token));
        if (temp == NULL) {
          printf("Failed reallocating memory for the END_OF_LINE token in the "
                 "%zu line!\n",
                 previousLine + 1);
          lexer_clean(lexer);
          free(lBuffer);
          return NULL;
        }
        lexer->tokens[previousLine] = temp;
      }

      lexer->tokens[previousLine][tokensNumberInCurrentLine].line =
          previousLine + 1; // This is for user readability, not 0-indexed.
      lexer->tokens[previousLine][tokensNumberInCurrentLine].lexeme = "";
      lexer->tokens[previousLine][tokensNumberInCurrentLine].start =
          currentLineCursor;
      lexer->tokens[previousLine][tokensNumberInCurrentLine].type = END_OF_LINE;

      // Cleaning the lexeme info after the tokenizing
      lLength = 0;
      lCapacity = initialLCapacity;
      char *temp = realloc(lBuffer, lCapacity * sizeof(char));
      if (temp == NULL) {
        printf("Error reallocating the buffer in the resetting process in the "
               "new line logic!\n");
        free(lBuffer);
        lexer_clean(lexer);
        return NULL;
      }
      lBuffer = temp;
      lBuffer[0] = '\0';

      // Finally, initializing a new Tokens array for the next line (which is
      // now currentLine)
      tokensAllocationCapacity = tokensInitialAllocationSize;
      lexer->tokens[currentLine] =
          malloc(tokensAllocationCapacity * sizeof(Token));
      if (lexer->tokens[currentLine] == NULL) {
        printf("Failed allocating memory for the new lines tokens!\n");
        free(lBuffer);
        lexer_clean(lexer);
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
        TokensTypes classifiedToken =
            classify_token(&lBuffer, &lLength, &isString);
        if (classifiedToken == UNKNOWN) {
          printf(
              "Error tokenizing the source code: unknown token at %zu:%zu.\n",
              currentLine + 1, currentLineCursor - lLength);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

        if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
          tokensAllocationCapacity *= 2;
          Token *temp = realloc(lexer->tokens[currentLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf("Error reallocating the tokens array for a new token at "
                   "line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[currentLine] = temp;
        }

        lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
            currentLine + 1;
        lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
            strdup(lBuffer);
        lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
            currentLineCursor - lLength;
        lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
            classifiedToken;

        tokensNumberInCurrentLine++;
        lLength = 0;
        lCapacity = initialLCapacity;
        char *temp = realloc(lBuffer, lCapacity * sizeof(char));
        if (temp == NULL) {
          printf("Error resetting the lexeme buffer after reading the in-line "
                 "token at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        lBuffer = temp;
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
        free(lBuffer);
        lexer_clean(lexer);
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

  // Adding a final END_OF_LINE token to the last line of tokens.
  // Please note that the old values of the tokensNumberInCurrentLine and
  // tokensAllocationCapacity are still unchanged because we did not hit a new
  // line!
  if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
    tokensAllocationCapacity++;
    Token *temp = realloc(lexer->tokens[lexer->lines - 1],
                          tokensAllocationCapacity * sizeof(Token));
    if (temp == NULL) {
      printf("Error reallocating memory for the END_OF_LINE of the last line "
             "of the source code tokens!\n");
      free(lBuffer);
      lexer_clean(lexer);
      return NULL;
    }
    lexer->tokens[lexer->lines - 1] = temp;
  }
  lexer->tokens[lexer->lines - 1][tokensNumberInCurrentLine].line =
      lexer->lines; // Again, not 0-indexed
  lexer->tokens[lexer->lines - 1][tokensNumberInCurrentLine].lexeme = "";
  lexer->tokens[lexer->lines - 1][tokensNumberInCurrentLine].start =
      lexer->srcLength - 1;
  lexer->tokens[lexer->lines - 1][tokensNumberInCurrentLine].type = END_OF_LINE;

  return lexer;
}
