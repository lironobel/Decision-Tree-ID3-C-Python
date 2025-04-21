// פונקציות כלליות כמו trim, entropy, מיון

// === utils.c ===
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"

// פונקציה לנקות רווחים מיותרים בתחילת ובסוף המילה
void trim_inplace(char *str)
{
    char *start = str;
    while (*start == ' ')
        start++;

    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r'))
        end--;

    *(end + 1) = '\0';
    memmove(str, start, strlen(start) + 1); // הזזה קדימה לתוך str המקורי
}

// פונקצייה שמחשבת את האנטרופיה של מחלקה מסוימת
//  מחזירה את ערך האנטרופיה של המחלקה
double entropy(int *classes, int num_classes)
{
    int total = 0;
    for (int i = 0; i < num_classes; i++)
        total += classes[i];

    double entropy_value = 0.0;
    for (int i = 0; i < num_classes; i++)
    {
        if (classes[i] > 0)
        {
            double p = (double)classes[i] / total;
            entropy_value -= p * log2(p);
        }
    }
    return entropy_value;
}

// פונקצייה שמשווה בין שני מספרים (for the qsort function)
int compare_doubles(const void *a, const void *b)
{
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

// פונקציה שמחזירה 1 אם העמודה מכילה  רק מספרים, אחרת מחזירה 0
int check_if_column_contains_numbers(FILE *file, int column_index)
{
    char line[1024];
    rewind(file);
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file))
    {
        char *token = strtok(line, ",");
        int col = 0;
        while (token)
        {
            if (col == column_index)
            {
                char *endptr;
                strtod(token, &endptr);
                if (*endptr != '\0' && *endptr != '\n')
                    return 0;
            }
            token = strtok(NULL, ",");
            col++;
        }
    }
    rewind(file); // העברת המצביע של הקובץ להתחלה
    return 1;
}

// פונקציה שמשווה אם מחרוזת קיימת במערך של מחרוזות
bool is_value_in_array(char **array, int size, const char *value)
{
    for (int i = 0; i < size; i++)
    {
        if (strcmp(array[i], value) == 0)
            return true;
    }
    return false;
}

// פונקציה לספירת השורות בקובץ
int count_rows(FILE *file)
{
    if (!file)
        return 0;

    rewind(file); // חזור להתחלה
    char line[2048];
    int count = 0;

    // דלג על שורת הכותרת
    if (!fgets(line, sizeof(line), file))
        return 0;

    while (fgets(line, sizeof(line), file))
    {
        count++;
    }
    rewind(file); // נחזור להתחלה למקרה שהקובץ ייקרא שוב
    return count;
}
