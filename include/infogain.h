
// === infogain.h ===
#ifndef INFOGAIN_H
#define INFOGAIN_H

#include <stdio.h>

// פונקציה שמקבלת קובץ, מספר עמודה, מערך של ספים, מספר שורות, רשימה של מחלקות, מספר מחלקות
// ומחשבת את הספים עבור העמודה ומכניסה אותם למערך
// עבור עמודה מילולית
void calculate_categorical_thresholds(FILE *file, int column_index, double *best_gain, int total_rows, char **list_classes, int class_count, double base_entropy, char **best_category);

// פונקציה שמקבלת קובץ, מספר עמודה, מערך של ספים, מספר שורות, רשימה של מחלקות, מספר מחלקות
// ומחשבת את הספים עבור העמודה ומכניסה אותם למערך
// עבור עמודה מספרית
void calculate_numeric_thresholds(FILE *file, int column_index, double *thresholds, int *threshold_count, int total_rows);

// פונקציה שמקבלת קובץ, מספר עמודה, מערך של ספים ומספר שורות ומחשבת את הספים עבור העמודה
// ומכניסה אותם למערך
void split_by_numeric_threshold(FILE *file, int column_index, double threshold, char **list_classes, int class_count, int *left_counts, int *right_counts, int *total_left, int *total_right);

// פונקציה שמקבלת קובץ, מספר עמודות, מספר שורות, מערך של רווחים, אנטרופיה בסיסית, מספר מחלקות ורשימה של מחלקות
// ומחשבת את הרווח המידע עבור כל עמודה ומכניסה אותו למערך
void find_best_infogain(FILE *file, int column_index, int total_rows, double *best_gain_for_each_column, double base_entropy, int class_count, char **list_classes, double *max_gain, int *max_gain_index, char **best_split_values);

#endif
