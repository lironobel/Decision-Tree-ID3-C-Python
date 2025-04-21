
// === utils.h ===
#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdbool.h>

// פונקצייה שמקבלת מחרוזת ומסירה ממנה רווחים מיותרים
char *trim(char *str);

// פונקצייה שמקבלת מערך של מספרים ומחזירה את האנטרופיה של המערך
double entropy(int *classes, int num_classes);

// פונקצייה שמקבלת שני מספרים ומחזירה את ההשוואה ביניהם
int compare_doubles(const void *a, const void *b);

// פונקצייה שמקבלת קובץ ומספר עמודה ומחזירה 1 אם העמודה מכילה מספרים ו-0 אם לא
int check_if_column_contains_numbers(FILE *file, int column_index);

// פונקציה שמשווה אם מחרוזת קיימת במערך של מחרוזות
bool is_value_in_array(char **array, int size, const char *value);

// פונקציה שמקבלת קובץ ומחזירה את מספר השורות בקובץ
int count_rows(FILE *file);

#endif
