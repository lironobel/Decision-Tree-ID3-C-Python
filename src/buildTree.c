// build_tree.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "dataset.h"
#include "infogain.h"
#include "utils.h"

// משתנים גלובליים לסטטיסטיקה
double global_total_ig = 0.0;
int global_ig_count = 0;
int tree_max_depth_reached = 0;

// פונקציה בונה עץ החלטה
// node - מצביע לצומת הנוכחי בעץ
// file - קובץ הנתונים
// depth - עומק הפיצול הנוכחי
// side - הצד הנוכחי: "Left", "Right" או "Root"
int build_tree(
    Node **node,
    FILE *file,
    int depth,
    const char *side,
    int max_depth_limit,
    char **class_names,
    int class_count)
{
    rewind(file);

    if (*node == NULL)
        *node = create_node(1, NULL, 0, -1, -1.0, 1, NULL); // יצירת צומת זמני

    int column_count = count_columnsfunc(file);
    int total_rows = count_rows(file);

    if (class_count == 0 || class_names == NULL)
    {
        printf("No classes found in the file.\n");
        return 0;
    }

    int *class_counts_for_each_class = calloc(class_count, sizeof(int));
    count_rows_for_each_class(file, class_counts_for_each_class, class_names, class_count);

    int active_class_count = 0;
    for (int i = 0; i < class_count; i++)
    {
        if (class_counts_for_each_class[i] > 0)
            active_class_count++;
    }

    double base_entropy = entropy(class_counts_for_each_class, class_count);

    rewind(file);
    SplitResult best_split = find_best_infogain(file, column_count - 1, total_rows, base_entropy, class_count, class_names);

    global_total_ig += best_split.gain;
    global_ig_count++;
    double average_ig = global_total_ig / global_ig_count;

    // הדפסת מידע על הפיצול
    printf("\n[INFO] Depth: %d | Side: %s | Best Gain: Column %d with Gain %.6f\n", depth, side, best_split.column_index, best_split.gain);

    // תנאי עצירה
    if (best_split.gain < 0.01 || depth >= max_depth_limit || total_rows <= 10 || average_ig < 0.01 || active_class_count <= 1)
    {
        // שמירת העומק הכי גבוהה שהגענו אליו
        if (depth > tree_max_depth_reached)
            tree_max_depth_reached = depth;

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

        printf("[LEAF] Depth: %d | Side: %s | Stopping (Gain: %.4f, Avg: %.4f)\n", depth, side, best_split.gain, average_ig);
        goto cleanup;
    }

    // פיצול לקבצים זמניים
    char left_filename[64], right_filename[64];
    sprintf(left_filename, "left_temp_%d.csv", depth);
    sprintf(right_filename, "right_temp_%d.csv", depth);

    rewind(file);
    FILE *left_f = create_temp_csv_filtered(file, best_split.column_index, best_split.value, 1, left_filename, best_split.is_numeric);
    rewind(file);
    FILE *right_f = create_temp_csv_filtered(file, best_split.column_index, best_split.value, 0, right_filename, best_split.is_numeric);

    // בדוק אם הקבצים נוצרו בהצלחה
    if (!left_f || !right_f)
    {
        printf("[ERROR] Failed to create filtered files.\n");
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
        printf("[ERROR] One of the splits is empty. Left: %d | Right: %d\n", left_rows, right_rows);
        fclose(left_f);
        fclose(right_f);
        remove(left_filename);
        remove(right_filename);
        goto cleanup;
    }

    printf("[SPLIT] Depth: %d | Side: %s | Column: %d | Value: %s | Type: %s\n",
           depth, side, best_split.column_index, best_split.value,
           best_split.is_numeric ? "Numeric" : "Categorical");

    rewind(left_f);
    rewind(right_f);

    (*node)->is_leaf = 0;
    (*node)->feature_index = best_split.column_index;
    (*node)->threshold = best_split.is_numeric ? atof(best_split.value) : -1.0;
    (*node)->is_numeric_split = best_split.is_numeric;
    (*node)->category_value = best_split.is_numeric ? NULL : strdup(best_split.value);

    // עדכון labels של צומת split עם התפלגות המחלקות הנוכחית
    (*node)->num_classes = class_count;
    (*node)->labels = calloc(class_count, sizeof(int));
    if (!(*node)->labels)
    {
        perror("calloc failed");
        exit(1);
    }
    for (int i = 0; i < class_count; i++)
        (*node)->labels[i] = class_counts_for_each_class[i];

    (*node)->left = create_node(1, NULL, 0, -1, -1.0, 1, NULL);
    (*node)->right = create_node(1, NULL, 0, -1, -1.0, 1, NULL);

    // קריאה רקורסיבית עם עדכון הצדדים
    build_tree(&((*node)->left), left_f, depth + 1, "Left", max_depth_limit, class_names, class_count);
    build_tree(&((*node)->right), right_f, depth + 1, "Right", max_depth_limit, class_names, class_count);

    fclose(left_f);
    fclose(right_f);
    remove(left_filename);
    remove(right_filename);

cleanup:
    free(best_split.value);
    free(class_counts_for_each_class);

    return tree_max_depth_reached; // מחזיר את העומק המקסימלי שהגענו אליו
}
