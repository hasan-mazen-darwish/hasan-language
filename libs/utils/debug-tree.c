#include "./debug-tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DebugTree dt_create(int identation) {
  DebugTreeBranch *branches =
      malloc(DT_DEFAULT_BRANCHES_MALLOC_SIZE * sizeof(DebugTreeBranch));
  if (branches == NULL) {
    printf("Error allocating memory for the debugger branches!\n");
    exit(1);
  }

  return (DebugTree){identation, branches, 0, DT_DEFAULT_BRANCHES_MALLOC_SIZE};
}

DebugTree dt_get_default() { return dt_create(2); }

void dt_clean(DebugTree *dt) {
  if (dt->branches != NULL) {
    free(dt->branches);
    dt->branches = 0;
    dt->branches_number = 0;
    dt->branches_size = 0;
  }
}

static void extend_branches_if_needed(DebugTree *dt) {
  if (dt == NULL) {
    printf("Error extending the branches of the debugging tree: the debugging "
           "tree itself is null!\n");
    exit(1);
  } else if (dt->branches == NULL) {
    printf("Error extending the branches of the debugging tree: the branches "
           "are null!\n");
    exit(1);
  }

  size_t size;
  if (dt->branches_size <= 0)
    size = DT_DEFAULT_BRANCHES_MALLOC_SIZE * sizeof(DebugTreeBranch);
  else
    size = dt->branches_size * 2;

  if (dt->branches_number >= dt->branches_size) {
    DebugTreeBranch *temp = realloc(dt->branches, size);
    if (temp == NULL) {
      printf("Error extending the branches of the debugging tree: failed "
             "reallocating new memory for them!\n");
      exit(1);
    }
    dt->branches = temp;
    dt->branches_size = size;
  }
}

void dt_add_branch(DebugTree *dt, int depth, char *text) {
  extend_branches_if_needed(dt);
  dt->branches[dt->branches_number].depth = depth;
  dt->branches[dt->branches_number].text = text;
  dt->branches_number++;
  return;
}

void dt_print(DebugTree *dt) {
  for (size_t i = 0; i < dt->branches_number; i++) {
    int cur_dep = dt->branches[i].depth; // cur_dep stands for current depth
    if (cur_dep == 0)
      printf("%s\n", dt->branches[i].text);
    else {
      for (int d = 1; d < cur_dep; d++)
        printf(DT_BOLD_GREEN DT_V_LINE DT_COLORS_RESET);
      if (i == dt->branches_number - 1 || dt->branches[i + 1].depth < cur_dep)
        printf(DT_BOLD_GREEN DT_LAST_BRANCH DT_COLORS_RESET);
      else
        printf(DT_BOLD_GREEN DT_BRANCH DT_COLORS_RESET);
    }
  }
}
