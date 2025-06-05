#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "dataset.h"
#include "utils.h" // כולל את trim

// מחזירה את האינדקס של הערך המקסימלי במערך
int argmax(int *array, int length)
{
    int max_index = 0;
    for (int i = 1; i < length; i++)
    {
        if (array[i] > array[max_index])
            max_index = i;
    }
    return max_index;
}

// פונקציית ניבוי – מקבלת עץ, שורה אחת ומערך שמות המחלקות
char *predict(Node *root, char **row, char **class_names)
{
    Node *current = root;

    while (!current->is_leaf)
    {
        char *value = trim(strdup(row[current->feature_index])); // ניקוי ערכים

        if (current->is_numeric_split)
        {
            double val = atof(value);
            if (val <= current->threshold)
                current = current->left;
            else
                current = current->right;
        }
        else
        {
            if (strcmp(value, current->category_value) == 0)
                current = current->left;
            else
                current = current->right;
        }

        free(value); // שחרור הזיכרון של value
    }

    int best_index = argmax(current->labels, current->num_classes);
    return class_names[best_index]; // לא צריך trim כאן שוב
}

// כותב תחזיות לקובץ CSV כולל שורת דיוק
void write_predictions(Node *root, const char *input_csv, const char *output_csv, char **class_names)
{
    FILE *fin = fopen(input_csv, "r");
    FILE *fout = fopen(output_csv, "w+");

    if (!fin || !fout)
    {
        printf("Error opening files.\n");
        return;
    }

    char line[2048];
    int row_id = 0;
    int correct_predictions = 0;
    int total_predictions = 0;

    fgets(line, sizeof(line), fin); // דילוג על כותרת
    fprintf(fout, ",accuracy:           \n");
    fprintf(fout, "row_id,true_label,predicted_label\n");

    while (fgets(line, sizeof(line), fin))
    {
        line[strcspn(line, "\n")] = 0;

        char *tokens[100];
        int i = 0;
        char *token = strtok(line, ",");

        while (token && i < 100)
        {
            tokens[i++] = token;
            token = strtok(NULL, ",");
        }

        char **row = malloc(i * sizeof(char *));
        for (int j = 0; j < i; j++)
            row[j] = tokens[j];

        // ניקוי של true ו-pred באמצעות strdup
        char *true_label = trim(strdup(row[i - 1]));
        char *predicted_label = trim(strdup(predict(root, row, class_names)));

        if (strcmp(true_label, predicted_label) == 0)
            correct_predictions++;
        
       
        fprintf(fout, "%d,%s,%s\n", row_id + 1, true_label, predicted_label);
        row_id++;
        total_predictions++;

        free(row);
        free(true_label);
        free(predicted_label);
    }

    fclose(fin);

    double accuracy = 100.0 * correct_predictions / total_predictions;
    fseek(fout, 0, SEEK_SET);
    fprintf(fout, ",accuracy: %.2f%%\n", accuracy);

    fclose(fout);
    printf("Predictions written to: %s with accuracy: %.2f%%\n", output_csv, accuracy);
    system("start \"\" \"predictions.csv\"");
}
