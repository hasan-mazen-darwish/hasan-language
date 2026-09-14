#include <stdio.h>
#include <stdlib.h>

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

  printf("The name of the file provided is %s\n", argv[1]);
  return 0;
}
