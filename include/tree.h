#ifndef TREE_H
#define TREE_H

#include <stdlib.h>

typedef struct Node
{
    int feature_index;       // מספר התכונה
    double threshold;        // סף הפיצול (למספרים)
    int is_leaf;             // האם מדובר בעלה
    int *labels;             // מערך עם ספירת דוגמאות לכל מחלקה
    int num_classes;         // מספר המחלקות
    int is_numeric_split;    //  האם הפיצול מספרי
    char *category_value;    //  שם הקטגוריה אם זה טקסט
    struct Node *left;
    struct Node *right;
} Node;


// יצירת צומת חדש בעץ
Node *create_node(int is_leaf, int *labels, int num_classes,
                  int feature_index, double threshold,
                  int is_numeric_split, const char *category_value);

void free_tree(Node *node);

#endif // TREE_H
