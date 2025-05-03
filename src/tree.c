// מבנה העץ, יצירה, הדפסה, שחרור

#include <stdio.h>
#include <stdlib.h>
#include "tree.h"


Node *create_node(int is_leaf, int *labels, int num_classes, int feature_index, float threshold)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node)
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    node->feature_index = feature_index;
    node->is_leaf = is_leaf;
    node->threshold = threshold;
    node->labels = labels;
    node->num_classes = num_classes;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_tree(Node *node) {// שחרור זיכרון של עץ ההחלטה
    if (node == NULL)
        return;

    // שחרר קודם את תתי-העצים
    free_tree(node->left);
    free_tree(node->right);

    // שחרר את מערך התוויות אם קיים
    if (node->labels != NULL)
        free(node->labels);

    // שחרר את הצומת עצמו
    free(node);
}
