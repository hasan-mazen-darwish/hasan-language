#include "./lexer.h"
#include "../utils/hashmap.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isHashmapInitialized = 0;
int isVariablesHashmapInitialized = 0;
Hashmap keywords;
Hashmap variables;

typedef struct Variable {
  char *name;
  TokensTypes type;
} Variable;

static int
add_keyword(char *key,
            TokensTypes tokenType) { // Returning 1 on success and 0 on failure
  TokensTypes *boxedToken = malloc(sizeof(TokensTypes));
  if (boxedToken == NULL) {
    printf("Error adding keyword \"%s\"!\n", key);
    return 0;
  }
  boxedToken[0] = tokenType;
  return hashmap_set_key(&keywords, key, boxedToken);
};

static int
add_variable(char *name,
             TokensTypes type) { // Returning 1 on success and 0 on failure
  Variable *boxedToken = malloc(sizeof(Variable));
  if (boxedToken == NULL) {
    printf("Error adding variable named \"%s\"!\n", name);
    return 0;
  }
  Variable var = {name, type};
  boxedToken[0] = var;
  return hashmap_set_key(&variables, name, boxedToken);
}

static TokensTypes classify_token(char **lexeme, size_t *lexemeLength,
                                  int *isString) {
  if (isHashmapInitialized == 0) {
    keywords = hashmap_new();
    add_keyword("print", FUNCTION_PRINT);
    add_keyword("(", SYMBOL_LEFT_PARENTHESIS);
    add_keyword(")", SYMBOL_RIGHT_PARENTHESIS);
    add_keyword("=", SYMBOL_EQUALS);
    add_keyword("+", SYMBOL_PLUS);
    add_keyword("-", SYMBOL_MINUS);
    add_keyword("*", SYMBOL_ASTERISK);
    add_keyword("/", SYMBOL_SLASH);
    add_keyword("{", SYMBOL_LEFT_CURLY_BRACKET);
    add_keyword("}", SYMBOL_RIGHT_CURLY_BRACKET);
    add_keyword("!", SYMBOL_NOT);
    add_keyword("\"", SYMBOL_DOUBLE_QUOTES);
    add_keyword("//", COMMENT);
    add_keyword("==", OPERATION_IS_EQUALS);
    add_keyword("!=", OPERATION_ISNT_EQUALS);
    add_keyword(">=", OPERATION_GREATER_OR_EQUALS_THAN);
    add_keyword("<=", OPERATION_SMALLER_OR_EQUALS_THAN);
    add_keyword("+=", OPERATION_PLUS_EQUALS);
    add_keyword("-=", OPERATION_MINUS_EQUALS);
    add_keyword("*=", OPERATION_MULTIPLIES_EQUALS);
    add_keyword("/=", OPERATION_DIVIDES_EQUALS);
    add_keyword(">", OPERATION_GREATER_THAN);
    add_keyword("<", OPERATION_SMALLER_THAN);
    add_keyword("++", OPERATION_PLUS_PLUS);
    add_keyword("--", OPERATION_MINUS_MINUS);
    add_keyword("&&", OPERATION_AND);
    add_keyword("||", OPERATION_OR);
    add_keyword("number", VARIABLE_NUMBER_KEYWORD);
    add_keyword("with", KEYWORD_WITH);
    add_keyword("and", KEYWORD_AND);
    add_keyword("if", KEYWORD_IF);
    add_keyword("else", KEYWORD_ELSE);
    add_keyword("switch", KEYWORD_SWITCH);
    add_keyword("case", KEYWORD_CASE);
    add_keyword("default", KEYWORD_DEFAULT);
    isHashmapInitialized = 1;
  }
  if (isVariablesHashmapInitialized == 0) {
    variables = hashmap_new();
    isVariablesHashmapInitialized = 1;
  }

  Variable *variable = (Variable *)hashmap_get_value(&variables, *lexeme);
  TokensTypes *tokenType = (TokensTypes *)hashmap_get_value(&keywords, *lexeme);
  // For unspecified tokens:
  if (tokenType == NULL) {
    if (variable == NULL)
      return UNKNOWN;
    return variable->type;
  }
  return *tokenType;
}

void lexer_clean(Lexer *lexer) {
  if (lexer == NULL)
    return;
  for (size_t i = 0; i < lexer->lines; i++) {
    free(lexer->tokens[i]->lexeme);
    free(lexer->tokens[i]);
  }
  free(lexer->tokens);
  free(lexer->src);
  hashmap_clean_hashmap(&keywords);

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
         *character == '}' || *character == '"' || *character == '&' ||
         *character == '|';
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
  case '"':
    return SYMBOL_DOUBLE_QUOTES;
  case '&':
    return OPERATION_BITWISE_AND;
  case '|':
    return OPERATION_BITWISE_OR;
  default:
    return UNKNOWN;
  }
}

// Returns 0 on failure, 1 on success
static int reset_lbuffer(char **lBuffer, size_t *lLength, size_t *lCapacity,
                         size_t *initialLCapacity) {
  *lLength = 0;
  *lCapacity = *initialLCapacity;
  char *temp = realloc(*lBuffer, *lCapacity * sizeof(char));
  if (temp == NULL) {
    free(*lBuffer);
    return 0;
  }
  *lBuffer = temp;
  (*lBuffer)[0] = '\0';
  return 1;
}

// Returning 1 on success, and 0 on failure
static int lexer_add_token(size_t *line, size_t *tokensNumber,
                           size_t *tokensAllocationCapacity,
                           Token **tokensArray, char *lexeme, size_t start,
                           TokensTypes type) {
  if (*tokensNumber >= *tokensAllocationCapacity) {
    *tokensAllocationCapacity *= 2;
    Token *temp =
        realloc(*tokensArray, *tokensAllocationCapacity * sizeof(Token));
    if (temp == NULL)
      return 0;
    *tokensArray = temp;
  }
  (*tokensArray)[*tokensNumber].line = *line + 1; // Lines are not 0-indexed.
  (*tokensArray)[*tokensNumber].lexeme = strdup(lexeme);
  (*tokensArray)[*tokensNumber].start = start;
  (*tokensArray)[*tokensNumber].type = type;
  (*tokensNumber) += 1;
  return 1;
}

Lexer *lexer_tokenify(Lexer *lexer) {
  // Fixing the lexer src to not get into any problem
  lexer->src[lexer->srcLength] = '\0';
  if (isVariablesHashmapInitialized == 0) {
    variables = hashmap_new();
  }

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
        if (lexer_add_token(&previousLine, &tokensNumberInCurrentLine,
                            &tokensAllocationCapacity,
                            &lexer->tokens[previousLine], lBuffer,
                            currentLineCursor - lLength, COMMENT) == 0) {
          printf("Failed reallocating memory for the comment token of the %zu "
                 "line!\n",
                 previousLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error reallocating the buffer in the resetting process in "
                 "the new line logic! (The old lBuffer is being tokenized)\n");
          lexer_clean(lexer);
          return NULL;
        }

        if (lexer_add_token(&previousLine, &tokensNumberInCurrentLine,
                            &tokensAllocationCapacity,
                            &lexer->tokens[previousLine], lBuffer,
                            currentLineCursor, END_OF_LINE) == 0) {
          printf("Failed reallocating memory for the END_OF_LINE token at the "
                 "end of line %zu!\n",
                 previousLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

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
          add_variable(lBuffer, recordingVariableType);
          classifiedToken = recordingVariableType;
          recordingVariableType = VARIABLE;
          isRecordingVariable = 0;
        } else if (isRecordingNumber == 1) {
          classifiedToken = DATA_NUMBER;
          isRecordingNumber = 0;
          isDotSpotted = 0;
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

        if (lexer_add_token(
                &currentLine, &tokensNumberInCurrentLine,
                &tokensAllocationCapacity, &lexer->tokens[previousLine],
                lBuffer, currentLineCursor - lLength, classifiedToken) == 0) {
          printf("Failed reallocating memory for the last token of the %zu "
                 "line!\n",
                 previousLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error reallocating the buffer in the resetting process in "
                 "the new line logic! (The old lBuffer is being tokenized)\n");
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
      }
      isRecordingVariable = 0;
      recordingVariableType = VARIABLE;

      // Now, we will be adding the END_OF_LINE token after we finish the last
      // token, then we will reset the lBuffer and lLength if we weren't in a
      // string. Note that we are using currentLine-1 since we styarted this if
      // statement with adding one to the currentLine

      if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                          &tokensAllocationCapacity,
                          &lexer->tokens[previousLine], lBuffer,
                          currentLineCursor, END_OF_LINE) == 0) {
        printf("Failed reallocating memory for the END_OF_LINE token in the "
               "%zu line!\n",
               previousLine + 1);
        lexer_clean(lexer);
        free(lBuffer);
        return NULL;
      }
      // Cleaning the lexeme info after the tokenizing
      if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
          0) {
        printf("Error reallocating the buffer in the resetting process in the "
               "new line logic!\n");
        free(lBuffer);
        lexer_clean(lexer);
        return NULL;
      }

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

    // Before doing anything, we will do string checking:
    if (*p == '"' && isComment == 0) {
      if (isString == 0) {
        if (lLength > 0) {
          TokensTypes classifiedToken;
          if (isRecordingVariable) {
            classifiedToken = recordingVariableType;
            recordingVariableType = VARIABLE;
            isRecordingVariable = 0;
          } else if (isRecordingNumber) {
            classifiedToken = DATA_NUMBER;
            isRecordingNumber = 0;
            isDotSpotted = 0;
          } else
            classifiedToken = classify_token(&lBuffer, &lLength, &isString);

          if (lexer_add_token(
                  &currentLine, &tokensNumberInCurrentLine,
                  &tokensAllocationCapacity, &lexer->tokens[currentLine],
                  lBuffer, currentLineCursor - lLength, classifiedToken) == 0) {
            printf("Error reallocating memory for tokenizing the token "
                   "before the text in line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }

          if (reset_lbuffer(&lBuffer, &lLength, &lCapacity,
                            &initialLCapacity) == 0) {
            printf("Error resetting the current buffer after tokenizing it "
                   "before the string at line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
        }
      }

      else if (isString == 1) {
        if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                            &tokensAllocationCapacity,
                            &lexer->tokens[currentLine], lBuffer,
                            currentLineCursor - lLength, DATA_STRING) == 0) {
          printf("Error reallocating memory for tokenizing the token "
                 "of the string in line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error resetting the current buffer after tokenizing it "
                 "before the string at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
      }

      p++;
      isString = isString == 0 ? 1 : 0;
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
          add_variable(lBuffer, recordingVariableType);
          classifiedToken = recordingVariableType;
          recordingVariableType = VARIABLE;
          isRecordingVariable = 0;
        }

        // Then we check if we are recording a number data:
        else if (isRecordingNumber == 1) {
          classifiedToken = DATA_NUMBER;
          isRecordingNumber = 0;
          isDotSpotted = 0;
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

        if (lexer_add_token(
                &currentLine, &tokensNumberInCurrentLine,
                &tokensAllocationCapacity, &lexer->tokens[currentLine], lBuffer,
                currentLineCursor - lLength, classifiedToken) == 0) {
          printf("Error reallocating the tokens array for a new token at "
                 "line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }

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

        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error resetting the lexeme buffer after reading the in-line "
                 "token at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
      }

      p++;
      continue;
    }

    // We will check if there is a string, so we can watch for the \ escape
    // character
    if (isString == 1 && *p == '\\') {
      p++;
      switch (*p) {
      case 'n':
        *p = '\n';
        break;
      case 'r':
        *p = '\r';
        break;
      case 't':
        *p = '\t';
        break;
      case 'v':
        *p = '\v';
        break;
      case 'b':
        *p = '\b';
        break;
      case 'f':
        *p = '\f';
        break;
      case 'a':
        *p = '\a';
        break;
      case '\\':
        *p = '\\';
        break;
      case '\'':
        *p = '\'';
        break;
      case '"':
        *p = '"';
        break;
      case '?':
        *p = '?';
        break;
      default:
        p--;
        break;
      }
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
              free(two_tokens_buffer);
              if (lLength > 0) {
                TokensTypes tokenBeforeComment =
                    classify_token(&lBuffer, &lLength, &isString);
                if (tokenBeforeComment == UNKNOWN) {
                  printf(
                      "Error tokenizing the sorce code: Unknown token \"%s\" "
                      "at %zu:%zu.\n",
                      lBuffer, currentLine + 1, currentLineCursor - lLength);
                  free(lBuffer);
                  lexer_clean(lexer);
                  return NULL;
                }
                if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                                    &tokensAllocationCapacity,
                                    &lexer->tokens[currentLine], lBuffer,
                                    currentLineCursor - lLength,
                                    tokenBeforeComment) == 0) {
                  printf("Error reallocating memory for the token before the "
                         "comment in line %zu!\n",
                         currentLine + 1);
                  free(lBuffer);
                  lexer_clean(lexer);
                  return NULL;
                }
                reset_lbuffer(&lBuffer, &lLength, &lCapacity,
                              &initialLCapacity);
              }
              continue;
            }

            if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                                &tokensAllocationCapacity,
                                &lexer->tokens[currentLine], two_tokens_buffer,
                                currentLineCursor - 2, classifiedToken) == 0) {
              printf("Error reallocating new memory for the tokens of the "
                     "double tokens at line %zu!\n",
                     currentLine + 1);
              free(lBuffer);
              lexer_clean(lexer);
              return NULL;
            }
            free(two_tokens_buffer);
            continue;
          }
          free(two_tokens_buffer);
        }

        TokensTypes symbolToken = symbol_to_token(p);
        // First we will check if we are recording a variable:
        if (isRecordingVariable == 1) {

          if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                              &tokensAllocationCapacity,
                              &lexer->tokens[currentLine], lBuffer,
                              currentLineCursor - lLength,
                              recordingVariableType) == 0) {
            printf("Error reallocating new memory for the tokens of the "
                   "variable name at line %zu!\n",
                   currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          isRecordingVariable = 0;
          add_variable(lBuffer, recordingVariableType);
          recordingVariableType = VARIABLE;
        }

        // Then we will check if it came after a number:
        else if (isRecordingNumber == 1) {
          if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                              &tokensAllocationCapacity,
                              &lexer->tokens[currentLine], lBuffer,
                              currentLineCursor - lLength, DATA_NUMBER) == 0) {
            printf("Error reallocating new memory for the token of the "
                   "symbol '%c' at line %zu!\n",
                   *p, currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
          isRecordingNumber = 0;
          isDotSpotted = 0;
        }

        // If the lexeme is full, we will tokenize it too:
        else if (lLength > 0) {
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
          if (lexer_add_token(
                  &currentLine, &tokensNumberInCurrentLine,
                  &tokensAllocationCapacity, &lexer->tokens[currentLine],
                  lBuffer, currentLineCursor - lLength, classifiedToken) == 0) {
            printf("Error reallocating new memory for the token of the "
                   "symbol '%c' at line %zu!\n",
                   *p, currentLine + 1);
            free(lBuffer);
            lexer_clean(lexer);
            return NULL;
          }
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
        if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                            &tokensAllocationCapacity,
                            &lexer->tokens[currentLine], lexeme,
                            currentLineCursor - 1, symbolToken) == 0) {
          printf("Error reallocating new memory for the token of the "
                 "symbol '%c' at line %zu!\n",
                 *p, currentLine + 1);
          free(lBuffer);
          free(lexeme);
          lexer_clean(lexer);
          return NULL;
        }
        free(lexeme);

        // Resetting lBuffer
        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error resetting the lexeme buffer after reading the symbol "
                 "token at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        isRecordingNumber = 0;
        isDotSpotted = 0;
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
        if (lexer_add_token(
                &currentLine, &tokensNumberInCurrentLine,
                &tokensAllocationCapacity, &lexer->tokens[currentLine], lBuffer,
                currentLineCursor - lLength, recordingVariableType) == 0) {
          printf("Error reallocating new memory for the tokens of the "
                 "variable name and the symbol after it at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        add_variable(lBuffer, recordingVariableType);

        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error resetting the lexeme buffer after reading the in-line "
                 "token at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
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
        TokensTypes symbolToken = symbol_to_token(p);
        if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                            &tokensAllocationCapacity,
                            &lexer->tokens[currentLine], lexeme,
                            currentLineCursor - 1, symbolToken) == 0) {
          printf("Error reallocating new memory for the tokens of the "
                 "variable name and the symbol after it at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
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
        if (lexer_add_token(&currentLine, &tokensNumberInCurrentLine,
                            &tokensAllocationCapacity,
                            &lexer->tokens[currentLine], lBuffer,
                            currentLineCursor - lLength, DATA_NUMBER) == 0) {
          printf("Error reallocating new memory for the tokens of the "
                 "number data and whatever after it at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        if (reset_lbuffer(&lBuffer, &lLength, &lCapacity, &initialLCapacity) ==
            0) {
          printf("Error resetting the lexeme buffer after reading the in-line "
                 "token at line %zu!\n",
                 currentLine + 1);
          free(lBuffer);
          lexer_clean(lexer);
          return NULL;
        }
        isRecordingNumber = 0;
        isDotSpotted = 0;
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
  if (lexer_add_token(&(lexer->lines), &tokensNumberInCurrentLine,
                      &tokensAllocationCapacity,
                      &lexer->tokens[lexer->lines - 1], "",
                      lexer->srcLength - 1, END_OF_LINE) == 0) {
    printf("Error reallocating memory for the END_OF_LINE of the last line "
           "of the source code tokens!\n");
    free(lBuffer);
    lexer_clean(lexer);
    return NULL;
  }
  hashmap_clean_hashmap(&keywords);
  return lexer;
}
