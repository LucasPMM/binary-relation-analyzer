#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "relation_analysis.h"
#include "relation_closure.h"
#include "relation_report.h"

static const char *boolean_text(bool value) {
    return value ? "true" : "false";
}

static int element_at(const Relation *relation, size_t index) {
    int element = 0;

    (void)relation_element_at(relation, index, &element);
    return element;
}

static void write_matrix(const Relation *relation, FILE *output) {
    size_t size = relation_size(relation);
    size_t row;
    size_t column;

    fputs("Relation matrix:\n\n\t", output);
    for (column = 0; column < size; column++) {
        fprintf(output, "%d ", element_at(relation, column));
    }

    fputs("\n\n", output);
    for (row = 0; row < size; row++) {
        fprintf(output, "%d\t", element_at(relation, row));
        for (column = 0; column < size; column++) {
            fprintf(output, "%d ", relation_contains(relation, row, column) ? 1 : 0);
        }
        fputc('\n', output);
    }
    fputc('\n', output);
}

static void write_reflexivity_details(const Relation *relation, FILE *output) {
    size_t index;

    fputs("Missing reflexive pairs: ", output);
    for (index = 0; index < relation_size(relation); index++) {
        if (!relation_contains(relation, index, index)) {
            fprintf(output, "(%d,%d); ", element_at(relation, index),
                    element_at(relation, index));
        }
    }
    fputc('\n', output);
}

static void write_irreflexivity_details(const Relation *relation, FILE *output) {
    size_t index;

    fputs("Irreflexive counterexamples: ", output);
    for (index = 0; index < relation_size(relation); index++) {
        if (relation_contains(relation, index, index)) {
            fprintf(output, "(%d,%d); ", element_at(relation, index),
                    element_at(relation, index));
        }
    }
    fputc('\n', output);
}

static void write_symmetry_details(const Relation *relation, FILE *output) {
    size_t source;
    size_t target;

    fputs("Missing symmetric pairs: ", output);
    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target) &&
                !relation_contains(relation, target, source)) {
                fprintf(output, "(%d,%d); ", element_at(relation, target),
                        element_at(relation, source));
            }
        }
    }
    fputc('\n', output);
}

static void write_antisymmetry_details(const Relation *relation, FILE *output) {
    size_t source;
    size_t target;

    fputs("Antisymmetric counterexamples: ", output);
    for (source = 0; source < relation_size(relation); source++) {
        for (target = source + 1; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target) &&
                relation_contains(relation, target, source)) {
                fprintf(output, "(%d,%d) and (%d,%d); ", element_at(relation, source),
                        element_at(relation, target), element_at(relation, target),
                        element_at(relation, source));
            }
        }
    }
    fputc('\n', output);
}

static void write_transitivity_details(const Relation *relation, const Relation *closure,
                                       FILE *output) {
    size_t source;
    size_t target;

    fputs("Missing transitive pairs: ", output);
    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(closure, source, target) &&
                !relation_contains(relation, source, target)) {
                fprintf(output, "(%d,%d); ", element_at(relation, source),
                        element_at(relation, target));
            }
        }
    }
    fputc('\n', output);
}

static void write_pair(FILE *output, int source, int target, bool *first) {
    if (!*first) {
        fputc(',', output);
    }
    fprintf(output, "(%d,%d)", source, target);
    *first = false;
}

static void write_closure(const char *name, const Relation *relation, const Relation *closure,
                          FILE *output) {
    bool first = true;
    size_t source;
    size_t target;

    fprintf(output, "%s closure = {", name);
    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(relation, source, target)) {
                write_pair(output, element_at(relation, source), element_at(relation, target),
                           &first);
            }
        }
    }

    for (source = 0; source < relation_size(relation); source++) {
        for (target = 0; target < relation_size(relation); target++) {
            if (relation_contains(closure, source, target) &&
                !relation_contains(relation, source, target)) {
                write_pair(output, element_at(relation, source), element_at(relation, target),
                           &first);
            }
        }
    }
    fputs("}\n", output);
}

static void write_analysis(const Relation *relation, const Relation *transitive_closure,
                           const RelationAnalysis *analysis, FILE *output) {
    fprintf(output, "1. Reflexive: %s\n", boolean_text(analysis->reflexive));
    if (!analysis->reflexive) {
        write_reflexivity_details(relation, output);
    }
    fprintf(output, "2. Irreflexive: %s\n", boolean_text(analysis->irreflexive));
    if (!analysis->irreflexive) {
        write_irreflexivity_details(relation, output);
    }
    fprintf(output, "3. Symmetric: %s\n", boolean_text(analysis->symmetric));
    if (!analysis->symmetric) {
        write_symmetry_details(relation, output);
    }
    fprintf(output, "4. Antisymmetric: %s\n", boolean_text(analysis->antisymmetric));
    if (!analysis->antisymmetric) {
        write_antisymmetry_details(relation, output);
    }
    fprintf(output, "5. Asymmetric: %s\n", boolean_text(analysis->asymmetric));
    fprintf(output, "6. Transitive: %s\n", boolean_text(analysis->transitive));
    if (!analysis->transitive) {
        write_transitivity_details(relation, transitive_closure, output);
    }
    fprintf(output, "\nEquivalence relation: %s\n", boolean_text(analysis->equivalence));
    fprintf(output, "Partial order: %s\n\n", boolean_text(analysis->partial_order));
}

bool relation_write_report(const Relation *relation, FILE *output) {
    RelationAnalysis analysis;
    Relation *reflexive_closure;
    Relation *symmetric_closure;
    Relation *transitive_closure;

    if (relation == NULL || output == NULL) {
        return false;
    }

    reflexive_closure = relation_reflexive_closure(relation);
    symmetric_closure = relation_symmetric_closure(relation);
    transitive_closure = relation_transitive_closure(relation);
    if (reflexive_closure == NULL || symmetric_closure == NULL || transitive_closure == NULL) {
        relation_destroy(reflexive_closure);
        relation_destroy(symmetric_closure);
        relation_destroy(transitive_closure);
        return false;
    }

    analysis = relation_analyze(relation);
    fputs("\nRelation properties:\n\n", output);
    write_matrix(relation, output);
    write_analysis(relation, transitive_closure, &analysis, output);
    write_closure("Reflexive", relation, reflexive_closure, output);
    write_closure("Symmetric", relation, symmetric_closure, output);
    write_closure("Transitive", relation, transitive_closure, output);
    fputc('\n', output);

    relation_destroy(reflexive_closure);
    relation_destroy(symmetric_closure);
    relation_destroy(transitive_closure);
    return !ferror(output);
}
