// === tree.h ===
#ifndef TREE_H
#define TREE_H

typedef struct Node
{
    int feature_index; // מספר התכונה
    float threshold;   // סף
    int is_leaf;       // האם עלה?
    int *labels;       // מספר הדוגמאות בכל מחלקה
    int num_classes;   // מספר המחלקות
    struct Node *left;
    struct Node *right;
} Node;

#endif