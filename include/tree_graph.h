#ifndef TREE_GRAPH_H
#define TREE_GRAPH_H

#include <stdio.h>
#include "tree.h"

// פונקציה לייצוא עץ ההחלטה לקובץ DOT
void export_tree_to_dot(Node *node, FILE *file, char **feature_names);

// פונקציה לייצוא עץ ההחלטה לקובץ PNG
void generate_and_open_graph(const char *dot_filename, const char *png_filename);

#endif
