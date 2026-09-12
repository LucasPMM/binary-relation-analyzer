#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "relation_io.h"

#define INPUT_BUFFER_SIZE 4096
#define MAX_INPUT_ELEMENTS 50

static bool parse_next_integer(const char **cursor, int *value) {
    char *end;
    long number;

    while (isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }
    if (**cursor == '\0') {
        return false;
    }

    errno = 0;
    number = strtol(*cursor, &end, 10);
    if (end == *cursor || errno == ERANGE || number < INT_MIN || number > INT_MAX) {
        return false;
    }

    *value = (int)number;
    *cursor = end;
    return true;
}

static bool contains_only_whitespace(const char *cursor) {
    while (isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return *cursor == '\0';
}

static bool line_exceeds_buffer(const char *line, FILE *input) {
    return strchr(line, '\n') == NULL && !feof(input);
}

static bool contains_element(const int *elements, size_t element_count, int element) {
    size_t index;

    for (index = 0; index < element_count; index++) {
        if (elements[index] == element) {
            return true;
        }
    }
    return false;
}

static bool read_header(FILE *input, int *elements, size_t *element_count) {
    char line[INPUT_BUFFER_SIZE];
    const char *cursor;
    int declared_count;
    size_t index;

    if (fgets(line, sizeof(line), input) == NULL || line_exceeds_buffer(line, input)) {
        fputs("Input error on line 1: missing or overlong header\n", stderr);
        return false;
    }

    cursor = line;
    if (!parse_next_integer(&cursor, &declared_count) || declared_count < 0 ||
        declared_count > MAX_INPUT_ELEMENTS) {
        fputs("Input error on line 1: invalid element count\n", stderr);
        return false;
    }

    for (index = 0; index < (size_t)declared_count; index++) {
        if (!parse_next_integer(&cursor, &elements[index])) {
            fputs("Input error on line 1: wrong number of elements\n", stderr);
            return false;
        }
        if (contains_element(elements, index, elements[index])) {
            fputs("Input error on line 1: duplicate elements\n", stderr);
            return false;
        }
    }

    if (!contains_only_whitespace(cursor)) {
        fputs("Input error on line 1: wrong number of elements\n", stderr);
        return false;
    }

    *element_count = (size_t)declared_count;
    return true;
}

static bool parse_pair_line(const char *line, int *source, int *target, bool *is_empty) {
    const char *cursor = line;

    while (isspace((unsigned char)*cursor)) {
        cursor++;
    }
    *is_empty = *cursor == '\0';
    if (*is_empty) {
        return true;
    }
    return parse_next_integer(&cursor, source) && parse_next_integer(&cursor, target) &&
           contains_only_whitespace(cursor);
}

static bool read_pairs(FILE *input, const char *filename, Relation *relation) {
    char line[INPUT_BUFFER_SIZE];
    size_t line_number = 1;

    while (fgets(line, sizeof(line), input) != NULL) {
        int source;
        int target;
        bool is_empty;

        line_number++;
        if (line_exceeds_buffer(line, input)) {
            fprintf(stderr, "Input error on line %zu: line is too long\n", line_number);
            return false;
        }

        if (!parse_pair_line(line, &source, &target, &is_empty)) {
            fprintf(stderr, "Input error on line %zu: invalid ordered pair\n", line_number);
            return false;
        }
        if (is_empty) {
            continue;
        }

        if (!relation_add_pair(relation, source, target)) {
            fprintf(stderr, "Input error on line %zu: unknown element\n", line_number);
            return false;
        }
    }

    if (ferror(input)) {
        fprintf(stderr, "I/O error while reading %s\n", filename);
        return false;
    }
    return true;
}

Relation *relation_load_from_file(const char *filename) {
    FILE *input;
    Relation *relation;
    int elements[MAX_INPUT_ELEMENTS];
    size_t element_count;

    if (filename == NULL) {
        fputs("Error: no input filename was provided\n", stderr);
        return NULL;
    }

    input = fopen(filename, "r");
    if (input == NULL) {
        fprintf(stderr, "Error: could not open input file %s\n", filename);
        return NULL;
    }

    if (!read_header(input, elements, &element_count)) {
        fclose(input);
        return NULL;
    }

    relation = relation_create(elements, element_count);
    if (relation == NULL) {
        fputs("Error: could not allocate the relation\n", stderr);
        fclose(input);
        return NULL;
    }

    if (!read_pairs(input, filename, relation)) {
        relation_destroy(relation);
        fclose(input);
        return NULL;
    }

    fclose(input);
    return relation;
}
