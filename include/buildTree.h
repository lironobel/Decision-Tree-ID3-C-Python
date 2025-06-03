#ifndef BUILDTREE_H
#define BUILDTREE_H

#include <stdio.h>
#include "tree.h"
#include "dataset.h"
#include <stdlib.h>

// הצהרת הפונקציה לבניית העץ (עם עומק וצד)
void build_tree(Node **node_ptr, FILE *f, int depth, const char *side);

#endif // BUILDTREE_H
