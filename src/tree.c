// מבנה העץ, יצירה, הדפסה, שחרור

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

Node *create_node(int is_leaf, int *labels, int num_classes,
                  int feature_index, double threshold,
                  int is_numeric_split, const char *category_value) // יצירת צומת חדש בעץ
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->is_leaf = is_leaf;
    node->feature_index = feature_index;
    node->threshold = threshold;
    node->num_classes = num_classes;
    node->is_numeric_split = is_numeric_split;

    // העתקת המערך של התוויות
    if (labels != NULL && num_classes > 0) {
        node->labels = (int *)malloc(sizeof(int) * num_classes);
        for (int i = 0; i < num_classes; i++)
            node->labels[i] = labels[i];
    } else {
        node->labels = NULL;
    }

    // אם הפיצול טקסטואלי – שמור מחרוזת
    if (!is_numeric_split && category_value != NULL)
    {
        node->category_value = strdup(category_value);
    }
    else
    {
        node->category_value = NULL;
    }

    node->left = NULL;
    node->right = NULL;

    return node;
}

void free_tree(Node *node) { // שחרור זיכרון של עץ ההחלטה
    if (node == NULL)
        return;

    // שחרר קודם את תתי-העצים
    free_tree(node->left);
    free_tree(node->right);

    // שחרר את מערך התוויות אם קיים
    if (node->labels != NULL)
        free(node->labels);

    // שחרר את ערך הקטגוריה אם קיים
    if (node->category_value != NULL)
        free(node->category_value);

    // שחרר את הצומת עצמו
    free(node);
}
