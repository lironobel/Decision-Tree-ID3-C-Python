#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "dataset.h"
#include "infogain.h"
#include "utils.h"

int global_depth = 0;
double global_total_ig = 0.0;
int global_ig_count = 0;

// פונקציה בונה עץ החלטה על סמך קובץ
// היא קוראת את הקובץ, מחשבת את רווח המידע עבור כל עמודה,
// ומבצעת פיצול על סמך העמודה עם רווח המידע הגבוה ביותר
void build_tree(Node **node, FILE *file)
{
    rewind(file);
    if (*node == NULL)
        *node = create_node(1, NULL, 0, -1, -1.0, 1, NULL); // צומת התחלה כעלה זמני

    global_depth++;

    int column_count = count_columnsfunc(file);
    int total_rows = count_rows(file);

    int class_count = 0;
    char **list_classes = count_classes(file, &class_count);
    if (class_count == 0)
    {
        printf("No classes found in the file.\n");
        return;
    }

    int *class_counts_for_each_class = calloc(class_count, sizeof(int));
    count_rows_for_each_class(file, class_counts_for_each_class, list_classes, class_count);

    double base_entropy = entropy(class_counts_for_each_class, class_count);

    rewind(file);
    SplitResult best_split = find_best_infogain(file, column_count - 1, total_rows, base_entropy, class_count, list_classes);

    global_total_ig += best_split.gain;
    global_ig_count++;
    double average_ig = global_total_ig / global_ig_count;

    printf("Best Gain Overall: Column %d with Gain %.6f\n", best_split.column_index, best_split.gain);

    // תנאי עצירה
    if (best_split.gain < 0.01 || global_depth >= 5 || total_rows <= 10 || average_ig < 0.01 || class_count == 1)
    {
        (*node)->is_leaf = 1;
        (*node)->num_classes = class_count;
        (*node)->labels = calloc(class_count, sizeof(int));
        if (!(*node)->labels)
        {
            perror("calloc failed");
            exit(1);
        }

        for (int i = 0; i < class_count; i++)
            (*node)->labels[i] = class_counts_for_each_class[i];

        printf("Leaf node: stopping condition met (gain: %.4f, avg: %.4f)\n", best_split.gain, average_ig);
        goto cleanup;
    }

    // פיצול הקובץ לשני קבצים זמניים
    char left_filename[64], right_filename[64];
    sprintf(left_filename, "left_temp_%d.csv", global_depth);
    sprintf(right_filename, "right_temp_%d.csv", global_depth);

    rewind(file);
    FILE *left_f = create_temp_csv_filtered(file, best_split.column_index, best_split.value, 1, left_filename, best_split.is_numeric);
    rewind(file);
    FILE *right_f = create_temp_csv_filtered(file, best_split.column_index, best_split.value, 0, right_filename, best_split.is_numeric);

    // בדוק אם הקבצים נוצרו בהצלחה
    if (!left_f || !right_f)
    {
        printf("[ERROR] Failed to create filtered files for split.\n");
        if (left_f)
            fclose(left_f);
        if (right_f)
            fclose(right_f);
        remove(left_filename);
        remove(right_filename);
        goto cleanup;
    }

    int left_rows = count_rows(left_f);
    int right_rows = count_rows(right_f);

    if (left_rows == 0 || right_rows == 0)
    {
        printf("[ERROR] One of the filtered files has no data. Left: %d | Right: %d\n", left_rows, right_rows);
        fclose(left_f);
        fclose(right_f);
        remove(left_filename);
        remove(right_filename);
        goto cleanup;
    }

    printf("[INFO] Left rows: %d | Right rows: %d\n", left_rows, right_rows);
    printf("Splitting node on column %d with value %s\n", best_split.column_index, best_split.value);
    printf("[DEBUG] Depth: %d | Rows: %d | Column: %d | Value: %s | Type: %s\n",
           global_depth, total_rows, best_split.column_index, best_split.value,
           best_split.is_numeric ? "Numeric" : "Categorical");

    rewind(left_f);
    rewind(right_f);

    // עדכון פרטי הצומת הנוכחי
    (*node)->is_leaf = 0;
    (*node)->feature_index = best_split.column_index;
    (*node)->threshold = best_split.is_numeric ? atof(best_split.value) : -1.0;
    (*node)->is_numeric_split = best_split.is_numeric;
    (*node)->category_value = best_split.is_numeric ? NULL : strdup(best_split.value);

    // יצירת בנים
    (*node)->left = create_node(1, NULL, 0, -1, -1.0, 1, NULL);
    (*node)->right = create_node(1, NULL, 0, -1, -1.0, 1, NULL);

    // קריאה רקורסיבית לבנים
    build_tree(&((*node)->left), left_f);
    build_tree(&((*node)->right), right_f);

    fclose(left_f);
    fclose(right_f);
    remove(left_filename);
    remove(right_filename);

cleanup:
    global_depth--;
    free(best_split.value);
    free(class_counts_for_each_class);
    for (int i = 0; i < class_count; i++)
        free(list_classes[i]);
    free(list_classes);
}
