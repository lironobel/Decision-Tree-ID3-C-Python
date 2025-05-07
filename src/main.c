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
    
    //פיצול ראשון בלבד
    // פתיחת הקובץ
    FILE *file = fopen("data\\imkad.csv", "r");
    if (!file) {
      printf("Failed to open CSV file ");
        system("pause");
      return 0;
    }
int column_count = 0;
    // קריאה לפונקציה שמחזירה את המערך של השמות של העמודות 
    //על הדרך גם סופרת כמה עמודות שונות יש לנו
    //בגלל זה נשלח את המצביע של העמודות לפונקציה
    char **feature_vector = create_and_print_feature_vector(file, &column_count);

    int total_rows = 0;  // מספר השורות בקובץ
    // קריאה לפונקציה שמחזירה את המערך של השמות של המחלקות
    int class_count = 0; // משתנה ששולחים לפונקצייה עם המצביע שערך שלו שומר כמה מחלקות שונות יש בקובץ
    char **list_classes = count_classes(file, &class_count, &total_rows);
    
    // יצירת מערך שיעקוב אחרי מספר התצפיות עבור כל מחלקה
    int *class_counts_for_each_class = (int *)calloc(class_count, sizeof(int));

    // ספירת התצפיות עבור כל מחלקה שונה
    count_rows_for_each_class(file, class_counts_for_each_class, list_classes, class_count);

    // חישוב האנטרופיה
    double base_entropy = entropy(class_counts_for_each_class, class_count);// שולח לו את המערך ששומר כמה תצפיות עבור כל מחלקה, שולח את המשתנה ששומר כמה מחלקות שונות ישנם בקובץ
    printf("Base Entropy: %.6f\n", base_entropy);

    // הדפסת כמות התצפיות עבור כל מחלקה
    for (int i = 0; i < class_count; i++)
    {
        printf("Class: %s, Count: %d\n", list_classes[i], class_counts_for_each_class[i]);
    }


    // חישוב ה-IG עבור כל עמודה
    SplitResult result = find_best_infogain(file, column_count - 1, total_rows, base_entropy, class_count, list_classes);

    printf("\n[INFO] Best Gain Overall: Column %s with Gain %.6f\n", feature_vector[result.column_index], result.gain); // הדפסת העמודה עם הרווח הטוב ביותר
    print_unique_values_in_column(file, result.column_index);                          // הדפסת הערכים  בעמודה
    printf("\n[INFO] Best Split Value: %s\n", result.value);    // הדפסת ערך הסף הטוב ביותר  => זה יהיה הערך שאיתו נבצע את הפיצול
    
    // יצירת קבצים זמניים עבור הפיצול
    // הכנת השמות של הקבצים
    char left_filename[100], right_filename[100]; // מקצה מקום למחרוזת
    sprintf(left_filename, "is_%s.csv", result.value);  // הכנת השם של הקובץ השמאלי
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

    rewind(left);// מחזיר את המצביע של הקובץ להתחלה
    rewind(right);// מחזיר את המצביע של הקובץ להתחלה
















    system("pause");
    // שחרור זיכרון
    fclose(file);
    fclose(left);
    fclose(right);
    remove(left_filename);
    remove(right_filename);

    for (int i = 0; i < column_count; i++)
        free(feature_vector[i]);
    free(feature_vector);

    for (int i = 0; i < class_count; i++)
        free(list_classes[i]);
    free(list_classes);

    free(class_counts_for_each_class);

   return 0;    
}