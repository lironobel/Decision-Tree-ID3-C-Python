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

int main()
{
    MakeSure("data\\adult.csv"); // בדוק אם הקובץ קיים ותקין

    const char *DOT_PATH = "C:\\Program Files\\Graphviz\\bin\\dot.exe"; // עדכן את הנתיב לפי המיקום האמיתי

    // פתיחת הקובץ
    FILE *file = fopen("data\\adult.csv", "r");
    if (!file)
    {
        printf("Failed to open CSV file ");
        system("pause");
        return 0;
    }

    // הדפסת מחלקות ייחודיות
    int class_count = 0;
    char **classes = count_classes(file, &class_count);
    if (classes == NULL || class_count == 0)
    {
        printf("No classes found in the file.\n");
        fclose(file);
        return 0;
    }
    printf("Classes found (%d):\n", class_count);
    for (int i = 0; i < class_count; i++)
    {
        printf("- %s\n", classes[i]);
        free(classes[i]);
    }
    free(classes);

    printf("Building decision tree...\n");
    Node *root = NULL;
    build_tree(&root, file, 1, "Root");

    int column_count = count_columnsfunc(file);
    int allocated_columns = 0;
    char **feature_names_vector = create_and_print_feature_vector(file, column_count, &allocated_columns);

    // כתיבת עץ לקובץ DOT במיקום מלא
    const char *dot_path = "C:\\Users\\liron\\Desktop\\Final-project\\VScode\\Project\\Final-project\\tree.dot";
    FILE *dot_file = fopen(dot_path, "w");
    if (dot_file)
    {
        export_tree_to_dot(root, dot_file, feature_names_vector);
        fclose(dot_file);

       // הפקת תמונה
        char command[512];
        sprintf(command, "\"%s\" -Tpng tree.dot -o tree.png", DOT_PATH);
        int result = system(command);
        if (result != 0)
        {
            printf("[ERROR] dot command failed with code: %d\n", result);
        }

        // פתיחה אוטומטית של התמונה
        if (fopen("tree.png", "r") != NULL)
        {
            system("start tree.png");
        }
        else
        {
            printf("[ERROR] tree.png not found when trying to open.\n");
        }
    }

    // שחרור זיכרון
    free_tree(root);
    fclose(file);
    for (int i = 0; i < column_count; i++)
    {
        free(feature_names_vector[i]);
    }

    system("pause");
    return 0;
}
