// הקריאה לפונקציות, שליטה על הזרימה
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dataset.h"
#include "infogain.h"
#include "utils.h"
#include "tree.h"
#include "buildTree.h"
#include "tree_graph.h"
#include "pretictedvalues.h"

static void maybe_pause(int should_pause)
{
    if (should_pause)
    {
        system("pause");
    }
}

static int is_positive_integer(const char *value)
{
    if (value == NULL || *value == '\0')
    {
        return 0;
    }

    for (const char *p = value; *p != '\0'; p++)
    {
        if (!isdigit((unsigned char)*p))
        {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[])
{
    int should_pause = 1;
    int max_depth_limit = 4;
    int enable_visualization = 1;
    int max_depth_set = 0;

    if (argc < 2)
    {
        printf("ERROR: Choose a valid folder/file path.\n");
        printf("Usage: %s [path_to_csv] [--no-pause] [--no-visuals] [max_depth]\n", argv[0]);
        maybe_pause(should_pause);
        return 1;
    }

    for (int arg_index = 2; arg_index < argc; arg_index++)
    {
        const char *arg = argv[arg_index];
        if (strcmp(arg, "--no-pause") == 0)
        {
            should_pause = 0;
        }
        else if (strcmp(arg, "--no-visuals") == 0)
        {
            enable_visualization = 0;
        }
        else if (is_positive_integer(arg) && !max_depth_set)
        {
            max_depth_limit = atoi(arg);
            max_depth_set = 1;
            if (max_depth_limit < 1)
            {
                printf("ERROR: max_depth must be >= 1\n");
                maybe_pause(should_pause);
                return 1;
            }
        }
        else
        {
            printf("ERROR: Invalid argument: %s\n", arg);
            printf("Usage: %s [path_to_csv] [--no-pause] [--no-visuals] [max_depth]\n", argv[0]);
            maybe_pause(should_pause);
            return 1;
        }
    }

    const char *Path = argv[1];
    printf("Using dataset: %s\n", Path);

    // פתיחת הקובץ
    FILE *file = fopen(Path, "r");
    if (!file)
    {
        printf("ERROR: Choose a valid folder/file path.\n");
        maybe_pause(should_pause);
        return 1;
    }

    MakeSure(Path);

    // קריאת המחלקות מהקובץ
    int class_count = 0;
    char **classes = count_classes(file, &class_count);
    if (classes == NULL || class_count == 0)
    {
        printf("No classes found in the file.\n");
        fclose(file);
        maybe_pause(should_pause);
        return 0;
    }

    // הדפסת המחלקות שנמצאו
    printf("Classes found (%d):\n", class_count);
    for (int i = 0; i < class_count; i++)
    {
        printf("- %s\n", classes[i]);
    }

    // בניית עץ ההחלטה
    printf("Building decision tree...\n");
    Node *root = NULL;
    int max_depth_reached = build_tree(&root, file, 1, "Root", max_depth_limit, classes, class_count);
    printf("MAX_DEPTH_REACHED: %d\n", max_depth_reached);

    // יצירת קובץ התחזיות
    printf("Writing predictions to CSV...\n");
    write_predictions(root, Path, "predictions.csv", classes);
    // הערה: הסרתי את הפתיחה האוטומטית של ה-CSV כדי שלא יקפצו לך 20 חלונות בכל הרצה
    // אם אתה רוצה את זה, פשוט תחזיר את ה-system("start predictions.csv");

    int allocated_columns = 0;
    char **feature_names_vector = NULL;

    if (enable_visualization)
    {
        // ספירת עמודות ויצירת שמות תכונות
        int column_count = count_columnsfunc(file);
        feature_names_vector = create_and_print_feature_vector(file, column_count, &allocated_columns);

        // --- ייצוא ויזואלי של העץ ---
        const char *dot_filename = "tree.dot";
        const char *png_filename = "tree.png";

        FILE *dot_file = fopen(dot_filename, "w");
        if (dot_file)
        {
            printf("Exporting colored tree to DOT...\n");
            export_tree_to_dot(root, dot_file, feature_names_vector, classes, class_count);
            fclose(dot_file);

            // שימוש בפונקציה החדשה שיצרנו ב-tree_graph.c
            // היא מטפלת גם בהרצה של Graphviz וגם בפתיחת התמונה
            generate_and_open_graph(dot_filename, png_filename);
        }
        else
        {
            printf("[ERROR] Could not create tree.dot\n");
        }
    }
    else
    {
        printf("Visualization skipped (--no-visuals).\n");
    }

    // שחרור זיכרון
    printf("Cleaning up memory...\n");
    free_tree(root);
    fclose(file);

    if (feature_names_vector != NULL)
    {
        for (int i = 0; i < allocated_columns; i++)
        {
            free(feature_names_vector[i]);
        }
        free(feature_names_vector);
    }
    for (int i = 0; i < class_count; i++)
    {
        free(classes[i]);
    }
    free(classes);

    if (enable_visualization)
        printf("\nDone! Look at the generated tree image.\n");
    else
        printf("\nDone! Core algorithm run completed.\n");
    maybe_pause(should_pause);
    return 0;
}