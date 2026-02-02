// הקריאה לפונקציות, שליטה על הזרימה
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataset.h"
#include "infogain.h"
#include "utils.h"
#include "tree.h"
#include "buildTree.h"
#include "tree_graph.h"
#include "pretictedvalues.h"

int main()
{
    char Path[] = "data\\iris.csv";
    MakeSure(Path); 

    // פתיחת הקובץ
    FILE *file = fopen(Path, "r");
    if (!file)
    {
        printf("Failed to open CSV file.\n");
        system("pause");
        return 0;
    }

    // קריאת המחלקות מהקובץ
    int class_count = 0;
    char **classes = count_classes(file, &class_count);
    if (classes == NULL || class_count == 0)
    {
        printf("No classes found in the file.\n");
        fclose(file);
        return 0;
    }

    // הדפסת המחלקות שנמצאו
    printf("Classes found (%d):\n", class_count);
    for (int i = 0; i < class_count; i++) { printf("- %s\n", classes[i]); }

    // בניית עץ ההחלטה
    printf("Building decision tree...\n");
    Node *root = NULL;
    build_tree(&root, file, 1, "Root");

    // יצירת קובץ התחזיות
    printf("Writing predictions to CSV...\n");
    write_predictions(root, Path, "predictions.csv", classes);
    // הערה: הסרתי את הפתיחה האוטומטית של ה-CSV כדי שלא יקפצו לך 20 חלונות בכל הרצה
    // אם אתה רוצה את זה, פשוט תחזיר את ה-system("start predictions.csv");

    // ספירת עמודות ויצירת שמות תכונות
    int column_count = count_columnsfunc(file);
    int allocated_columns = 0;
    char **feature_names_vector = create_and_print_feature_vector(file, column_count, &allocated_columns);

    // --- ייצוא ויזואלי של העץ (כאן השינוי המרכזי) ---
    const char *dot_filename = "tree.dot";
    const char *png_filename = "tree.png";
    
    FILE *dot_file = fopen(dot_filename, "w");
    if (dot_file)
    {
        printf("Exporting colored tree to DOT...\n");
        export_tree_to_dot(root, dot_file, feature_names_vector);
        fclose(dot_file);

        // שימוש בפונקציה החדשה שיצרנו ב-tree_graph.c
        // היא מטפלת גם בהרצה של Graphviz וגם בפתיחת התמונה
        generate_and_open_graph(dot_filename, png_filename);
    }
    else {
        printf("[ERROR] Could not create tree.dot\n");
    }

    // שחרור זיכרון
    printf("Cleaning up memory...\n");
    free_tree(root);
    fclose(file);

    for (int i = 0; i < column_count; i++) { free(feature_names_vector[i]); }
    for (int i = 0; i < class_count; i++) { free(classes[i]); }
    free(classes);

    printf("\nDone! Look at the generated tree image.\n");
    system("pause");
    return 0;
}