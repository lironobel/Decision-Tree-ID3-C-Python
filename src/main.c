// רק הקריאה לפונקציות, שליטה על הזרימה

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataset.h"
#include "infogain.h"
#include "utils.h"
#include "tree.h"

int main()
{
    // פתיחת הקובץ
    FILE *file = NULL;
    open_file_for_reading(&file, "data\\adult.csv"); // פותח את הקובץ לקריאה

    int column_count = 0;
    // קריאה לפונקציה שמחזירה את המערך של השמות של העמודות
    char **feature_vector = create_and_print_feature_vector(file, &column_count);

    // משתנים לשמירת שמות המחלקות
    int class_count = 0; // מספר המחלקות השונות
    int total_rows = 0;  // מספר השורות בקובץ

    // קריאה לפונקציה שמחזירה את המערך של השמות של המחלקות
    int class_count = 0;
    char **list_classes = count_classes(file, &class_count, &total_rows);
    
    // יצירת מערך שיעקוב אחרי מספר התצפיות עבור כל מחלקה
    int *class_counts_for_each_class = (int *)calloc(class_count, sizeof(int));

    // ספירת התצפיות עבור כל מחלקה
    count_rows_for_each_class(file, class_counts_for_each_class, list_classes, class_count);

    // חישוב האנטרופיה
    double base_entropy = entropy(class_counts_for_each_class, class_count);
    printf("Base Entropy: %.6f\n", base_entropy);

    // הדפסת כמות התצפיות עבור כל מחלקה
    for (int i = 0; i < class_count; i++)
    {
        printf("Class: %s, Count: %d\n", list_classes[i], class_counts_for_each_class[i]);
    }

    // חישוב ה-IG עבור כל עמודה
    double *best_gain_for_each_column = (double *)malloc(column_count * sizeof(double)); // מערך ששומר את ערך הרווח הטוב ביותר עבור כל עמודה
    if (!best_gain_for_each_column)                                                      // בדוק אם ההקצאה לא הצליחה
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    int maxmaxinfoINDEX = 0;                                                   // משתנה לשמירת העמודה עם הרווח המידע הגבוה ביותר
    double maxmaxinfo = 0.0;                                                   // משתנה לשמירת הרווח המידע הגבוה ביותר
    char **best_split_values = (char **)malloc(column_count * sizeof(char *)); // מערך לשמירת ערכי הספים הטובים ביותר עבור כל עמודה
    if (!best_split_values)                                                    // בדוק אם ההקצאה לא הצליחה
    {
        printf("Memory allocation failed\n");
        free(best_gain_for_each_column);
        return 1;
    }

    // חישוב ה-IG עבור כל עמודה
    find_best_infogain(file, column_count - 1, total_rows, best_gain_for_each_column, base_entropy, class_count, list_classes, &maxmaxinfo, &maxmaxinfoINDEX, best_split_values);
    printf("\n[INFO] Best Gain Overall: Column %s with Gain %.6f\n", feature_vector[maxmaxinfoINDEX], maxmaxinfo);
    print_unique_values_in_column(file, maxmaxinfoINDEX);                          // הדפסת הערכים  בעמודה
    printf("\n[INFO] Best Split Value: %s\n", best_split_values[maxmaxinfoINDEX]); // הדפסת ערך הסף הטוב ביותר  => זה יהיה הערך שאיתו נבצע את הפיצול

    // יצירת קבצים זמניים עבור הפיצול
    // הכנת השמות של הקבצים
    char left_filename[100], right_filename[100]; // מקצה מקום למחרוזת
    sprintf(left_filename, "is_%s.csv", best_split_values[maxmaxinfoINDEX]);
    sprintf(right_filename, "is_not_%s.csv", best_split_values[maxmaxinfoINDEX]);

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
    int is_numeric = check_if_column_contains_numbers(file, maxmaxinfoINDEX);

    FILE *left = create_temp_csv_filtered(file, maxmaxinfoINDEX, best_split_values[maxmaxinfoINDEX], 1, left_filename, is_numeric);
    FILE *right = create_temp_csv_filtered(file, maxmaxinfoINDEX, best_split_values[maxmaxinfoINDEX], 0, right_filename, is_numeric);

    fclose(left);
    fclose(right);

    // פתח מחדש לקריאה
    left = fopen(left_filename, "r");
    right = fopen(right_filename, "r");

    printf("Left rows for the first split: %d\n", count_rows(left));
    printf("Right rows for the first split: %d\n", count_rows(right));

    rewind(left);

    // שחרור זיכרון
    fclose(file);
    fclose(left);
    fclose(right);
    remove(left_filename);
    remove(right_filename);

    for (int i = 0; i < class_count; i++)
        if (best_split_values[i])
            free(best_split_values[i]);
    free(best_split_values);

    for (int i = 0; i < column_count; i++)
        free(feature_vector[i]);
    free(feature_vector);

    for (int i = 0; i < class_count; i++)
        free(list_classes[i]);
    free(list_classes);

    free(class_counts_for_each_class);
    free(best_gain_for_each_column);

    return 0;
}