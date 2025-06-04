#ifndef TREE_GRAPH_H
#define TREE_GRAPH_H

#include <stdio.h>
#include "tree.h"

// פונקציה לייצוא עץ ההחלטה לקובץ DOT
void export_tree_to_dot(Node *node, FILE *file, char **feature_names);

#endif
