// === dataset.h ===
#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>
#include <stdbool.h>

// הפונקציה הראשית שמדפיסה ערכים ייחודיים בעמודה מסוימת
void print_unique_values_in_column(FILE *file, int column_index);

// פונקציה לפתיחת קובץ
void open_file_for_reading(FILE **file, const char *filename);

// פונקציה שמקבלת קובץ ומחזירה מערך של מחרוזות שמייצגות את השמות של העמודות
char **create_and_print_feature_vector(FILE *file, int *column_count);

// פונקציה שמקבלת קובץ ומחזירה מערך של מחרוזות שמייצגות את השמות של המחלקות
char **count_classes(FILE *file, int *class_count, int *total_rows);

// פונקציה שמקבלת קובץ ומחזירה מערך של מספרים שמייצגים את מספר השורות עבור כל מחלקה
void count_rows_for_each_class(FILE *file, int *class_counts, char **list_classes, int class_count);

// פונקצייה שיוצרת קובץ מסונן לפי תצפיות
FILE *create_temp_csv_filtered(FILE *file, int column_index, const char *match_value, int keep_if_match, const char *temp_filename, int is_numeric);


#endif