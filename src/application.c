#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "application.h"
#include "relation.h"

#define INPUT_FILE "input.txt"
#define INPUT_BUFFER_SIZE 4096
#define MAX_INPUT_ELEMENTS 50

typedef struct {
    bool reflexive;
    bool irreflexive;
    bool symmetric;
    bool antisymmetric;
    bool asymmetric;
    bool transitive;
    bool equivalence;
    bool partial_order;
} RelationAnalysis;

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

static Relation *load_relation(const char *filename) {
    FILE *input = fopen(filename, "r");
    Relation *relation = NULL;
    char line[INPUT_BUFFER_SIZE];
    const char *cursor;
    int declared_count;
    int elements[MAX_INPUT_ELEMENTS];
    size_t line_number = 1;
    size_t index;

    if (input == NULL) {
        fprintf(stderr, "Erro, nao foi possivel abrir o arquivo %s\n", filename);
        return NULL;
    }

    if (fgets(line, sizeof(line), input) == NULL || line_exceeds_buffer(line, input)) {
        fprintf(stderr, "Erro de entrada na linha 1: cabecalho ausente ou muito longo\n");
        goto error;
    }

    cursor = line;
    if (!parse_next_integer(&cursor, &declared_count) || declared_count < 0 ||
        declared_count > MAX_INPUT_ELEMENTS) {
        fprintf(stderr, "Erro de entrada na linha 1: numero de elementos invalido\n");
        goto error;
    }

    for (index = 0; index < (size_t)declared_count; index++) {
        if (!parse_next_integer(&cursor, &elements[index])) {
            fprintf(stderr, "Erro de entrada na linha 1: quantidade de elementos incorreta\n");
            goto error;
        }
        if (contains_element(elements, index, elements[index])) {
            fprintf(stderr, "Erro de entrada na linha 1: elementos duplicados\n");
            goto error;
        }
    }

    if (!contains_only_whitespace(cursor)) {
        fprintf(stderr, "Erro de entrada na linha 1: quantidade de elementos incorreta\n");
        goto error;
    }

    relation = relation_create(elements, (size_t)declared_count);
    if (relation == NULL) {
        fprintf(stderr, "Erro, nao foi possivel alocar a relacao\n");
        goto error;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        int source;
        int target;

        line_number++;
        if (line_exceeds_buffer(line, input)) {
            fprintf(stderr, "Erro de entrada na linha %zu: linha muito longa\n", line_number);
            goto error;
        }

        cursor = line;
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '\0') {
            continue;
        }

        if (!parse_next_integer(&cursor, &source) || !parse_next_integer(&cursor, &target) ||
            !contains_only_whitespace(cursor)) {
            fprintf(stderr, "Erro de entrada na linha %zu: par ordenado invalido\n", line_number);
            goto error;
        }

        if (!relation_add_pair(relation, source, target)) {
            fprintf(stderr, "Erro de entrada na linha %zu: elemento desconhecido\n", line_number);
            goto error;
        }
    }

    if (ferror(input)) {
        fprintf(stderr, "Erro durante a leitura de %s\n", filename);
        goto error;
    }

    fclose(input);
    return relation;

error:
    fclose(input);
    relation_destroy(relation);
    return NULL;
}

static int element_at(const Relation *relation, size_t index) {
    int element = 0;

    (void)relation_element_at(relation, index, &element);
    return element;
}

static Relation *create_transitive_closure(const Relation *relation) {
    Relation *closure = relation_clone(relation);
    size_t size = relation_size(relation);
    size_t intermediate;
    size_t source;
    size_t target;

    if (closure == NULL) {
        return NULL;
    }

    for (intermediate = 0; intermediate < size; intermediate++) {
        for (source = 0; source < size; source++) {
            if (!relation_contains(closure, source, intermediate)) {
                continue;
            }
            for (target = 0; target < size; target++) {
                if (relation_contains(closure, intermediate, target)) {
                    (void)relation_add_pair_at(closure, source, target);
                }
            }
        }
    }
    return closure;
}

static RelationAnalysis analyze_relation(const Relation *relation,
                                         const Relation *transitive_closure) {
    RelationAnalysis analysis = {true, true, true, true, true, true, false, false};
    size_t size = relation_size(relation);
    size_t source;
    size_t target;

    for (source = 0; source < size; source++) {
        if (relation_contains(relation, source, source)) {
            analysis.irreflexive = false;
            analysis.asymmetric = false;
        } else {
            analysis.reflexive = false;
        }

        for (target = source + 1; target < size; target++) {
            bool forward = relation_contains(relation, source, target);
            bool reverse = relation_contains(relation, target, source);

            if (forward != reverse) {
                analysis.symmetric = false;
            }
            if (forward && reverse) {
                analysis.antisymmetric = false;
                analysis.asymmetric = false;
            }
        }
    }

    for (source = 0; source < size; source++) {
        for (target = 0; target < size; target++) {
            if (relation_contains(transitive_closure, source, target) &&
                !relation_contains(relation, source, target)) {
                analysis.transitive = false;
            }
        }
    }

    analysis.equivalence = analysis.reflexive && analysis.symmetric && analysis.transitive;
    analysis.partial_order = analysis.reflexive && analysis.antisymmetric && analysis.transitive;
    return analysis;
}

static void print_matrix(const Relation *relation) {
    size_t size = relation_size(relation);
    size_t row;
    size_t column;

    printf("\n\t");
    for (column = 0; column < size; column++) {
        printf("%d ", element_at(relation, column));
    }

    printf("\n\n\n");
    for (row = 0; row < size; row++) {
        printf("%d\t", element_at(relation, row));
        for (column = 0; column < size; column++) {
            printf("%d ", relation_contains(relation, row, column) ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n\n");
}

static void print_reflexivity(const Relation *relation, bool reflexive) {
    size_t index;

    printf("1. Reflexiva: %c\n", reflexive ? 'V' : 'F');
    if (reflexive) {
        return;
    }

    for (index = 0; index < relation_size(relation); index++) {
        if (!relation_contains(relation, index, index)) {
            printf("(%d,%d); ", element_at(relation, index), element_at(relation, index));
        }
    }
    printf("\n");
}

static void print_irreflexivity(const Relation *relation, bool irreflexive) {
    size_t index;

    printf("2. Irreflexiva: %c\n", irreflexive ? 'V' : 'F');
    if (irreflexive) {
        return;
    }

    for (index = 0; index < relation_size(relation); index++) {
        if (relation_contains(relation, index, index)) {
            printf("(%d,%d); ", element_at(relation, index), element_at(relation, index));
        }
    }
    printf("\n");
}

static void print_symmetry(const Relation *relation, bool symmetric) {
    size_t source;
    size_t target;

    printf("3. Simetrica: %c\n", symmetric ? 'V' : 'F');
    if (symmetric) {
        return;
    }

    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target) &&
                !relation_contains(relation, target, source)) {
                printf("(%d,%d) e (%d,%d); ", element_at(relation, source),
                       element_at(relation, target), element_at(relation, target),
                       element_at(relation, source));
            }
        }
    }
    printf("\n");
}

static void print_antisymmetry(const Relation *relation, bool antisymmetric) {
    size_t source;
    size_t target;

    printf("4. Anti-simetrica: %c\n", antisymmetric ? 'V' : 'F');
    if (antisymmetric) {
        return;
    }

    for (source = 0; source < relation_size(relation); source++) {
        for (target = source + 1; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target) &&
                relation_contains(relation, target, source)) {
                printf("(%d,%d) e (%d,%d); ", element_at(relation, source),
                       element_at(relation, target), element_at(relation, target),
                       element_at(relation, source));
            }
        }
    }
    printf("\n");
}

static void print_transitivity(const Relation *relation, const Relation *closure,
                               bool transitive) {
    size_t source;
    size_t target;

    printf("6. Transitiva: %c\n", transitive ? 'V' : 'F');
    if (transitive) {
        return;
    }

    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(closure, source, target) &&
                !relation_contains(relation, source, target)) {
                printf("(%d,%d); ", element_at(relation, source),
                       element_at(relation, target));
            }
        }
    }
    printf("\n");
}

static void print_pair(int source, int target, bool *first) {
    if (!*first) {
        printf(",");
    }
    printf("(%d,%d)", source, target);
    *first = false;
}

static void print_existing_pairs(const Relation *relation, bool *first) {
    size_t source;
    size_t target;

    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target)) {
                print_pair(element_at(relation, source), element_at(relation, target), first);
            }
        }
    }
}

static void print_reflexive_closure(const Relation *relation) {
    bool first = true;
    size_t index;

    printf("Fecho reflexivo da relaçao = {");
    print_existing_pairs(relation, &first);
    for (index = 0; index < relation_size(relation); index++) {
        if (!relation_contains(relation, index, index)) {
            print_pair(element_at(relation, index), element_at(relation, index), &first);
        }
    }
    printf("}\n");
}

static void print_symmetric_closure(const Relation *relation) {
    bool first = true;
    size_t source;
    size_t target;

    printf("Fecho simetrico da relaçao = {");
    print_existing_pairs(relation, &first);
    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target) &&
                !relation_contains(relation, target, source)) {
                print_pair(element_at(relation, target), element_at(relation, source), &first);
            }
        }
    }
    printf("}\n");
}

static void print_transitive_closure(const Relation *relation, const Relation *closure) {
    bool first = true;
    size_t source;
    size_t target;

    printf("Fecho transitivo da relaçao = {");
    print_existing_pairs(relation, &first);
    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(closure, source, target) &&
                !relation_contains(relation, source, target)) {
                print_pair(element_at(relation, source), element_at(relation, target), &first);
            }
        }
    }
    printf("}\n");
}

static bool print_report(const Relation *relation) {
    Relation *transitive_closure = create_transitive_closure(relation);
    RelationAnalysis analysis;

    if (transitive_closure == NULL) {
        fprintf(stderr, "Erro, nao foi possivel calcular o fecho transitivo\n");
        return false;
    }

    analysis = analyze_relation(relation, transitive_closure);
    printf("\n\nPropriedades:\n\n");
    print_matrix(relation);
    print_reflexivity(relation, analysis.reflexive);
    print_irreflexivity(relation, analysis.irreflexive);
    print_symmetry(relation, analysis.symmetric);
    print_antisymmetry(relation, analysis.antisymmetric);
    printf("5. Asimetrica: %c\n", analysis.asymmetric ? 'V' : 'F');
    print_transitivity(relation, transitive_closure, analysis.transitive);
    printf("\nRelaçao de equivalencia: %c\n", analysis.equivalence ? 'V' : 'F');
    printf("Relaçao de ordem parcial: %c\n\n", analysis.partial_order ? 'V' : 'F');
    print_reflexive_closure(relation);
    print_symmetric_closure(relation);
    print_transitive_closure(relation, transitive_closure);
    printf("\n");

    relation_destroy(transitive_closure);
    return true;
}

int run_application(void) {
    Relation *relation = load_relation(INPUT_FILE);
    bool success;

    if (relation == NULL) {
        return EXIT_FAILURE;
    }

    success = print_report(relation);
    relation_destroy(relation);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
