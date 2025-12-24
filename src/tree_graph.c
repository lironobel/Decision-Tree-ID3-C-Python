#include <stdio.h>
#include "tree.h"
#include "tree_graph.h"

/*
 * פונקציה פנימית שמדפיסה את הצומת והקשתות של עץ ההחלטה בפורמט DOT
 * עבור כל צומת בעץ היא יוצרת מזהה ייחודי, ומדפיסה את התוכן שלו:
 * אם זה עלה – מציגה את מספר הדוגמאות לכל מחלקה וגם את האחוזים היחסיים
 * אם זה צומת רגיל – מציגה את התנאי לפיצול (תכונה וסף או ערך קטגוריאלי)
 */
void print_node_dot(Node *node, FILE *file, int *id_counter, char **feature_names);

/*
 * פונקציה עיקרית שמייצאת את עץ ההחלטה לקובץ בפורמט DOT (גרף)
 * מתחילה את מבנה הקובץ, קוראת לפונקציה רקורסיבית להדפסת הצמתים, וסוגרת את הגרף
 */
void export_tree_to_dot(Node *node, FILE *file, char **feature_names)
{
    fprintf(file, "digraph DecisionTree {\n");
    fprintf(file, "node [shape=ellipse, style=filled, fillcolor=white];\n");
    int id_counter = 0; // מונה ייחודי לצמתים
    print_node_dot(node, file, &id_counter, feature_names);
    fprintf(file, "}\n");
}

/*
 * פונקציה רקורסיבית להדפסת צומת בודד וקשתות לצמתים הבנים שלו בקובץ DOT
 *
 * אם הצומת הוא עלה:
 *   - מדפיסה את מספר הדוגמאות לכל מחלקה (class)
 *   - מחשבת את האחוז היחסי של כל מחלקה מתוך סך הכל ומוסיפה להצגה
 * אם הצומת הוא לא עלה:
 *   - מציגה את התנאי (תכונה > סף אם מספרי, או תכונה == ערך אם קטגוריאלי)
 *   - מוסיפה קשתות (חצים) לצמתים הבנים (True/False)
 */
void print_node_dot(Node *node, FILE *file, int *id_counter, char **feature_names)
{
    if (node == NULL)
        return;

    int current_id = (*id_counter)++; // מזהה ייחודי לכל צומת

    if (node->is_leaf)
    {
        fprintf(file, "node%d [label=\"Leaf\\n", current_id);

        // חישוב סך כל הדוגמאות בעלה
        int total = 0;
        for (int i = 0; i < node->num_classes; i++)
        {
            total += node->labels[i];
        }

        // הדפסת מספר הדוגמאות ואחוזים לכל מחלקה
        for (int i = 0; i < node->num_classes; i++)
        {
            double percentage = (total > 0) ? (100.0 * node->labels[i] / total) : 0.0;
            fprintf(file, "Class %d: %d (%.2f%%)\\n", i, node->labels[i], percentage);
        }

        fprintf(file, "\", shape=box, style=filled, fillcolor=lightgray];\n");
    }
    else
    {
        const char *feature_name = feature_names[node->feature_index];

        if (node->is_numeric_split)
        {
            fprintf(file, "node%d [label=\"%s > %.3f\"];\n", current_id, feature_name, node->threshold);
        }
        else
        {
            fprintf(file, "node%d [label=\"%s == %s\"];\n", current_id, feature_name, node->category_value);
        }

        // הדפסת צומת שמאלי
        if (node->left)
        {
            int left_id = *id_counter;
            print_node_dot(node->left, file, id_counter, feature_names);
            fprintf(file, "node%d -> node%d [label=\"False\"];\n", current_id, left_id);
        }

        // הדפסת צומת ימני
        if (node->right)
        {
            int right_id = *id_counter;
            print_node_dot(node->right, file, id_counter, feature_names);
            fprintf(file, "node%d -> node%d [label=\"True\"];\n", current_id, right_id);
        }
    }
}
