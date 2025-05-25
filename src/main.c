// רק הקריאה לפונקציות, שליטה על הזרימה
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

    // נתיב לקובץ dot של Graphviz
    const char *DOT_PATH = "C:\\Program Files\\Graphviz\\bin\\dot.exe"; // עדכן את הנתיב לפי המיקום האמיתי

    // פתיחת הקובץ
    FILE *file = fopen("data\\adult.csv", "r");
    if (!file)
    {
        printf("Failed to open CSV file ");
        system("pause");
        return 0;
    }

    // הדפסה של המחלקות הייחודיות בקובץ
    int class_count = 0;
    char **classes = count_classes(file, &class_count); // קבל את השמות והמספר
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
    }
    // שחרור זיכרון
    for (int i = 0; i < class_count; i++)
    {
        free(classes[i]);
    }
    free(classes);

    // קריאה לפונקציה שמבנה עץ החלטה
    printf("Building decision tree...\n");
    Node *root = NULL;
    build_tree(&root, file);

    int column_count = count_columnsfunc(file);                                   // קריאה לפונקציה שסופרת כמה עמודות שונות יש בקובץ
    char **feature_names_vector = (char **)malloc(sizeof(char *) * column_count); // הקצה זיכרון למערך של שמות תכונות

    int allocated_columns = 0;
    feature_names_vector = create_and_print_feature_vector(file, column_count, &allocated_columns);

    FILE *dot_file = fopen("tree.dot", "w");
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
    fclose(dot_file);
    for (int i = 0; i < column_count; i++)
    {
        free(feature_names_vector[i]);
    }

    // שני פיצולים איטרטיביים
    /*
    printf("Total Rows: %d\n", count_rows(file));
    // פונקצייה שסופרת כמה עמודות שונות יש בקובץ
    int column_count = count_columnsfunc(file);

    int allocated_columns = 0;
    // קריאה לפונקציה שמחזירה את המערך של השמות של העמודות
    char **feature_vector = create_and_print_feature_vector(file, column_count, &allocated_columns);

    // מספר השורות בקובץ
    int total_rows = count_rows(file);

    // קריאה לפונקציה שמחזירה את המערך של השמות של המחלקות
    int class_count = 0; // משתנה ששולחים לפונקצייה עם המצביע שערך שלו שומר כמה מחלקות שונות יש בקובץ
    char **list_classes = count_classes(file, &class_count);

    // יצירת מערך שיעקוב אחרי מספר התצפיות עבור כל מחלקה
    int *class_counts_for_each_class = (int *)calloc(class_count, sizeof(int));

    // ספירת התצפיות עבור כל מחלקה שונה
    count_rows_for_each_class(file, class_counts_for_each_class, list_classes, class_count);

    // חישוב האנטרופיה
    double base_entropy = entropy(class_counts_for_each_class, class_count); // שולח לו את המערך ששומר כמה תצפיות עבור כל מחלקה, שולח את המשתנה ששומר כמה מחלקות שונות ישנם בקובץ
    printf("Base Entropy: %.6f\n", base_entropy);

    // הדפסת כמות התצפיות עבור כל מחלקה
    for (int i = 0; i < class_count; i++)
    {
        printf("Class: %s, Count: %d\n", list_classes[i], class_counts_for_each_class[i]);
    }

    // חישוב ה-IG עבור כל עמודה
    SplitResult result = find_best_infogain(file, column_count - 1, total_rows, base_entropy, class_count, list_classes);

    printf("\n[INFO] Best Gain Overall: Column %s with Gain %.6f\n", feature_vector[result.column_index], result.gain); // הדפסת העמודה עם הרווח הטוב ביותר
    print_unique_values_in_column(file, result.column_index);                                                           // הדפסת הערכים  בעמודה
    printf("\n[INFO] Best Split Value: %s\n", result.value);                                                            // הדפסת ערך הסף הטוב ביותר  => זה יהיה הערך שאיתו נבצע את הפיצול

    // יצירת קבצים זמניים עבור הפיצול
    // הכנת השמות של הקבצים
    char left_filename[100], right_filename[100];           // מקצה מקום למחרוזת
    sprintf(left_filename, "is_%s.csv", result.value);      // הכנת השם של הקובץ השמאלי
    sprintf(right_filename, "is_not_%s.csv", result.value); // הכנת שם הקובץ הימני

    // ניקוי תווים בעייתיים בשם הקובץ
    for (int i = 0; left_filename[i]; i++)
    {
        if (left_filename[i] == ' ' || left_filename[i] == '-')
            left_filename[i] = '_';
    }
    for (int i = 0; right_filename[i]; i++)
    {
        if (right_filename[i] == ' ' || right_filename[i] == '-')
            right_filename[i] = '_';
    }

    // יצירת הקבצים
    int is_numeric = check_if_column_contains_numbers(file, result.column_index); // בדוק אם העמודה היא מספרית או קטגוריאלית

    FILE *left = create_temp_csv_filtered(file, result.column_index, result.value, 1, left_filename, is_numeric);
    FILE *right = create_temp_csv_filtered(file, result.column_index, result.value, 0, right_filename, is_numeric);

    if (!left || !right) // בדוק אם הפונקציה הצליחה ליצור את הקבצים
    {
        printf("Failed to create temp files\n");
        fclose(file);
        return 1;
    }
    fclose(left);
    fclose(right);

    // פתח מחדש לקריאה
    left = fopen(left_filename, "r");
    right = fopen(right_filename, "r");

    printf("Left rows for the first split: %d\n", count_rows(left));
    printf("Right rows for the first split: %d\n", count_rows(right));

    // עד כאן פיצול ראשון

    // פיצול שני

    int column_countRigth = count_columnsfunc(right);
    int column_countLeft = count_columnsfunc(left);
    printf("\n\n[INFO] Splitting again...\n\n");

    rewind(left);
    rewind(right);

    int allocated_columnsforRight = 0;
    int allocated_columnsforLeft = 0;
    printf("feature_vector for right: \n");
    char **feature_vectorForRight = create_and_print_feature_vector(right, column_countRigth, &allocated_columnsforRight);
    printf("feature_vector for left: \n");
    char **feature_vectorForLeft = create_and_print_feature_vector(left, column_countLeft,  &allocated_columnsforLeft);

    int total_rowsRigth = count_rows(right);
    int total_rowsLeft = count_rows(left);
    int class_countRigth = 0;
    int class_countLeft = 0;

    char **list_classesRigth = count_classes(right, &class_countRigth);
    char **list_classesLeft = count_classes(left, &class_countLeft);

    int *class_counts_for_each_classRigth = (int *)calloc(class_countRigth, sizeof(int));
    int *class_counts_for_each_classLeft = (int *)calloc(class_countLeft, sizeof(int));

    count_rows_for_each_class(left, class_counts_for_each_classLeft, list_classesLeft, class_countLeft);
    count_rows_for_each_class(right, class_counts_for_each_classRigth, list_classesRigth, class_countRigth);

    double base_entropyRigth = entropy(class_counts_for_each_classRigth, class_countRigth);
    double base_entropyLeft = entropy(class_counts_for_each_classLeft, class_countLeft);
    printf("Base Entropy for right side: %.6f\n", base_entropyRigth);
    printf("Base Entropy for left side: %.6f\n\n", base_entropyLeft);

    SplitResult resultRigth = find_best_infogain(right, column_countRigth - 1, total_rowsRigth, base_entropyRigth, class_countRigth, list_classesRigth);
    SplitResult resultLeft = find_best_infogain(left, column_countLeft - 1, total_rowsLeft, base_entropyLeft, class_countLeft, list_classesLeft);

    if (resultRigth.column_index == -1)
    {
        printf("[INFO] No valid split found for Right Side (Gain: %.6f)\n", resultRigth.gain);
    }
    else
    {
        printf("[INFO] Best Gain Overall for Right Side: Column %s with Gain %.6f\n",
               feature_vectorForRight[resultRigth.column_index], resultRigth.gain);
    }
    if (resultLeft.column_index == -1)
    {
        printf("[INFO] No valid split found for Left Side (Gain: %.6f)\n", resultLeft.gain);
    }
    else
    {
        printf("[INFO] Best Gain Overall for Left Side: Column %s with Gain %.6f\n",
               feature_vectorForLeft[resultLeft.column_index], resultLeft.gain);
    }

    printf("[INFO] Best Split Value for Right: %s\n", resultRigth.value);
    printf("[INFO] Best Split Value for Left: %s\n\n", resultLeft.value);

    char left_left_filename[100], left_right_filename[100];
    char right_right_filename[100], right_left_filename[100];
    FILE *left_left = NULL, *left_right = NULL, *right_left = NULL, *right_right = NULL;

    printf("[DEBUG] Rows before left second split: %d\n", count_rows(left));
    if (resultLeft.value != NULL && resultLeft.column_index != -1)
    {
        sprintf(left_left_filename, "is_%s.csv", resultLeft.value);
        sprintf(left_right_filename, "is_not_%s.csv", resultLeft.value);
        for (int i = 0; left_left_filename[i]; i++)
            if (left_left_filename[i] == ' ' || left_left_filename[i] == '-')
                left_left_filename[i] = '_';
        for (int i = 0; left_right_filename[i]; i++)
            if (left_right_filename[i] == ' ' || left_right_filename[i] == '-')
                left_right_filename[i] = '_';

        // פתיחה אחת בלבד לצורך בדיקה
        FILE *left_file = fopen(left_filename, "r");
        int is_numericLeft = check_if_column_contains_numbers(left_file, resultLeft.column_index);
        fclose(left_file);

        // פיצול בפועל - שתי קריאות עם אותו is_numericLeft
        FILE *left_copy1 = fopen(left_filename, "r");
        FILE *left_copy2 = fopen(left_filename, "r");

        left_left = create_temp_csv_filtered(left_copy1, resultLeft.column_index, resultLeft.value, 1, left_left_filename, is_numericLeft);
        left_right = create_temp_csv_filtered(left_copy2, resultLeft.column_index, resultLeft.value, 0, left_right_filename, is_numericLeft);

        fclose(left_copy1);
        fclose(left_copy2);

        if (left_left && left_right)
        {
            fclose(left_left);
            fclose(left_right);

            left_left = fopen(left_left_filename, "r");
            left_right = fopen(left_right_filename, "r");

            printf("Left rows for the left second split: %d\n", count_rows(left_left));
            printf("Right rows for the left second split: %d\n\n", count_rows(left_right));

            fclose(left_left);
            fclose(left_right);
            remove(left_left_filename);
            remove(left_right_filename);
        }
    }
    else
    {
        printf("[INFO] Skipping Left Split - No valid split found.\n"); // אם לא נמצא פיצול תקין, לא נבצע את הפיצול
    }

    printf("[DEBUG] Rows before right second split: %d\n", count_rows(right));
    if (resultRigth.value != NULL && resultRigth.column_index != -1)
    {
        sprintf(right_right_filename, "is_%s.csv", resultRigth.value);
        sprintf(right_left_filename, "is_not_%s.csv", resultRigth.value);
        for (int i = 0; right_right_filename[i]; i++)
            if (right_right_filename[i] == ' ' || right_right_filename[i] == '-')
                right_right_filename[i] = '_';
        for (int i = 0; right_left_filename[i]; i++)
            if (right_left_filename[i] == ' ' || right_left_filename[i] == '-')
                right_left_filename[i] = '_';

        // בדיקה אחת בלבד אם העמודה מספרית
        FILE *right_file = fopen(right_filename, "r");
        int is_numericRight = check_if_column_contains_numbers(right_file, resultRigth.column_index);
        fclose(right_file);

        // פיצול בפועל על שני עותקים של הקובץ
        FILE *right_copy1 = fopen(right_filename, "r");
        FILE *right_copy2 = fopen(right_filename, "r");

        right_left = create_temp_csv_filtered(right_copy1, resultRigth.column_index, resultRigth.value, 1, right_left_filename, is_numericRight);
        right_right = create_temp_csv_filtered(right_copy2, resultRigth.column_index, resultRigth.value, 0, right_right_filename, is_numericRight);

        fclose(right_copy1);
        fclose(right_copy2);

        if (right_left && right_right)
        {
            fclose(right_left);
            fclose(right_right);
            right_left = fopen(right_left_filename, "r");
            right_right = fopen(right_right_filename, "r");
            printf("Left rows for the right second split: %d\n", count_rows(right_left));
            printf("Right rows for the right second split: %d\n\n", count_rows(right_right));
            fclose(right_left);
            fclose(right_right);
        }
    }
    else
    {
        printf("[INFO] Skipping Right Split - No valid split found.\n"); // אם לא נמצא פיצול תקין, לא נבצע את הפיצול
    }

    // שחרור זיכרון
    fclose(file);
    fclose(left);
    fclose(right);
    remove(left_filename);
    remove(right_filename);
    remove(right_left_filename);
    remove(right_right_filename);
    remove(left_left_filename);
    remove(left_right_filename);

    if (feature_vector)
    {
        for (int i = 0; i < allocated_columns; i++)
            if (feature_vector[i])
                free(feature_vector[i]);
        free(feature_vector);
    }

    if (list_classes)
    {
        for (int i = 0; i < class_count; i++)
            free(list_classes[i]);
        free(list_classes);
    }

    free(class_counts_for_each_class);
    free(class_counts_for_each_classRigth);
    free(class_counts_for_each_classLeft);

    for (int i = 0; i < class_countRigth; i++)
        free(list_classesRigth[i]);
    free(list_classesRigth);

    if (list_classesLeft)
    {
        for (int i = 0; i < class_countLeft; i++)
            if (list_classesLeft[i])
                free(list_classesLeft[i]);
        free(list_classesLeft);
    }

    for (int i = 0; i < allocated_columnsforRight; i++)
        free(feature_vectorForRight[i]);
    free(feature_vectorForRight);

    for (int i = 0; i < allocated_columnsforLeft; i++)
        free(feature_vectorForLeft[i]);
    free(feature_vectorForLeft);
    */

    system("pause");

    return 0;
}