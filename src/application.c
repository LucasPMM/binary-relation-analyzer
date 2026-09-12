#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "application.h"
#include "relation.h"
#include "relation_io.h"
#include "relation_report.h"

#define PROGRAM_NAME "binary-relation-analyzer"

static void write_usage(FILE *output) {
    fprintf(output, "Usage: %s [--] INPUT_FILE\n", PROGRAM_NAME);
    fprintf(output, "       %s --help\n", PROGRAM_NAME);
    fputs("Analyze the properties and closures of a finite binary relation.\n", output);
}

static const char *parse_input_filename(int argc, char *argv[]) {
    if (argc == 2 && argv[1][0] != '-') {
        return argv[1];
    }
    if (argc == 3 && strcmp(argv[1], "--") == 0) {
        /* "--" permits filenames that begin with a dash. */
        return argv[2];
    }
    return NULL;
}

int run_application(int argc, char *argv[]) {
    const char *input_filename;
    Relation *relation;
    bool success;

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        write_usage(stdout);
        return EXIT_SUCCESS;
    }

    input_filename = parse_input_filename(argc, argv);
    if (input_filename == NULL) {
        if (argc == 2 && strcmp(argv[1], "--") == 0) {
            fputs("Error: expected an input file after --\n", stderr);
        } else if (argc == 2 && argv[1][0] == '-') {
            fprintf(stderr, "Error: unknown option: %s\n", argv[1]);
        } else {
            fputs("Error: expected exactly one input file\n", stderr);
        }
        write_usage(stderr);
        return EXIT_FAILURE;
    }

    relation = relation_load_from_file(input_filename);
    if (relation == NULL) {
        return EXIT_FAILURE;
    }

    success = relation_write_report(relation, stdout);
    relation_destroy(relation);

    if (!success) {
        fputs("Error: could not produce the relation report\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
