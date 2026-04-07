#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "tree_graph.h"

/*
 * פונקציה פנימית שמדפיסה את הצומת והקשתות של עץ ההחלטה בפורמט DOT
 * עבור כל צומת בעץ היא יוצרת מזהה ייחודי, ומדפיסה את התוכן שלו:
 * אם זה עלה – מציגה את מספר הדוגמאות לכל מחלקה וגם את האחוזים היחסיים
 * אם זה צומת רגיל – מציגה את התנאי לפיצול (תכונה וסף או ערך קטגוריאלי)
 */
void print_node_dot(Node *node, FILE *file, int *id_counter, char **feature_names, char **class_names, int num_classes)
{
    if (node == NULL)
        return;

    int current_id = (*id_counter)++; // מזהה ייחודי לכל צומת

    // חישוב סך כל הדוגמאות בצומת הנוכחי לצורך עיצוב
    int total = 0;
    for (int i = 0; i < node->num_classes; i++)
    {
        total += node->labels[i];
    }

    if (node->is_leaf)
    {
        // קביעת צבע העלה לפי המחלקה השולטת (V2 Upgrade)
        const char *fillcolor = "#E0E0E0"; // אפור כברירת מחדל
        if (node->labels[1] > node->labels[0])
            fillcolor = "#99FF99"; // ירוק אם רוב ל-Class 1
        else if (node->labels[0] > node->labels[1])
            fillcolor = "#FF9999"; // אדום אם רוב ל-Class 0

        fprintf(file, "    node%d [label=\"Leaf\\n", current_id);

        // הדפסת מספר הדוגמאות ואחוזים לכל מחלקה
        for (int i = 0; i < node->num_classes; i++)
        {
            double percentage = (total > 0) ? (100.0 * node->labels[i] / total) : 0.0;
            const char *cname = (class_names && i < num_classes) ? class_names[i] : "?";
            fprintf(file, "%s: %d (%.2f%%)\\n", cname, node->labels[i], percentage);
        }

        // הגדרת עיצוב העלה עם הצבע שנבחר
        fprintf(file, "\", shape=box, style=filled, fillcolor=\"%s\"];\n", fillcolor);
    }
    else
    {
        const char *feature_name = feature_names[node->feature_index];

        // עיצוב צומת החלטה בצורת אליפסה עם צבע כחול בהיר
        if (node->is_numeric_split)
        {
            fprintf(file, "    node%d [label=\"%s > %.3f\", shape=ellipse, style=filled, fillcolor=\"#D1E8FF\"];\n", current_id, feature_name, node->threshold);
        }
        else
        {
            fprintf(file, "    node%d [label=\"%s == %s\", shape=ellipse, style=filled, fillcolor=\"#D1E8FF\"];\n", current_id, feature_name, node->category_value);
        }

        // הדפסת צומת שמאלי (False) - קשת אדומה
        if (node->left)
        {
            int left_id = *id_counter;
            print_node_dot(node->left, file, id_counter, feature_names, class_names, num_classes);
            fprintf(file, "    node%d -> node%d [label=\"False\", color=\"red\"];\n", current_id, left_id);
        }

        // הדפסת צומת ימני (True) - קשת ירוקה ועבה
        if (node->right)
        {
            int right_id = *id_counter;
            print_node_dot(node->right, file, id_counter, feature_names, class_names, num_classes);
            fprintf(file, "    node%d -> node%d [label=\"True\", color=\"green\", penwidth=2];\n", current_id, right_id);
        }
    }
}

/*
 * פונקציה עיקרית שמייצאת את עץ ההחלטה לקובץ בפורמט DOT (גרף)
 * מתחילה את מבנה הקובץ, קוראת לפונקציה רקורסיבית להדפסת הצמתים, וסוגרת את הגרף
 */
void export_tree_to_dot(Node *node, FILE *file, char **feature_names, char **class_names, int num_classes)
{
    fprintf(file, "digraph DecisionTree {\n");
    fprintf(file, "    graph [rankdir=TB, nodesep=0.5, ranksep=0.5, fontname=\"Arial\"];\n");
    int id_counter = 0;
    print_node_dot(node, file, &id_counter, feature_names, class_names, num_classes);
    fprintf(file, "}\n");
}

/*
 * פונקציה חדשה המבצעת אוטומציה:
 * 1. הופכת את קובץ ה-DOT לתמונת PNG בעזרת Graphviz
 * 2. שומרת את התמונה בלבד (ללא פתיחה אוטומטית)
 */
void generate_and_open_graph(const char *dot_filename, const char *img_filename)
{
    char command[1024]; // הגדלנו מעט את החוצץ לנתיבים ארוכים

    // אם קיים GRAPHVIZ_DOT נשתמש בו, אחרת ננסה dot מה-PATH
    const char *dot_path = getenv("GRAPHVIZ_DOT");
    if (dot_path == NULL || dot_path[0] == '\0')
    {
        dot_path = "dot";
    }

    // הרצת הפקודה (שימוש בגרשיים כדי לטפל ברווחים)
    snprintf(command, sizeof(command), "cmd /c \"\"%s\" -Tpng \"%s\" -o \"%s\"\"", dot_path, dot_filename, img_filename);

    printf("Executing: %s\n", command);
    int result = system(command);

    if (result != 0)
    {
        printf("[ERROR] Graphviz failed to generate image. Check GRAPHVIZ_DOT or PATH.\n");
        return;
    }

    printf("Tree image generated: %s\n", img_filename);
}