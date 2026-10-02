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
  else if (strcmp(*lexeme, "number") == 0)
    return VARIABLE_NUMBER_KEYWORD;
  else if (strcmp(*lexeme, "{") == 0)
    return SYMBOL_LEFT_CURLY_BRACKET;
  else if (strcmp(*lexeme, "}") == 0)
    return SYMBOL_RIGHT_CURLY_BRACKET;
  else if (strcmp(*lexeme, "!") == 0)
    return SYMBOL_NOT;
  else if (strcmp(*lexeme, "*") == 0)
    return SYMBOL_ASTERISK;
  else if (strcmp(*lexeme, "/") == 0)
    return SYMBOL_SLASH;
  else if (strcmp(*lexeme, "==") == 0)
    return OPERATION_IS_EQUALS;
  else if (strcmp(*lexeme, "!=") == 0)
    return OPERATION_ISNT_EQUALS;
  else if (strcmp(*lexeme, ">") == 0)
    return OPERATION_GREATER_THAN;
  else if (strcmp(*lexeme, ">=") == 0)
    return OPERATION_GREATER_OR_EQUALS_THAN;
  else if (strcmp(*lexeme, "<") == 0)
    return OPERATION_SMALLER_THAN;
  else if (strcmp(*lexeme, "<=") == 0)
    return OPERATION_SMALLER_OR_EQUALS_THAN;
  else if (strcmp(*lexeme, "++") == 0)
    return OPERATION_PLUS_PLUS;
  else if (strcmp(*lexeme, "+=") == 0)
    return OPERATION_PLUS_EQUALS;
  else if (strcmp(*lexeme, "--") == 0)
    return OPERATION_MINUS_MINUS;
  else if (strcmp(*lexeme, "-=") == 0)
    return OPERATION_MINUS_EQUALS;
  else if (strcmp(*lexeme, "-") == 0)
    return SYMBOL_MINUS;
  else if (strcmp(*lexeme, "*=") == 0)
    return OPERATION_MULTIPLIES_EQUALS;
  else if (strcmp(*lexeme, "/=") == 0)
    return OPERATION_DIVIDES_EQUALS;
  else if (strcmp(*lexeme, "//") == 0)
    return COMMENT;

  // For unspecified tokens:
  return UNKNOWN;
}

void lexer_clean(Lexer *lexer) {
  for (size_t i = 0; i < lexer->lines; i++) {
    free(lexer->tokens[i]->lexeme);
    free(lexer->tokens[i]);
  }
  free(lexer->tokens);
  free(lexer->src);

  // Unnecessary. Just nice-to-haves
  lexer->srcLength = 0;
  lexer->lines = 0;
}

// This function will check if the character given can be inside a variable name
// or cannot. For example, _ can be found inside a variable, but ; cannot.
static int is_variable_character_valid(char *character) {
  if (isalpha(*character))
    return 1;
  if (isdigit(*character))
    return 1;

  switch (*character) {
  case '_':
    return 1;
  }
  return 0;
}

static int is_symbol(char *character) {
  return *character == '=' || *character == '+' || *character == '(' ||
         *character == ')' || *character == '-' || *character == '!' ||
         *character == '*' || *character == '/' || *character == '{' ||
         *character == '}';
}

static TokensTypes symbol_to_token(char *character) {
  switch (*character) {
  case '=':
    return SYMBOL_EQUALS;
  case '+':
    return SYMBOL_PLUS;
  case '(':
    return SYMBOL_LEFT_PARENTHESIS;
  case ')':
    return SYMBOL_RIGHT_PARENTHESIS;
  case '-':
    return SYMBOL_MINUS;
  case '!':
    return SYMBOL_NOT;
  case '*':
    return SYMBOL_ASTERISK;
  case '/':
    return SYMBOL_SLASH;
  case '{':
    return SYMBOL_LEFT_CURLY_BRACKET;
  case '}':
    return SYMBOL_RIGHT_CURLY_BRACKET;
  default:
    return UNKNOWN;
  }
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
  int isComment = 0;
  int isRecordingVariable = 0; // Checks if the recording token is going to be
                               // labeled VARIABLE_something.
  int isRecordingNumber = 0; // Checking if we are recording a number, so we can
                             // tokenize it into DATA_NUMBER.
  int isDotSpotted =
      0; // This is for checking cases like 1.2.1 where it is invalid
  TokensTypes recordingVariableType = VARIABLE;

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
      // Before tokenizing the lBuffer, we will check if it is a comment, then
      // we will add the END_OF_LINE token and continue in the while loop:
      if (isComment == 1) {
        if (tokensNumberInCurrentLine >= tokensAllocationCapacity - 1) {
          tokensAllocationCapacity += 2;
          Token *temp = realloc(lexer->tokens[previousLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf(
                "Failed reallocating memory for the comment token of the %zu "
                "line!\n",
                previousLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[previousLine] = temp;
        }

        lexer->tokens[previousLine][tokensNumberInCurrentLine].line =
            previousLine + 1;
        lexer->tokens[previousLine][tokensNumberInCurrentLine].lexeme =
            strdup(lBuffer);
        lexer->tokens[previousLine][tokensNumberInCurrentLine].start =
            currentLineCursor - lLength;
        lexer->tokens[previousLine][tokensNumberInCurrentLine].type = COMMENT;
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
        lBuffer = temp;

        lexer->tokens[previousLine][tokensNumberInCurrentLine].line =
            previousLine + 1;
        lexer->tokens[previousLine][tokensNumberInCurrentLine].lexeme = "";
        lexer->tokens[previousLine][tokensNumberInCurrentLine].start =
            currentLineCursor;
        lexer->tokens[previousLine][tokensNumberInCurrentLine].type =
            END_OF_LINE;

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
        isComment = 0;
        p++;
        continue;
      }

      // Here, we will tokenize the lBuffer if it's not null
      if (lLength > 0) {
        TokensTypes classifiedToken;
        if (isRecordingVariable == 1) {
          classifiedToken = recordingVariableType;
          recordingVariableType = VARIABLE;
          isRecordingVariable = 0;
        } else if (isRecordingNumber == 1) {
          classifiedToken = DATA_NUMBER;
          isRecordingNumber = 0;
        } else {
          classifiedToken = classify_token(&lBuffer, &lLength, &isString);
        }

        if (classifiedToken == UNKNOWN) {
          printf("Error tokenizing the source code: unknown token \"%s\" at "
                 "%zu:%zu.\n",
                 lBuffer, previousLine + 1, currentLineCursor - lLength);
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
        lBuffer = temp;
      }
      isRecordingVariable = 0;
      recordingVariableType = VARIABLE;

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

    if (isspace((unsigned char)*p) != 0 && (lLength == 0 || isString == 0) &&
        isComment == 0) {
      // This either detects whitespaces in the beginning of the text or the
      // whitespaces after the non-string tokens, and it must be not in a
      // comment

      if (lLength > 0) {
        TokensTypes classifiedToken;
        // We check if there is a variable being recorded first:
        if (isRecordingVariable == 1) {
          classifiedToken = recordingVariableType;
          recordingVariableType = VARIABLE;
          isRecordingVariable = 0;
        }

        // Then we check if we are recording a number data:
        else if (isRecordingNumber == 1) {
          classifiedToken = DATA_NUMBER;
          isRecordingNumber = 0;
        }

        else {
          classifiedToken = classify_token(&lBuffer, &lLength, &isString);
        }

        // If it is a symbol, we take the corresponding token:
        if (is_symbol(lBuffer)) {
          classifiedToken = symbol_to_token(lBuffer);
          char symbol = *lBuffer;
          char *temp = realloc(lBuffer, 2 * sizeof(char));
          if (temp == NULL) {
            printf("Error reallocating the lexeme buffer in order to fit the "
                   "symbol after the whitespace at %zu:%zu!\n",
                   currentLine + 1, currentLineCursor - 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lBuffer = temp;
          lLength = 1;
          lCapacity = initialLCapacity;
          lBuffer[0] = symbol;
          lBuffer[1] = '\0';
        }

        if (classifiedToken == UNKNOWN) {
          printf("Error tokenizing the source code: unknown token \"%s\" at "
                 "%zu:%zu.\n",
                 lBuffer, currentLine + 1, currentLineCursor - lLength);
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

        // Finally, we will check if the token will require recording a variable
        // or not.
        if (classifiedToken == VARIABLE_NUMBER_KEYWORD) {
          isRecordingVariable = 1;
          recordingVariableType = VARIABLE_NUMBER;
        }

        // TODO: Add a hash table for the variables, and attach them to the
        // lexer, so the parser, AST, and interpreter won't do additional
        // processing to check if the variable do exist or not. That's the lexer
        // mission anyways.

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

    // Now, no whitespace detected, but before writing into the buffer, we need
    // to check if the buffer previously is a string or a comment or not, if
    // not, we will check it.
    if (isString == 0 && isComment == 0) {
      // First of all, we will check if it is a symbol, so we will tokenize it
      // directly, resetting the variable recording and number recording.

      if (is_symbol(p) == 1) {
        // We will peak at the next char, then we will tokenize it if it's
        // two-symbols token:
        if (is_symbol(p + 1) == 1) {
          char *two_tokens_buffer = malloc(3 * sizeof(char));
          if (two_tokens_buffer == NULL) {
            printf("Error allocating memory for the two symbols token!\n");
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }

          two_tokens_buffer[0] = *p;
          two_tokens_buffer[1] = *(p + 1);
          two_tokens_buffer[2] = '\0';

          size_t lexemeLength = 2;
          TokensTypes classifiedToken =
              classify_token(&two_tokens_buffer, &lexemeLength, &isString);
          if (classifiedToken != UNKNOWN) {
            p += 2;
            if (classifiedToken == COMMENT) {
              isComment = 1;
              continue;
            }
            if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
              tokensAllocationCapacity *= 2;
              Token *temp = realloc(lexer->tokens[currentLine],
                                    tokensAllocationCapacity * sizeof(Token));
              if (temp == NULL) {
                printf("Error reallocating new memory for the tokens of the "
                       "double tokens at line %zu!\n",
                       currentLine + 1);
                free(lBuffer);
                lexer_clean(lexer);
                return NULL;
              }
              lexer->tokens[currentLine] = temp;
            }
            lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
                currentLine + 1; // Line numbering isn't 0-indexed.
            lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
                strdup(two_tokens_buffer);
            lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
                currentLineCursor - 2;
            lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
                classifiedToken;
            tokensNumberInCurrentLine++;
            continue;
          }
        }

        TokensTypes symbolToken = symbol_to_token(p);
        // First we will check if we are recording a variable:
        if (isRecordingVariable == 1) {
          if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
            tokensAllocationCapacity *= 2;
            Token *temp = realloc(lexer->tokens[currentLine],
                                  tokensAllocationCapacity * sizeof(Token));
            if (temp == NULL) {
              printf("Error reallocating new memory for the tokens of the "
                     "variable name at line %zu!\n",
                     currentLine + 1);
              free(lBuffer);
              lexer_clean(lexer);
              return NULL;
            }
            lexer->tokens[currentLine] = temp;
          }
          lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
              currentLine + 1; // Line numbering isn't 0-indexed.
          lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
              strdup(lBuffer);
          lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
              currentLineCursor - lLength;
          lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
              recordingVariableType;
          tokensNumberInCurrentLine++;
          isRecordingVariable = 0;
          recordingVariableType = VARIABLE;
        }

        // Then we will check if it came after a number:
        else if (isRecordingNumber == 1) {
          if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
            tokensAllocationCapacity *= 2;
            Token *temp = realloc(lexer->tokens[currentLine],
                                  tokensAllocationCapacity * sizeof(Token));
            if (temp == NULL) {
              printf("Error reallocating new memory for the token of the "
                     "symbol '%c' at line %zu!\n",
                     *p, currentLine + 1);
              free(lBuffer);
              lexer_clean(lexer);
              return NULL;
            }
            lexer->tokens[currentLine] = temp;
          }
          lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
              currentLine + 1; // Lines are not 0-indexed
          lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
              strdup(lBuffer);
          lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
              currentLineCursor - lLength;
          lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
              DATA_NUMBER;
          tokensNumberInCurrentLine++;
          isRecordingNumber = 0;
        }

        // If the lexeme is full, we will tokenize it too:
        else if (lLength > 0) {
          if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
            tokensAllocationCapacity *= 2;
            Token *temp = realloc(lexer->tokens[currentLine],
                                  tokensAllocationCapacity * sizeof(Token));
            if (temp == NULL) {
              printf("Error reallocating new memory for the token of the "
                     "symbol '%c' at line %zu!\n",
                     *p, currentLine + 1);
              free(lBuffer);
              lexer_clean(lexer);
              return NULL;
            }
            lexer->tokens[currentLine] = temp;
          }
          TokensTypes classifiedToken =
              classify_token(&lBuffer, &lLength, &isString);
          if (classifiedToken == UNKNOWN) {
            printf("Error tokenizing the source code: unknown token \"%s\" at "
                   "%zu:%zu",
                   lBuffer, currentLine + 1, currentLineCursor - lLength);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }

          lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
              currentLine + 1; // not 0-indexed
          lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
              strdup(lBuffer);
          lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
              currentLineCursor - lLength;
          lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
              classifiedToken;
          tokensNumberInCurrentLine++;
        }

        // Inserting the symbol into the tokens:
        char *lexeme = malloc(2 * sizeof(char));
        if (lexeme == NULL) {
          printf("Error generating a memory allocation for the lexeme of the "
                 "symbol after the variable at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        lexeme[0] = *p;
        lexeme[1] = '\0';
        if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
          tokensAllocationCapacity *= 2;
          Token *temp = realloc(lexer->tokens[currentLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf("Error reallocating new memory for the token of the "
                   "symbol '%c' at line %zu!\n",
                   *p, currentLine + 1);
            free(lBuffer);
            free(lexeme);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[currentLine] = temp;
        }
        lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
            currentLine + 1; // The line number is not 0-indexed
        lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
            strdup(lexeme);
        lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
            currentLineCursor - 1;
        lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
            symbolToken;
        tokensNumberInCurrentLine++;
        free(lexeme);

        // Resetting lBuffer
        lLength = 0;
        lCapacity = initialLCapacity;
        char *temp = realloc(lBuffer, lCapacity * sizeof(char));
        if (temp == NULL) {
          printf("Error resetting the lexeme buffer after reading the symbol "
                 "token at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        lBuffer = temp;
        isRecordingNumber = 0;
        isRecordingVariable = 0;
        p++;
        continue;
      }

      // Then, we will check if we are recording a variable, and the
      // user tried to insert a digit as the first character of the variable
      // name:
      else if (isRecordingVariable == 1 && isdigit(*p) && lLength == 0) {
        printf("Error tokenizing the source code: Cannot use a number digit as "
               "the beginning of a variable name at %zu:%zu",
               currentLine + 1, currentLineCursor);
        free(lBuffer);
        lexer_clean(lexer);
        return NULL;
      }

      // And then, we will check if the user has written a character that is not
      // valid inside the variable name. So, we will therefore tokenize the
      // variable and then check the symbol:
      else if (isRecordingVariable == 1 &&
               is_variable_character_valid(p) == 0 && lLength > 0) {
        if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
          tokensAllocationCapacity *= 2;
          Token *temp = realloc(lexer->tokens[currentLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf("Error reallocating new memory for the tokens of the "
                   "variable name and the symbol after it at line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[currentLine] = temp;
        }

        lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
            strdup(lBuffer);
        lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
            currentLine + 1; // The current line is not 0-indexed.
        lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
            currentLineCursor - lLength;
        lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
            recordingVariableType;
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
        isRecordingVariable = 0;
        recordingVariableType = VARIABLE;

        // Now checking the current symbol if it is a valid symbol or not:
        if (is_symbol(p) == 0) {
          printf("Error tokenizing the source code: cannot identify symbol "
                 "'%c' at %zu:%zu.\n",
                 *p, currentLine + 1, currentLineCursor - 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

        TokensTypes symbolToken = symbol_to_token(p);
        if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
          tokensAllocationCapacity *= 2;
          Token *temp = realloc(lexer->tokens[currentLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf("Error reallocating new memory for the tokens of the "
                   "variable name and the symbol after it at line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[currentLine] = temp;
        }
        char *lexeme = malloc(2 * sizeof(char));
        if (lexeme == NULL) {
          printf("Error generating a memory allocation for the lexeme of the "
                 "symbol after the variable at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        lexeme[0] = *p;
        lexeme[1] = '\0';
        lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
            currentLine + 1; // The line number is not 0-indexed
        lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
            strdup(lexeme);
        lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
            currentLineCursor - 1;
        lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
            symbolToken;
        tokensNumberInCurrentLine++;
        free(lexeme);
        p++; // To skip the current symbol, since we tokenized it
        continue;
      }

      // Now, we will check if the buffer starts with a digit so we define a
      // number:
      else if (isdigit(*p) && isRecordingNumber == 0 &&
               isRecordingVariable == 0 && lLength == 0) {
        isRecordingNumber = 1;
      }

      // Now, as we are actually recording a number; we will be watching for
      // the dot:
      else if (*p == '.' && isRecordingNumber == 1) {
        if (isDotSpotted == 1) {
          printf("Error tokenizing the source code: Cannot use two dots inside "
                 "the number, at %zu:%zu.\n",
                 currentLine + 1, currentLineCursor);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        } else
          isDotSpotted = 1;
      }

      // Now, we will check if we are watching a number value, but not a
      // decimal point nor a digit is recorded, we will terminate the number
      // and add it's value to the tokens array:
      else if (isRecordingNumber == 1 && (*p != '.' && isdigit(*p) == 0)) {
        if (tokensNumberInCurrentLine >= tokensAllocationCapacity) {
          tokensAllocationCapacity *= 2;
          Token *temp = realloc(lexer->tokens[currentLine],
                                tokensAllocationCapacity * sizeof(Token));
          if (temp == NULL) {
            printf("Error reallocating new memory for the tokens of the "
                   "number data and whatever after it at line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          lexer->tokens[currentLine] = temp;
        }

        lexer->tokens[currentLine][tokensNumberInCurrentLine].lexeme =
            strdup(lBuffer);
        lexer->tokens[currentLine][tokensNumberInCurrentLine].line =
            currentLine + 1; // The current line is not 0-indexed.
        lexer->tokens[currentLine][tokensNumberInCurrentLine].start =
            currentLineCursor - lLength;
        lexer->tokens[currentLine][tokensNumberInCurrentLine].type =
            DATA_NUMBER;
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
        isRecordingNumber = 0;
      }
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
