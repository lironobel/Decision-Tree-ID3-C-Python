#include "tree.h"
#include "dataset.h"
#include "infogain.h"

int global_depth = 0;
double global_total_ig = 0.0;
int global_ig_count = 0;

void build_tree(Node *node, FILE *f) {
    int class_count1 = 0, total_rows = 0;
    rewind(f); // תמיד נחזור להתחלה לפני קריאה
    char **unique_classes = count_classes(f, &class_count1, &total_rows);

    if (node == NULL) {
        node = create_node(0, NULL, 0, -1, -1);
    }

    global_depth++;
    rewind(f);
    SplitResult best_split = find_best_infogain(f, class_count1, total_rows, global_total_ig, class_count1, unique_classes);

    global_total_ig += best_split.gain;
    global_ig_count++;
    double average_ig = global_total_ig / global_ig_count;

    printf("Best Gain Overall: Column %d with Gain %.6f\n", best_split.column_index, best_split.gain);

    // תנאי עצירה 1
    if (class_count1 == 1) {
        node->is_leaf = 1;
        node->num_classes = 1;
        node->labels = malloc(sizeof(int));
        node->labels[0] = atoi(unique_classes[0]);
        printf("Leaf node: all examples belong to class %s\n", unique_classes[0]);
        goto cleanup;
    }

    // תנאי עצירה 2–5
    if (best_split.gain < 0.01 || global_depth >= 5 || total_rows <= 10 || average_ig < 0.01) {
        node->is_leaf = 1;
        node->num_classes = class_count1;
        node->labels = calloc(class_count1, sizeof(int));
        printf("Leaf node: stopping condition met (gain: %.4f, avg: %.4f)\n", best_split.gain, average_ig);
        goto cleanup;
    }

    // פיצול
    printf("Splitting node on column %d with value %s\n", best_split.column_index, best_split.value);
    node->feature_index = best_split.column_index;
    node->is_leaf = 0;
    node->threshold = best_split.is_numeric ? atof(best_split.value) : -1;

    char left_filename[] = "left_temp.csv";
    char right_filename[] = "right_temp.csv";

    rewind(f);
    create_temp_csv_filtered(f, best_split.column_index, best_split.value, 1, left_filename, best_split.is_numeric);
    rewind(f);
    create_temp_csv_filtered(f, best_split.column_index, best_split.value, 0, right_filename, best_split.is_numeric);

    FILE *left_f = fopen(left_filename, "r");
    FILE *right_f = fopen(right_filename, "r");
    if (!left_f || !right_f) {
        perror("Failed to open temp split files");
        exit(1);
    }

    node->left = create_node(0, NULL, 0, best_split.column_index, node->threshold);
    node->right = create_node(0, NULL, 0, best_split.column_index, node->threshold);

    build_tree(node->left, left_f);
    build_tree(node->right, right_f);

    fclose(left_f);
    fclose(right_f);
    remove(left_filename);
    remove(right_filename);

cleanup:
    for (int i = 0; i < class_count1; i++) free(unique_classes[i]);
    free(unique_classes);
    free(best_split.value);
}
