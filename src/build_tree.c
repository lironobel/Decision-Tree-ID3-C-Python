#include "tree.h"
#include "dataset.h"
#include "infogain.h"
int global_depth = 0;  // משתנה גלובלי לשמירת העומק של העץ

//משתנים בשביל חישוב IG ממוצע עד כה
double global_total_ig = 0.0; // משתנה גלובלי לשמירת סך ה-IG //זה כדי לקבוע האם כדאי לעשות עצירה באלגוריתם או להמשיך בעץ 
int global_ig_count = 0;    // משתנה גלובלי לשמירת כמות ה-IG

void build_tree(Node *node, const char *csv_path) {  // 
    int class_count1 = 0, total_rows = 0;
    char **unique_classes = count_classes(csv_path, &class_count1, &total_rows);// כך נשנה את הערך של המשתנה שסופר כמה מחלקות שונות יש בקובץ בשביל התנאי עצירה הראשון
    
    SplitResult best_split = find_best_infogain(csv_path, class_count1, total_rows, global_total_ig, class_count1, unique_classes); // חישוב ה-IG עבור כל עמודה
    //תנאי עצירה ראשון
    // אם יש רק מחלקה אחת, ניצור צומת עלה
    if (class_count1 == 1) { 
        node->is_leaf = 1;
        node->num_classes = 1; // מספר המחלקות הוא 1
        node->labels = malloc(sizeof(int)); // הקצה זיכרון למערך של מחלקות
        node->labels[0] = total_rows; 
        printf("Leaf node: all examples belong to class %s\n", unique_classes[0]);
        for (int i = 0; i < class_count1; i++) {
            free(unique_classes[i]);
        }
        free(unique_classes);
        free(best_split.value);
        return;
    }

    //תנאי עצירה שני
    //ניצור צומת עלה אם הרווח מידע של הקובץ נמוך מידי ולא כדאי להמשיך עוד פיצול
    if(best_split.gain < 0.01) { // אם הרווח מידע נמוך מ-0.01, ניצור צומת עלה
        node->is_leaf = 1;
        node->num_classes = class_count1; // מספר המחלקות הוא מספר המחלקות השונות בקובץ
        node->labels = malloc(class_count1 * sizeof(int)); // הקצה זיכרון למערך של מחלקות
        for (int i = 0; i < class_count1; i++) {
            node->labels[i] = 0; // אתחול לערך 0
        }
        printf("Leaf node: low information gain\n");
        for (int i = 0; i < class_count1; i++) {
            free(unique_classes[i]);
        }
        free(unique_classes);
        free(best_split.value);
        return;
    }

    //תנאי עצירה שלישי
    // אם העומק של העץ הגיע ל-5, ניצור צומת עלה
    if (global_depth >= 5) {
        node->is_leaf = 1;
        node->num_classes = class_count1; // מספר המחלקות הוא מספר המחלקות השונות בקובץ
        node->labels = malloc(class_count1 * sizeof(int)); // הקצה זיכרון למערך של מחלקות
        for (int i = 0; i < class_count1; i++) {
            node->labels[i] = 0; // אתחול לערך 0
        }
        printf("Leaf node: maximum depth reached\n");
        for (int i = 0; i < class_count1; i++) {
            free(unique_classes[i]);
        }
        free(unique_classes);
        free(best_split.value);
        return;
    }

    // נמשיך לפיצול העץ
    printf("Splitting node on column %d with value %s\n", best_split.column_index, best_split.value);
    node->feature_index = best_split.column_index;
    node->threshold = best_split.value[0];
    node->left = create_node(0, NULL, 0, best_split.column_index, best_split.value[0]);
    node->right = create_node(0, NULL, 0, best_split.column_index, best_split.value[0]);
    build_tree(node->left, csv_path); // פיצול לעץ שמאלי
    build_tree(node->right, csv_path); // פיצול לעץ ימני

    
    // במקרה שלא נעצרים, נמשיך הלאה — עדיין צריך לשחרר את unique_classes
    for (int i = 0; i < class_count1; i++) {
        free(unique_classes[i]);
    }
    free(unique_classes);
}


