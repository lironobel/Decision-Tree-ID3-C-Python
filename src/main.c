// רק הקריאה לפונקציות, שליטה על הזרימה
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataset.h"
#include "infogain.h"
#include "utils.h"
#include "tree.h"
#include "build_tree.h"

int main()
{
    // פיצול ראשון בלבד

    // פתיחת הקובץ
    FILE *file = fopen("data\\iris.csv", "r");
    if (!file)
    {
        printf("Failed to open CSV file ");
        system("pause");
        return 0;
    }
    int column_count = 0;
    // קריאה לפונקציה שמחזירה את המערך של השמות של העמודות
    // על הדרך גם סופרת כמה עמודות שונות יש לנו
    // בגלל זה נשלח את המצביע של העמודות לפונקציה
    char **feature_vector = create_and_print_feature_vector(file, &column_count);

    int total_rows = 0; // מספר השורות בקובץ
    // קריאה לפונקציה שמחזירה את המערך של השמות של המחלקות
    int class_count = 0; // משתנה ששולחים לפונקצייה עם המצביע שערך שלו שומר כמה מחלקות שונות יש בקובץ
    char **list_classes = count_classes(file, &class_count, &total_rows);

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
    int column_countRigth = 0;
    int column_countLeft = 0;
    printf("\n\n[INFO] Splitting again...\n");

    rewind(left);  // מחזיר את המצביע של הקובץ להתחלה
    rewind(right); // מחזיר את המצביע של הקובץ להתחלה

    printf("feature_vector for right: \n");
    char **feature_vectorForRight = create_and_print_feature_vector(right, &column_countRigth); // קריאה לפונקציה שמחזירה את המערך של השמות של העמודות פיצול ימינה
    printf("feature_vector for left: \n");
    char **feature_vectorForLeft = create_and_print_feature_vector(left, &column_countLeft); // קריאה לפונקציה שמחזירה את המערך של השמות של העמודות פיצול שמאלה

    int total_rowsRigth = 0; // מספר השורות בקובץ
    int total_rowsLeft = 0;  // מספר השורות בקובץ
    // קריאה לפונקציה שמחזירה את המערך של השמות של המחלקות
    int class_countRigth = 0; // משתנה ששולחים לפונקצייה עם המצביע שערך שלו שומר כמה מחלקות שונות יש בקובץ
    int class_countLeft = 0;  // משתנה ששולחים לפונקצייה עם המצביע שערך שלו שומר כמה מחלקות שונות יש בקובץ

    char **list_classesRigth = count_classes(right, &class_countRigth, &total_rowsRigth);
    char **list_classesLeft = count_classes(left, &class_countLeft, &total_rowsLeft);

    // יצירת מערך שיעקוב אחרי מספר התצפיות עבור כל מחלקה
    int *class_counts_for_each_classRigth = (int *)calloc(class_countRigth, sizeof(int));
    int *class_counts_for_each_classLeft = (int *)calloc(class_countLeft, sizeof(int)); // מערך שיעקוב אחרי מספר התצפיות עבור כל מחלקה

    // ספירת התצפיות עבור כל מחלקה שונה בקובץ
    count_rows_for_each_class(left, class_counts_for_each_classLeft, list_classesLeft, class_countLeft);     // ספירת התצפיות עבור כל מחלקה שונה בקובץ השמאלי
    count_rows_for_each_class(right, class_counts_for_each_classRigth, list_classesRigth, class_countRigth); // ספירת התצפיות עבור כל מחלקה שונה בקובץ הימני

    // חישוב האנטרופיה
    double base_entropyRigth = entropy(class_counts_for_each_classRigth, class_countRigth); // שולח לו את המערך ששומר כמה תצפיות עבור כל מחלקה, שולח את המשתנה ששומר כמה מחלקות שונות ישנם בקובץ
    double base_entropyLeft = entropy(class_counts_for_each_classLeft, class_countLeft);    // שולח לו את המערך ששומר כמה תצפיות עבור כל מחלקה, שולח את המשתנה ששומר כמה מחלקות שונות ישנם בקובץ
    printf("Base Entropy for right side: %.6f\n", base_entropyRigth);                       // הדפסת האנטרופיה של הקובץ הימני
    printf("Base Entropy for left side: %.6f\n", base_entropyLeft);                         // הדפסת האנטרופיה של הקובץ השמאלי

    // חישוב ה-IG עבור כל עמודה
    SplitResult resultRigth = find_best_infogain(right, column_countRigth - 1, total_rowsRigth, base_entropyRigth, class_countRigth, list_classesRigth);
    SplitResult resultLeft = find_best_infogain(left, column_countLeft - 1, total_rowsLeft, base_entropyLeft, class_countLeft, list_classesLeft);
    printf("[DEBUG] splitting by column %d with threshold %s\n", resultRigth.column_index, resultRigth.value);
    printf("[DEBUG] splitting by column %d with threshold %s\n", resultLeft.column_index, resultLeft.value);
    // בדיקה האם יש פיצול אפשרי
    if (resultRigth.column_index == -1)
    {
        printf("[INFO] No valid split found for Left Side (Gain: %.6f)\n", resultRigth.gain);
    }
    else
    {
        printf("[INFO] Best Gain Overall for Left Side: Column %s with Gain %.6f\n",
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

    printf("[INFO] Best Split Value for Right: %s\n", resultRigth.value); // הדפסת ערך הסף הטוב ביותר  => זה יהיה הערך שאיתו נבצע את הפיצול
    printf("[INFO] Best Split Value for Left: %s\n", resultLeft.value);   // הדפסת ערך הסף הטוב ביותר  => זה יהיה הערך שאיתו נבצע את הפיצול

    // יצירת קבצים זמניים עבור הפיצול
    // הכנת השמות של הקבצים
    // הכנת השמות של הקבצים
    char left_left_filename[100], left_right_filename[100];   // שמות הקבצים לפיצול שמאלי
    char right_right_filename[100], right_left_filename[100]; // שמות הקבצים לפיצול ימני
    FILE *left_left = NULL, *left_right = NULL, *right_left = NULL, *right_right = NULL;

    printf("[DEBUG] Rows before left second split: %d\n", count_rows(left));
    // ===== פיצול צד שמאל =====
    if (resultLeft.value != NULL && resultLeft.column_index != -1)
    {
        // יצירת שמות לקבצים לפי ערך הפיצול השמאלי
        sprintf(left_left_filename, "is_%s.csv", resultLeft.value);
        sprintf(left_right_filename, "is_not_%s.csv", resultLeft.value);

        // החלפת תווים בעייתיים בשם הקובץ (רווחים ומקפים)
        for (int i = 0; left_left_filename[i]; i++)
            if (left_left_filename[i] == ' ' || left_left_filename[i] == '-')
                left_left_filename[i] = '_';
        for (int i = 0; left_right_filename[i]; i++)
            if (left_right_filename[i] == ' ' || left_right_filename[i] == '-')
                left_right_filename[i] = '_';

        // פתיחת קבצים כדי לבדוק האם העמודה היא מספרית
        FILE *left_file = fopen(left_filename, "r");
        FILE *right_file = fopen(right_filename, "r");
        int is_numericLeftleft = check_if_column_contains_numbers(left_file, resultLeft.column_index);
        int is_numericLeftRight = check_if_column_contains_numbers(right_file, resultLeft.column_index);
        fclose(left_file);
        fclose(right_file);

        // יצירת קבצי פלט עבור הפיצול השמאלי
        rewind(left);  // מחזיר את המצביע של הקובץ להתחלה
        rewind(right); // מחזיר את המצביע של הקובץ להתחלה
        left_left = create_temp_csv_filtered(left, resultLeft.column_index, resultLeft.value, 1, left_left_filename, is_numericLeftleft);
        left_right = create_temp_csv_filtered(left, resultLeft.column_index, resultLeft.value, 0, left_right_filename, is_numericLeftRight);

        // הדפסת סיכום פיצול שמאלי
        if (left_left && left_right)
        {
            printf("\nLeft rows for the left second split: %d\n", count_rows(left_left));
            printf("Right rows for the left second split: %d\n", count_rows(left_right));
            fclose(left_left);
            fclose(left_right);
            remove(left_left_filename);
            remove(left_right_filename);
        }
    }
    else
    {
        printf("[INFO] Skipping Left Split - No valid split found.\n");
    }

    printf("[DEBUG] Rows before right second split: %d\n", count_rows(right));
    // ===== פיצול צד ימין =====
    if (resultRigth.value != NULL && resultRigth.column_index != -1)
    {
        // יצירת שמות לקבצים לפי ערך הפיצול הימני
        sprintf(right_right_filename, "is_%s.csv", resultRigth.value);
        sprintf(right_left_filename, "is_not_%s.csv", resultRigth.value);

        // ניקוי תווים בעייתיים בשם הקובץ
        for (int i = 0; right_right_filename[i]; i++)
            if (right_right_filename[i] == ' ' || right_right_filename[i] == '-')
                right_right_filename[i] = '_';
        for (int i = 0; right_left_filename[i]; i++)
            if (right_left_filename[i] == ' ' || right_left_filename[i] == '-')
                right_left_filename[i] = '_';

        // פתיחת קבצים כדי לבדוק האם העמודה מספרית
        FILE *left_file = fopen(left_filename, "r");
        FILE *right_file = fopen(right_filename, "r");
        int is_numericRightleft = check_if_column_contains_numbers(left_file, resultRigth.column_index);
        int is_numericRightRight = check_if_column_contains_numbers(right_file, resultRigth.column_index);
        fclose(left_file);
        fclose(right_file);

        // יצירת קבצי פלט לפיצול הימני
        rewind(left);  // מחזיר את המצביע של הקובץ להתחלה
        rewind(right); // מחזיר את המצביע של הקובץ להתחלה
        right_left = create_temp_csv_filtered(right, resultRigth.column_index, resultRigth.value, 1, right_left_filename, is_numericRightleft);
        right_right = create_temp_csv_filtered(right, resultRigth.column_index, resultRigth.value, 0, right_right_filename, is_numericRightRight);

        // הדפסת סיכום פיצול ימני
        if (right_left && right_right)
        {
            printf("Left rows for the right second split: %d\n", count_rows(right_left));
            printf("Right rows for the right second split: %d\n", count_rows(right_right));
            fclose(right_left);
            fclose(right_right);
        }
    }
    else
    {
        printf("[INFO] Skipping Right Split - No valid split found.\n");
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

    for (int i = 0; i < column_count; i++)
        free(feature_vector[i]);
    free(feature_vector);

    for (int i = 0; i < class_count; i++)
        free(list_classes[i]);
    free(list_classes);

    free(class_counts_for_each_class);
    free(class_counts_for_each_classRigth);
    free(class_counts_for_each_classLeft);

    for (int i = 0; i < class_countRigth; i++)
        free(list_classesRigth[i]);
    free(list_classesRigth);

    for (int i = 0; i < class_countLeft; i++)
        free(list_classesLeft[i]);
    free(list_classesLeft);

    for (int i = 0; i < column_countRigth; i++)
        free(feature_vectorForRight[i]);
    free(feature_vectorForRight);

    for (int i = 0; i < column_countLeft; i++)
        free(feature_vectorForLeft[i]);
    free(feature_vectorForLeft);

    system("pause");
    return 0;
}