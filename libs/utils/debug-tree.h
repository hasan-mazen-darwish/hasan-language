#ifndef _DEBUG_TREE_H_
#define _DEBUG_TREE_H_

#include <stdlib.h>

// DT stands for Debug Tree
// H stands for Horizontal
// V stands for Vertical
#define DT_H_LINE "\xE2\x94\x80"
#define DT_V_LINE "\xE2\x94\x82"
#define DT_BRANCH "\xE2\x94\x9C"
#define DT_LAST_BRANCH "\xE2\x94\x94"

#define DT_DEFAULT_BRANCHES_MALLOC_SIZE 8

#define DT_BOLD_GREEN "\033[0;1;32m"
#define DT_COLORS_RESET "\033[0m"

typedef struct DebugTreeBranch {
  int depth;
  char *text;
} DebugTreeBranch;

typedef struct DebugTree {
  int identation;
  DebugTreeBranch *branches;
  size_t branches_number;
  size_t branches_size;
} DebugTree;

DebugTree dt_create(int identation);
DebugTree dt_get_default();
void dt_clean(DebugTree *dt);
void dt_add_branch(DebugTree *dt, int depth, char *text);
void dt_print(DebugTree *dt);

#endif // !_DEBUG_TREE_H_
