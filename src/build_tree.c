#include "tree.h"
#include "dataset.h"
int global_depth = 0;  // משתנה גלובלי לשמירת העומק של העץ

//משתנים בשביל חישוב IG ממוצע עד כה
double global_total_ig = 0.0; // משתנה גלובלי לשמירת סך ה-IG //זה כדי לקבוע האם כדאי לעשות עצירה באלגוריתם או להמשיך בעץ 
int global_ig_count = 0;    // משתנה גלובלי לשמירת כמות ה-IG

void build_tree(Node *node, const char *csv_path) {  // 
    int class_count1 = 0, total_rows = 0;
    char **unique_classes = count_classes(csv_path, &class_count1, &total_rows);// כך נשנה את הערך של המשתנה שסופר כמה מחלקות שונות יש בקובץ בשביל התנאי עצירה הראשון
    
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
        return;
    }

    //תנאי עצירה שני
    //ניצור צומת עלה אם הרווח מידע של הקובץ נמוך מידי ולא כדאי להמשיך עוד פיצול
    



    // במקרה שלא נעצרים, נמשיך הלאה — עדיין צריך לשחרר את unique_classes
    for (int i = 0; i < class_count1; i++) {
        free(unique_classes[i]);
    }
    free(unique_classes);
}


