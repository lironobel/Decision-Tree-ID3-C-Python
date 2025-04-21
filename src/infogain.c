// === infogain.c ===
// חישוב Information Gain וספים בצורה יעילה עם הדפסות דיבוג + שמירת ערך הסף הטוב ביותר

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "infogain.h"
#include "utils.h"
#include "dataset.h"

void calculate_categorical_thresholds(FILE *file, int column_index, double *best_gain, int total_rows, char **list_classes, int class_count, double base_entropy, char **best_category)
{
    rewind(file);
    char line[1024];
    char *categories[100];
    int num_categories = 0;
    double max_gain = 0.0;
    char *best_value = NULL;

    rewind(file);
    fgets(line, sizeof(line), file); // דילוג על כותרת

    while (fgets(line, sizeof(line), file))
    {
        char *copy = strdup(line);
        char *token = strtok(copy, ",");
        char *category = NULL;
        char *class_val = NULL;
        int col = 0;

        while (token)
        {
            if (col == column_index)
                category = token;
            class_val = token;
            token = strtok(NULL, ",");
            col++;
        }

        trim_inplace(category);
        trim_inplace(class_val);

        int exists = 0;
        for (int i = 0; i < num_categories; i++)
        {
            if (strcmp(categories[i], category) == 0)
            {
                exists = 1;
                break;
            }
        }
        if (!exists)
        {
            categories[num_categories++] = strdup(category);
        }
        free(copy);
    }

    for (int i = 0; i < num_categories; i++)
    {
        rewind(file);
        fgets(line, sizeof(line), file);

        int *left = calloc(class_count, sizeof(int));
        int *right = calloc(class_count, sizeof(int));
        int total_left = 0, total_right = 0;

        while (fgets(line, sizeof(line), file))
        {
            char *copy = strdup(line);
            char *token = strtok(copy, ",");
            char *category = NULL;
            char *class_val = NULL;
            int col = 0;

            while (token)
            {
                if (col == column_index)
                    category = token;
                class_val = token;
                token = strtok(NULL, ",");
                col++;
            }

            trim_inplace(category);
            trim_inplace(class_val);

            for (int j = 0; j < class_count; j++)
            {
                if (strcmp(class_val, list_classes[j]) == 0)
                {
                    if (strcmp(category, categories[i]) == 0)
                    {
                        left[j]++;
                        total_left++;
                    }
                    else
                    {
                        right[j]++;
                        total_right++;
                    }
                    break;
                }
            }
            free(copy);
        }

        double H_left = entropy(left, class_count);
        double H_right = entropy(right, class_count);
        double gain = base_entropy - ((total_left * H_left + total_right * H_right) / (double)total_rows);

        if (gain > max_gain)
        {
            max_gain = gain;
            best_value = categories[i];
        }

        free(left);
        free(right);
    }

    *best_gain = max_gain;
    *best_category = strdup(best_value);

    for (int i = 0; i < num_categories; i++)
        free(categories[i]);
    rewind(file);
}

void calculate_numeric_thresholds(FILE *file, int column_index, double *thresholds, int *threshold_count, int total_rows)
{
    rewind(file);
    char line[1024];
    double *column_values = malloc(total_rows * sizeof(double));
    if (!column_values)
        return;

    rewind(file);
    fgets(line, sizeof(line), file);

    int row = 0;
    while (fgets(line, sizeof(line), file) && row < total_rows)
    {
        char *token = strtok(line, ",");
        int col = 0;
        while (token)
        {
            if (col == column_index)
            {
                column_values[row] = atof(token);
                break;
            }
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }

    qsort(column_values, total_rows, sizeof(double), compare_doubles);
    int t = 0;
    for (int i = 1; i < total_rows; i++)
    {
        if (column_values[i] != column_values[i - 1])
        {
            thresholds[t++] = (column_values[i] + column_values[i - 1]) / 2.0;
        }
    }
    *threshold_count = t;
    free(column_values);
    rewind(file);
}

void split_by_numeric_threshold(FILE *file, int column_index, double threshold, char **list_classes, int class_count, int *left_counts, int *right_counts, int *total_left, int *total_right)
{
    rewind(file);
    char line[1024];
    rewind(file);
    fgets(line, sizeof(line), file);

    *total_left = 0;
    *total_right = 0;

    while (fgets(line, sizeof(line), file))
    {
        char line_copy[1024];
        strcpy(line_copy, line);

        char *token = strtok(line, ",");
        int col = 0;
        double value = 0;

        while (token)
        {
            if (col == column_index)
            {
                value = atof(token);
                break;
            }
            token = strtok(NULL, ",");
            col++;
        }

        char *class_value = strrchr(line_copy, ',');
        if (class_value)
        {
            class_value = class_value + 1;
            trim_inplace(class_value);

            for (int i = 0; i < class_count; i++)
            {
                if (strcmp(class_value, list_classes[i]) == 0)
                {
                    if (value <= threshold)
                    {
                        left_counts[i]++;
                        (*total_left)++;
                    }
                    else
                    {
                        right_counts[i]++;
                        (*total_right)++;
                    }
                    break;
                }
            }
        }
    }
    rewind(file);
}

void find_best_infogain(FILE *file, int column_count, int total_rows, double *best_gain_for_each_column, double base_entropy, int class_count, char **list_classes, double *maxmaxinfo, int *maxmaxinfoINDEX, char **best_split_values)
{
    rewind(file);
    printf("\n[INFO] Calculating Information Gain for %d columns...\n", column_count);
    rewind(file);

    for (int i = 0; i < column_count; i++)
    {
        printf("\n>> Column %d: ", i);
        int is_numeric = check_if_column_contains_numbers(file, i);
        double max_gain = 0.0;

        if (is_numeric)
        {
            printf("numeric\n");
            double *thresholds = malloc((total_rows - 1) * sizeof(double));
            int threshold_count = 0;
            double best_threshold = 0.0;

            calculate_numeric_thresholds(file, i, thresholds, &threshold_count, total_rows);

            for (int j = 0; j < threshold_count; j += 50)
            {
                int *left_counts = calloc(class_count, sizeof(int));
                int *right_counts = calloc(class_count, sizeof(int));
                int total_left = 0, total_right = 0;

                split_by_numeric_threshold(file, i, thresholds[j], list_classes, class_count,
                                           left_counts, right_counts, &total_left, &total_right);

                double H_left = entropy(left_counts, class_count);
                double H_right = entropy(right_counts, class_count);

                double gain = base_entropy - ((total_left * H_left + total_right * H_right) / (double)total_rows);

                if (gain > max_gain)
                {
                    max_gain = gain;
                    best_threshold = thresholds[j];
                }

                free(left_counts);
                free(right_counts);
            }

            char buffer[32];
            sprintf(buffer, "%.6f", best_threshold);
            best_split_values[i] = strdup(buffer);

            free(thresholds);
        }
        else
        {
            printf("categorical\n");
            double gain = 0.0;
            char *best_category = NULL;
            calculate_categorical_thresholds(file, i, &gain, total_rows,
                                             list_classes, class_count,
                                             base_entropy, &best_category);
            max_gain = gain;
            best_split_values[i] = strdup(best_category);
            free(best_category);
        }

        if (max_gain > *maxmaxinfo)
        {
            *maxmaxinfo = max_gain;
            *maxmaxinfoINDEX = i;
        }

        best_gain_for_each_column[i] = max_gain;
        printf("[Done] Best Gain for Column %d: %.6f\n", i, max_gain);
    }
    rewind(file);
    printf("\n[INFO] Finished Information Gain calculation.\n");
}
