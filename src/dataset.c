// קריאת CSV, איתור מחלקות, יצירת feature vector
// === dataset.c ===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dataset.h"
#include "utils.h"

#define MAX_LINE_LENGTH 1024
#define MAX_UNIQUE_VALUES 1000
#define MAX_COLUMN_COUNT 32

// פונקציה לפתיחת קובץ
void open_file_for_reading(FILE **file, const char *filename)
{
    *file = fopen(filename, "r"); // פותח את הקובץ לקריאה
    if (!*file)
    {
        printf("Error opening file!!!!!!!!!!!!!!!!\n");
        exit(1);
    }
}

// הפונקציה הראשית שמדפיסה ערכים ייחודיים בעמודה מסוימת
// רק אם העמודה מכילה ערכים שאינם מספרים
void print_unique_values_in_column(FILE *file, int column_index)
{
    if (check_if_column_contains_numbers(file, column_index))
    {
        printf("The column contains only numbers.\n");
        return;
    }
    char line[MAX_LINE_LENGTH];
    char *token;
    char **unique_values = malloc(MAX_UNIQUE_VALUES * sizeof(char *));
    int unique_count = 0;

    if (!unique_values)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    // מדלגים על שורת הכותרת
    if (!fgets(line, sizeof(line), file))
    {
        printf("Failed to read header.\n");
        free(unique_values);
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        int current_column = 0;
        token = strtok(line, ",");

        while (token && current_column < column_index)
        {
            token = strtok(NULL, ",");
            current_column++;
        }

        if (token && current_column == column_index)
        {
            // מסירים תווי רווח או newline בסוף
            token[strcspn(token, "\n\r")] = '\0';

            if (!is_value_in_array(unique_values, unique_count, token))
            {
                if (unique_count >= MAX_UNIQUE_VALUES)
                {
                    printf("Too many unique values (limit: %d)\n", MAX_UNIQUE_VALUES);
                    break;
                }
                unique_values[unique_count] = malloc(strlen(token) + 1);
                strcpy(unique_values[unique_count], token);
                unique_count++;
            }
        }
    }

    printf("\nUnique values in column %d:\n", column_index + 1);
    for (int i = 0; i < unique_count; i++)
    {
        printf("- %s\n", unique_values[i]);
        free(unique_values[i]);
    }

    free(unique_values);
    rewind(file); // מחזיר את הקובץ להתחלה
}

// פונקציה שמקבלת קובץ ומחזירה מערך של מחרוזות שמייצגות את השמות של העמודות
// ומדפיסה את השמות של העמודות
char **create_and_print_feature_vector(FILE *file, int *column_count)
{

    char line[MAX_LINE_LENGTH];
    if (!fgets(line, sizeof(line), file)) // מוודא שהקובץ לא ריק או פגום לפני שמנסים לעבד אותו
    {
        printf("Error reading file.\n");
        return NULL;
    }

    *column_count = 1;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == ',')
            (*column_count)++;
    }

    char **feature_vector = (char **)malloc(*column_count * sizeof(char *));
    if (!feature_vector)
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    char *token = strtok(line, ",");
    int i = 0;
    while (token)
    {
        feature_vector[i] = (char *)malloc(strlen(token) + 1);
        if (!feature_vector[i])
        {
            printf("Memory allocation failed for column name!\n");
            for (int j = 0; j < i; j++)
                free(feature_vector[j]);
            free(feature_vector);
            return NULL;
        }
        strcpy(feature_vector[i], token);
        i++;
        token = strtok(NULL, ",");
    }
    if (feature_vector == NULL)
    {
        printf("Failed to create feature vector.\n");
        return 0;
    }
    else
    {
        printf("\nFeature Vector:\n");
        for (int i = 0; i < *column_count; i++)
            printf("%d: %s\n", i + 1, feature_vector[i]);
    }
    rewind(file); // העברת המצביע של הקובץ להתחלה
    return feature_vector;
}

// פונקציה לספירת סוגי המחלקות הייחודיות
char **count_classes(FILE *file, int *class_count, int *total_rows)
{
    char line[1024];
    char **unique_classes = NULL; // מערך לשמירת מחלקות ייחודיות
    int num_classes = 0;
    int class_column = -1;

    // קריאת כותרות הקובץ כדי לקבוע את מספר העמודות
    if (fgets(line, sizeof(line), file))
    {
        // ספירת מספר העמודות לפי מספר הפסיקים
        int col_count = 0;
        for (char *p = line; *p; p++)
        {
            if (*p == ',')
                col_count++;
        }
        class_column = col_count; // העמודה האחרונה היא מספר העמודות הכולל
    }

    // קריאת הנתונים
    while (fgets(line, sizeof(line), file))
    {
        (*total_rows)++;
        char *token = strtok(line, ",");
        int col = 0;
        char *class_value = NULL;

        while (token)
        {
            if (col == class_column)
            {
                class_value = token; // קרא את הערך של המחלקה
                break;
            }
            token = strtok(NULL, ",");
            col++;
        }

        if (class_value != NULL)
        {
            trim_inplace(class_value);

            // בדוק אם המחלקה כבר קיימת ברשימה
            int found = 0;
            for (int i = 0; i < num_classes; i++)
            {
                if (strcmp(unique_classes[i], class_value) == 0)
                {
                    found = 1;
                    break;
                }
            }

            // אם לא נמצא, הוסף את המחלקה לרשימה
            if (!found)
            {
                unique_classes = realloc(unique_classes, (num_classes + 1) * sizeof(char *));
                if (!unique_classes)
                {
                    printf("Memory allocation failed.\n");
                    return NULL;
                }
                unique_classes[num_classes] = strdup(class_value);
                num_classes++;
            }
        }
    }

    // עדכון class_count עם מספר סוגי המחלקות
    *class_count = num_classes;
    if (unique_classes == NULL)
    {
        printf("Failed to read classes.\n");
        return 0;
    }
    rewind(file);
    return unique_classes;
}

// פונקציה לספירת התצפיות עבור כל מחלקה
// מחזירה מערך של מספר התצפיות עבור כל מחלקה
void count_rows_for_each_class(FILE *file, int *class_counts, char **list_classes, int class_count)
{
    char line[1024];
    int class_column = -1;

    // קריאת הכותרת כדי לזהות את מספר העמודות
    if (fgets(line, sizeof(line), file))
    {
        int col_count = 0;
        for (char *p = line; *p; p++)
        {
            if (*p == ',')
                col_count++;
        }
        class_column = col_count;
    }

    while (fgets(line, sizeof(line), file))
    {
        char *token = strtok(line, ",");
        int col = 0;
        char *class_value = NULL;

        while (token)
        {
            if (col == class_column)
            {
                class_value = token;
                break;
            }
            token = strtok(NULL, ",");
            col++;
        }

        if (class_value != NULL)
        {
            trim_inplace(class_value);

            for (int j = 0; j < class_count; j++)
            {
                if (strcmp(class_value, list_classes[j]) == 0)
                {
                    class_counts[j]++;
                    break;
                }
            }
        }
    }
    rewind(file);
}

// פונקציה שמקבלת קובץ ומחזירה קובץ מסונן לפי תצפיות
FILE *create_temp_csv_filtered(FILE *file, int column_index, const char *match_value, int keep_if_match, const char *temp_filename, int is_numeric)
{
    rewind(file);
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file)
    {
        printf("Failed to create temp file.\n");
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    // העתקת שורת הכותרת
    if (fgets(line, sizeof(line), file))
    {
        fprintf(temp_file, "%s", line);
    }

    while (fgets(line, sizeof(line), file))
    {
        char line_copy[MAX_LINE_LENGTH];
        strcpy(line_copy, line);

        char *token;
        char *tokens[MAX_COLUMN_COUNT];
        int token_index = 0;

        token = strtok(line_copy, ",");
        while (token != NULL && token_index < MAX_COLUMN_COUNT)
        {
            tokens[token_index++] = token;
            token = strtok(NULL, ",");
        }

        if (token_index <= column_index)
            continue;

        int is_match = 0;
        if (is_numeric)
        {
            double val = atof(tokens[column_index]);
            double threshold = atof(match_value);
            is_match = val < threshold;
        }
        else
        {
            is_match = strcmp(tokens[column_index], match_value) == 0;
        }

        if ((keep_if_match && is_match) || (!keep_if_match && !is_match))
        {
            fprintf(temp_file, "%s", line);
        }
    }

    rewind(temp_file);
    return temp_file;
}
