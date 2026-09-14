#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "./libs/lexer/lexer.h"

// The function that will read the file size

int read_file(FILE *fp, char **src, size_t *length) {
  size_t cap = 8 * 1024;
  size_t len = 0;

  char *buff = malloc(cap * sizeof(char));
  if(!buff) return 1;

  for(;;) {
    // Checking if the length of the current read is 
    if(len == cap) {
      if(cap > SIZE_MAX/2) {
        free(buff);
        return 1;
      }

      // If the cap did not exceed the max size of the size_t, then multiply it
      cap *= 2;
      char *temp = realloc(buff, cap * sizeof(char));
      if(temp == NULL) {
        free(buff);
        return 1;
      }
      buff = temp;
    }

    size_t successfulReads = fread(buff, sizeof(char), cap - len, fp);
    len += successfulReads;
    if(successfulReads == 0) {
      if(ferror(fp)) {
        free(buff);
        return 1;
      }
      break;
    }
  }

  char *temp = realloc(buff, (len + 1) * sizeof(char));
  if(temp == NULL) {
    free(buff);
    return 1;
  }
  buff = temp;
  buff[len] = '\0';

  *src = buff;
  *length = len;
  return 0;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    printf("Please provide the name of the file to run.\n");
    printf("Usage: %s <file-path>\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");
  if(fp == NULL) {
    printf("Error: Failed to open the file existing in the provided path!\n");
    return 1;
  }

  char *src;
  size_t length;
  if(read_file(fp, &src, &length) != 0) {
    printf("Error: Unable to locate the size of the file you are trying to run.\n");
    return 1;
  }

  Lexer lexer = { .src = src, .srcLength = length };

  printf("The name of the file provided is %s\n", argv[1]);
  return 0;
}
