#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "application.h"
#include "relation.h"
#include "relation_io.h"
#include "relation_report.h"

#define INPUT_FILE "input.txt"

int run_application(void) {
    Relation *relation = relation_load_from_file(INPUT_FILE);
    bool success;

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
