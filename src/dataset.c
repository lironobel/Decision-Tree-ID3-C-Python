// קריאת CSV, איתור מחלקות, יצירת feature vector
// === dataset.c ===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dataset.h"
#include "utils.h"
#include "buildTree.h"

#define MAX_LINE_LENGTH 1024
#define MAX_UNIQUE_VALUES 1000
#define MAX_COLUMN_COUNT 100
#define MAX_TEMP_FILENAME_LENGTH 100

// הפונקציה הראשית שמדפיסה ערכים ייחודיים בעמודה מסוימת
// רק אם העמודה מכילה ערכים שאינם מספרים
void print_unique_values_in_column(FILE *file, int column_index)
{
    rewind(file);
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
char **create_and_print_feature_vector(FILE *file, int column_count, int *allocated_columns)
{
    rewind(file);

    char line[MAX_LINE_LENGTH];
    if (!fgets(line, sizeof(line), file)) // מוודא שהקובץ לא ריק או פגום לפני שמנסים לעבד אותו
    {
        printf("Error reading file.\n");
        return NULL;
    }

    char **feature_vector = (char **)malloc(column_count * sizeof(char *));
    if (!feature_vector)
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    char *token = strtok(line, ",");
    int i = 0;
    while (token)
    {
        (*allocated_columns)++;
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
        for (int i = 0; i < column_count; i++)
            printf("%d: %s\n", i + 1, feature_vector[i]);
    }
    rewind(file); // העברת המצביע של הקובץ להתחלה
    return feature_vector;
}

// פונקצייה לספירת מספר העמודות בקובץ
int count_columnsfunc(FILE *file)
{
    rewind(file);
    int column_count = 0;
    char line[MAX_LINE_LENGTH];
    if (!fgets(line, sizeof(line), file)) // מוודא שהקובץ לא ריק או פגום לפני שמנסים לעבד אותו
    {
        printf("Error reading file.\n");
        return 0;
    }

    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == ',')
            column_count++;
    }
    rewind(file);
    return column_count;
}

// פונקציה לספירת סוגי המחלקות הייחודיות
char **count_classes(FILE *file, int *class_count)
{
    char line[1024];
    char **unique_classes = NULL; // מערך לשמירת מחלקות ייחודיות
    int num_classes = 0;
    int class_column = -1;
    rewind(file);
    if (!file)
    {
        printf("Invalid file pointer.\n");
        *class_count = 0;
        return NULL;
    }

    // קריאת כותרות הקובץ כדי לקבוע את מספר העמודות
    if (!fgets(line, sizeof(line), file))
    {
        printf("Error reading file header.\n");
        *class_count = 0;
        return NULL;
    }

    // ספירת מספר העמודות לפי מספר הפסיקים
    int col_count = 0;
    for (char *p = line; *p; p++)
    {
        if (*p == ',')
            col_count++;
    }
    class_column = col_count; // העמודה האחרונה היא מספר העמודות הכולל

    // קריאת הנתונים
    while (fgets(line, sizeof(line), file))
    {
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
            trim(class_value);
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
                char *copy = strdup(class_value);
                if (!copy)
                {
                    printf("Memory allocation failed.\n");
                    // ניקוי מה שכבר הוקצה
                    for (int j = 0; j < num_classes; j++)
                        free(unique_classes[j]);
                    free(unique_classes);
                    *class_count = 0;
                    return NULL;
                }

                char **temp = realloc(unique_classes, (num_classes + 1) * sizeof(char *));
                if (!temp)
                {
                    printf("Memory allocation failed.\n");
                    free(copy);
                    for (int j = 0; j < num_classes; j++)
                        free(unique_classes[j]);
                    free(unique_classes);
                    *class_count = 0;
                    return NULL;
                }

                unique_classes = temp;
                unique_classes[num_classes] = copy;
                num_classes++;
            }
        }
    }

    // עדכון class_count עם מספר סוגי המחלקות
    *class_count = num_classes;

    if (num_classes == 0)
    {
        printf("No classes found in the file.\n");
        free(unique_classes);
        return NULL;
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
    rewind(file);
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
            trim(class_value);

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

// פונקצייה שמקבלת קובץ
//  ומחזירה מצביע לקובץ החדש שמכיל את השורות המסוננות
FILE *create_temp_csv_filtered(FILE *file, int column_index, const char *match_value, int keep_if_match, const char *temp_filename, int is_numeric)
{
    rewind(file);
    int written_rows = 0;
    FILE *temp_file = fopen(temp_filename, "w");
    if (!temp_file)
    {
        printf("[ERROR] Failed to create temp file: %s\n", temp_filename);
        return NULL;
    }

    char line[MAX_LINE_LENGTH];

    if (fgets(line, sizeof(line), file))
    {
        if (strlen(line) < 2)
        {
            printf("[ERROR] Header line is too short or empty in source file.\n");
            fclose(temp_file);
            return NULL;
        }
        fputs(line, temp_file);
    }
    else
    {
        printf("[ERROR] Failed to read header line from source file.\n");
        fclose(temp_file);
        return NULL;
    }

    while (fgets(line, sizeof(line), file))
    {
        char original_line[MAX_LINE_LENGTH];
        strcpy(original_line, line);

        char line_copy[MAX_LINE_LENGTH];
        strcpy(line_copy, line);

        char *tokens[MAX_COLUMN_COUNT];
        int col = 0;
        char *token = strtok(line_copy, ",");

        while (token && col < MAX_COLUMN_COUNT)
        {
            tokens[col++] = token;
            token = strtok(NULL, ",");
        }

        if (col <= column_index)
            continue;

        char *value = tokens[column_index];
        value[strcspn(value, "\n\r")] = '\0';
        trim(value);

        int match = 0;
        if (is_numeric)
        {
            double val = atof(value);
            double threshold = atof(trim((char *)match_value));
            match = val <= threshold;
        }
        else
        {
            char match_copy[256];
            strncpy(match_copy, match_value, sizeof(match_copy));
            match_copy[sizeof(match_copy) - 1] = '\0';
            trim(match_copy);
            match = strcmp(value, match_copy) == 0;
        }

        int result = keep_if_match ? match : !match;

        if (result)
        {
            fputs(original_line, temp_file);
            written_rows++;
        }
    }

    fflush(temp_file);
    fclose(temp_file);

    if (written_rows == 0)
    {
        printf("[WARNING] No rows matched for filtering into %s\n", temp_filename);
        return NULL;
    }

    FILE *read_file = fopen(temp_filename, "r");
    if (!read_file)
    {
        printf("[ERROR] Failed to reopen temp file: %s\n", temp_filename);
        return NULL;
    }

    char header_check[MAX_LINE_LENGTH];
    if (!fgets(header_check, sizeof(header_check), read_file))
    {
        printf("[ERROR] File %s opened but header line is missing or unreadable.\n", temp_filename);
        fclose(read_file);
        return NULL;
    }

    rewind(read_file);
    return read_file;
}

// פונקצייה שתרוץ בהתחלה של התהליך ותבדוק אם משהו פגום או יכול להשתבש
void MakeSure(const char *filename)
{
    int system_result = 0; // משתנה לשמירת תוצאת המערכת
    printf("[INFO] Starting MakeSure() checks for file: %s\n", filename);
    FILE *file = fopen(filename, "r");
    // בדיקה אם הקובץ נפתח בהצלחה
    if (!file)
    {
        printf("[ERROR] Failed to open file: %s\n", filename);
        system("pause");
        exit(1);
    }
    printf("[INFO] File opened successfully: %s\n", filename);

    // בדיקת מספר עמודות
    int column_count = count_columnsfunc(file);
    if (column_count <= 0 )
    {
        printf("[ERROR] Invalid column count: %d\n", column_count);
        fclose(file);
        system("pause");
    }
    printf("[INFO] Column count: %d\n", column_count);

    // בדיקת שמות עמודות
    int allocated_columns = 0;
    char **feature_vector = create_and_print_feature_vector(file, column_count, &allocated_columns);
    if (!feature_vector)
    {
        printf("[ERROR] Failed to create feature vector.\n");
        fclose(file);
        system("pause");
    }
    for (int i = 0; i < allocated_columns; i++)
    {
        if (feature_vector[i] == NULL || strlen(feature_vector[i]) == 0)
        {
            printf("[ERROR] Empty or NULL column name at index %d.\n", i);
            fclose(file);
                   system("pause");

        }
    }
    printf("[INFO] All column names are valid.\n");

    // בדיקת מחלקות
    int class_count = 0;
    char **class_names = count_classes(file, &class_count);
    if (!class_names || class_count == 0)
    {
        printf("[ERROR] No classes found in the file.\n");
        fclose(file);
                system("pause");

    }
    printf("[INFO] Classes found (%d):\n", class_count);
    for (int i = 0; i < class_count; i++)
    {
        printf("- %s\n", class_names[i]);
        free(class_names[i]);
    }
    free(class_names);

    // בדיקה כללית של ערכים ריקים ושגויים בעמודות
    char line[MAX_LINE_LENGTH];
    int row_number = 1;
    rewind(file);
    fgets(line, sizeof(line), file); // דילוג על כותרת

    bool *is_numeric_column = (bool *)calloc(column_count, sizeof(bool));
    for (int i = 0; i < column_count; i++)
    {
        if (i == column_count - 1) // התעלמות מהעמודה האחרונה (המחלקה)
            is_numeric_column[i] = false;
        else
            is_numeric_column[i] = check_if_column_contains_numbers(file, i);
    }

    rewind(file);
    fgets(line, sizeof(line), file); // דילוג על כותרת שוב

    while (fgets(line, sizeof(line), file))
    {
        row_number++;
        int col = 0;
        char *token = strtok(line, ",");
        while (token)
        {
            trim(token);
            if (strlen(token) == 0)
            {
                printf("[WARNING] Empty value at row %d, column %d.\n", row_number, col + 1);
                system_result = 1;
            }
            else if (col < column_count - 1 && is_numeric_column[col])
            {
                // בדוק מספרים רק בעמודות שאינן עמודת המחלקה
                char *endptr;
                strtod(token, &endptr);
                if (endptr == token || *endptr != '\0')
                {
                    printf("[WARNING] Invalid numeric value at row %d, column %d: %s\n", row_number, col + 1, token);
                    system("pause");
                }
            }
            token = strtok(NULL, ",");
            col++;
        }

        if (col < column_count)
        {
            printf("[WARNING] Row %d has fewer columns than expected. Expected: %d, Found: %d\n", row_number, column_count, col);
        }
    }

    fclose(file);
    free(is_numeric_column);

    for (int i = 0; i < allocated_columns; i++)
        free(feature_vector[i]);
    if(system_result != 0)
    {
        printf("[ERROR] MakeSure() checks failed. Please fix the issues and try again.\n");
        system("pause");
        exit(1);
    }
    printf("[INFO] MakeSure() checks passed. Everything looks good!\n");
}
