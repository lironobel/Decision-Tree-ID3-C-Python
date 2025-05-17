#include <stdio.h>
#include "tree.h"
#include "tree_graph.h"

void print_node_dot(Node *node, FILE *file, int *id_counter, char **feature_names);

void export_tree_to_dot(Node *node, FILE *file, char **feature_names) {
    fprintf(file, "digraph DecisionTree {\n");
    fprintf(file, "node [shape=ellipse, style=filled, fillcolor=white];\n");
    int id_counter = 0;
    print_node_dot(node, file, &id_counter, feature_names);
    fprintf(file, "}\n");
}

void print_node_dot(Node *node, FILE *file, int *id_counter, char **feature_names) {
    if (node == NULL) return;

    int current_id = (*id_counter)++;

    if (node->is_leaf) {
        fprintf(file, "node%d [label=\"Leaf\\n", current_id);
        for (int i = 0; i < node->num_classes; i++) {
            fprintf(file, "Class %d: %d\\n", i, node->labels[i]);
        }
        fprintf(file, "\", shape=box, style=filled, fillcolor=lightgray];\n");
    } else {
        const char *feature_name = feature_names[node->feature_index];

        if (node->is_numeric_split) {
            fprintf(file, "node%d [label=\"%s > %.3f\"];\n", current_id, feature_name, node->threshold);
        } else {
            fprintf(file, "node%d [label=\"%s == %s\"];\n", current_id, feature_name, node->category_value);
        }

        if (node->left) {
            int left_id = *id_counter;
            print_node_dot(node->left, file, id_counter, feature_names);
            fprintf(file, "node%d -> node%d [label=\"False\"];\n", current_id, left_id);
        }

        if (node->right) {
            int right_id = *id_counter;
            print_node_dot(node->right, file, id_counter, feature_names);
            fprintf(file, "node%d -> node%d [label=\"True\"];\n", current_id, right_id);
        }
    }
}
