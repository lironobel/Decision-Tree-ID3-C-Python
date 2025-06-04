#ifndef PREDICTEDVALUES_H
#define PREDICTEDVALUES_H

#include <stdio.h>
#include "tree.h"

// פונקציה שכותבת תחזיות של העץ לקובץ CSV
void write_predictions(Node *root, const char *input_csv, const char *output_csv, char **class_names);

#endif
