#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "dataset.h"

// מחזירה את האינדקס של הערך המקסימלי במערך
// הפונקצייה תעזור להבין איזה מחלקה אנחנו מנבעים בעץ שנוצר לנו
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
        char *value = row[current->feature_index];

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
    }

    // הגענו לעלה – מחזירים את שם המחלקה עם הכי הרבה מופעים
    int best_index = argmax(current->labels, current->num_classes);
    return class_names[best_index];
}

// כותב תחזיות לקובץ CSV
void write_predictions(Node *root, const char *input_csv, const char *output_csv, char **class_names)
{
    FILE *fin = fopen(input_csv, "r");
    FILE *fout = fopen(output_csv, "w");

    if (!fin || !fout)
    {
        printf("בעיה בפתיחת קבצים.\n");
        return;
    }

    char line[2048];
    int row_id = 0;

    // קריאת שורת כותרת
    fgets(line, sizeof(line), fin);
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
        {
            row[j] = tokens[j];
        }

        char *true_label = row[i - 1];
        char *predicted_label = predict(root, row, class_names);

        row_id++;
        fprintf(fout, "%d,%s,%s\n", row_id, true_label, predicted_label);

        free(row);
    }

    fclose(fin);
    fclose(fout);
    printf("[✔] נוצר קובץ התחזיות: %s\n", output_csv);
}
