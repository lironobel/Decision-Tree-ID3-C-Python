#include "tree.h"
#include "dataset.h"
#include "infogain.h"
int global_depth = 0;  // משתנה גלובלי לשמירת העומק של העץ

//משתנים בשביל חישוב IG ממוצע עד כה
//זה כדי לקבוע האם כדאי לעשות עצירה באלגוריתם או להמשיך בעץ 
//לפי הרווח מידע הממוצע עד כה
double global_total_ig = 0.0; // משתנה גלובלי לשמירת סך ה-IG 
int global_ig_count = 0;    // משתנה גלובלי לשמירת כמות ה-IG

void build_tree(Node *node, const char *csv_path) {  // 
    int class_count1 = 0, total_rows = 0;
    char **unique_classes = count_classes(csv_path, &class_count1, &total_rows);// כך נשנה את הערך של המשתנה שסופר כמה מחלקות שונות יש בקובץ בשביל התנאי עצירה הראשון
    
    //לפני פיצול ראשון
    if(node == NULL) { // אם הצומת הוא NULL, ניצור צומת חדש
        node = create_node(0, NULL, 0, -1, -1); // יצירת צומת חדש
    }
    global_depth++; // הגדלת העומק של העץ

    SplitResult best_split = find_best_infogain(csv_path, class_count1, total_rows, global_total_ig, class_count1, unique_classes); // חישוב ה-IG עבור כל עמודה
    
    double average_ig = global_total_ig / global_ig_count;
    printf("Best Gain Overall: Column %d with Gain %.6f\n", best_split.column_index, best_split.gain); // הדפסת העמודה עם הרווח המידע הגבוה ביותר
    
    
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

    //תנאי עצירה רביעי
    // אם מספר השורות בקובץ קטן מ-10, ניצור צומת עלה
    if (total_rows <= 10) {
        node->is_leaf = 1;
        node->num_classes = class_count1; // מספר המחלקות הוא מספר המחלקות השונות בקובץ
        node->labels = malloc(class_count1 * sizeof(int)); // הקצה זיכרון למערך של מחלקות
        for (int i = 0; i < class_count1; i++) {
            node->labels[i] = 0; // אתחול לערך 0
        }
        printf("Leaf node: not enough examples\n");
        for (int i = 0; i < class_count1; i++) {
            free(unique_classes[i]);
        }
        free(unique_classes);
        free(best_split.value);
        return;
    }
    //תנאי עצירה חמישי
    // אם הרווח מידע הממוצע עד כה הוא נמוך מ-0.01, ניצור צומת עלהif (average_ig < 0.01) {
    if (average_ig < 0.01) {
        node->is_leaf = 1;
        node->num_classes = class_count1;
        node->labels = calloc(class_count1, sizeof(int));
        printf("Leaf node: average information gain too low (%.6f)\n", average_ig);
        for (int i = 0; i < class_count1; i++) free(unique_classes[i]);
        free(unique_classes);
        free(best_split.value);
        return;
    }

    // נמשיך לפיצול העץ
    printf("Splitting node on column %d with value %s\n", best_split.column_index, best_split.value);
    node->feature_index = best_split.column_index;
    node->left = create_node(0, NULL, 0, best_split.column_index, best_split.value[0]);
    node->right = create_node(0, NULL, 0, best_split.column_index, best_split.value[0]);
    node->is_leaf = 0; // לא עלה
    if (best_split.is_numeric)
    node->threshold = atof(best_split.value); // המר את המחרוזת למספר
    else
    node->threshold = -1; // או ערך אחר לסימון שאין סף מספרי

    global_depth++; // הגדלת העומק של העץ

    char *left_csv = create_temp_csv_filtered(csv_path, best_split.column_index, best_split.value, 1);  // <= לדוגמה: left <= value
    char *right_csv = create_temp_csv_filtered(csv_path, best_split.column_index, best_split.value, 0); // => right > value
    
    build_tree(node->left, left_csv);
    build_tree(node->right, right_csv);
    
    remove(left_csv);
    remove(right_csv);
    free(left_csv);
    free(right_csv);
    
    
    // במקרה שלא נעצרים, נמשיך הלאה — עדיין צריך לשחרר את unique_classes
    for (int i = 0; i < class_count1; i++) {
        free(unique_classes[i]);
    }
    free(unique_classes);
}


