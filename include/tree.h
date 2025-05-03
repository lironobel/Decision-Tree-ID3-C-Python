#ifndef TREE_H
#define TREE_H

#include <stdlib.h>

typedef struct Node
{
    int feature_index;     // מספר התכונה
    float threshold;       // סף הפיצול
    int is_leaf;           // האם מדובר בעלה
    int *labels;           // מערך עם ספירת דוגמאות לכל מחלקה
    int num_classes;       // מספר המחלקות
    struct Node *left;     // בן שמאלי
    struct Node *right;    // בן ימני
} Node;

// יצירת צומת חדש בעץ
Node *create_node(int is_leaf, int *labels, int num_classes, int feature_index, float threshold);

// בניית עץ ההחלטה (אם אתה משתמש בפונקציה הזו מחוץ ל-tree.c)
void build_tree(Node *node, const char *csv_path);

void free_tree(Node *node);

#endif // TREE_H
