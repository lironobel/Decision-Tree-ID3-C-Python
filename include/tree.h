// === tree.h ===
#ifndef TREE_H
#define TREE_H

typedef struct Node {
	int is_leaf;
	int *labels;
	int num_classes;
	int feature_index;
	float threshold;
	struct Node *left;
	struct Node *right;
} Node;

Node *create_node(int is_leaf, int *labels, int num_classes, int feature_index, float threshold);
#endif