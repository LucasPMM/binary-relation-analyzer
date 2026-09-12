#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "relation.h"
#include "relation_analysis.h"
#include "relation_closure.h"

#define MAX_EXHAUSTIVE_ELEMENTS 3

typedef bool ReferenceMatrix[MAX_EXHAUSTIVE_ELEMENTS][MAX_EXHAUSTIVE_ELEMENTS];

static void matrix_from_mask(ReferenceMatrix matrix, size_t size, unsigned int mask) {
    size_t source;
    size_t target;

    for (source = 0; source < size; source++) {
        for (target = 0; target < size; target++) {
            size_t bit = source * size + target;
            matrix[source][target] = (mask & (1U << bit)) != 0;
        }
    }
}

static void copy_matrix(ReferenceMatrix destination, ReferenceMatrix source, size_t size) {
    size_t row;
    size_t column;

    for (row = 0; row < size; row++) {
        for (column = 0; column < size; column++) {
            destination[row][column] = source[row][column];
        }
    }
}

static Relation *relation_from_matrix(ReferenceMatrix matrix, size_t size) {
    const int elements[] = {10, 20, 30};
    Relation *relation = relation_create(elements, size);
    size_t source;
    size_t target;

    assert(relation != NULL);
    for (source = 0; source < size; source++) {
        for (target = 0; target < size; target++) {
            if (matrix[source][target]) {
                assert(relation_add_pair_at(relation, source, target));
            }
        }
    }
    return relation;
}

static RelationAnalysis reference_analysis(ReferenceMatrix matrix, size_t size) {
    RelationAnalysis analysis = {true, true, true, true, true, true, false, false};
    size_t source;
    size_t intermediate;
    size_t target;

    for (source = 0; source < size; source++) {
        if (matrix[source][source]) {
            analysis.irreflexive = false;
        } else {
            analysis.reflexive = false;
        }

        for (target = 0; target < size; target++) {
            if (matrix[source][target] != matrix[target][source]) {
                analysis.symmetric = false;
            }
            if (source != target && matrix[source][target] && matrix[target][source]) {
                analysis.antisymmetric = false;
            }
            if (matrix[source][target] && matrix[target][source]) {
                analysis.asymmetric = false;
            }
        }
    }

    for (source = 0; source < size; source++) {
        for (intermediate = 0; intermediate < size; intermediate++) {
            for (target = 0; target < size; target++) {
                if (matrix[source][intermediate] && matrix[intermediate][target] &&
                    !matrix[source][target]) {
                    analysis.transitive = false;
                }
            }
        }
    }

    analysis.equivalence = analysis.reflexive && analysis.symmetric && analysis.transitive;
    analysis.partial_order = analysis.reflexive && analysis.antisymmetric && analysis.transitive;
    return analysis;
}

static void compute_reference_closures(ReferenceMatrix original, size_t size,
                                       ReferenceMatrix reflexive, ReferenceMatrix symmetric,
                                       ReferenceMatrix transitive) {
    bool changed;
    size_t source;
    size_t intermediate;
    size_t target;

    copy_matrix(reflexive, original, size);
    copy_matrix(symmetric, original, size);
    copy_matrix(transitive, original, size);

    for (source = 0; source < size; source++) {
        reflexive[source][source] = true;
        for (target = 0; target < size; target++) {
            if (original[source][target]) {
                symmetric[target][source] = true;
            }
        }
    }

    /* Saturation is intentionally independent from the production Warshall loop. */
    do {
        changed = false;
        for (source = 0; source < size; source++) {
            for (intermediate = 0; intermediate < size; intermediate++) {
                for (target = 0; target < size; target++) {
                    if (transitive[source][intermediate] && transitive[intermediate][target] &&
                        !transitive[source][target]) {
                        transitive[source][target] = true;
                        changed = true;
                    }
                }
            }
        }
    } while (changed);
}

static size_t matrix_pair_count(ReferenceMatrix matrix, size_t size) {
    size_t count = 0;
    size_t source;
    size_t target;

    for (source = 0; source < size; source++) {
        for (target = 0; target < size; target++) {
            if (matrix[source][target]) {
                count++;
            }
        }
    }
    return count;
}

static void assert_analysis_equal(RelationAnalysis actual, RelationAnalysis expected) {
    assert(actual.reflexive == expected.reflexive);
    assert(actual.irreflexive == expected.irreflexive);
    assert(actual.symmetric == expected.symmetric);
    assert(actual.antisymmetric == expected.antisymmetric);
    assert(actual.asymmetric == expected.asymmetric);
    assert(actual.transitive == expected.transitive);
    assert(actual.equivalence == expected.equivalence);
    assert(actual.partial_order == expected.partial_order);
}

static void assert_relation_equals_matrix(const Relation *relation,
                                          ReferenceMatrix expected, size_t size) {
    size_t source;
    size_t target;

    assert(relation_size(relation) == size);
    assert(relation_pair_count(relation) == matrix_pair_count(expected, size));
    for (source = 0; source < size; source++) {
        for (target = 0; target < size; target++) {
            assert(relation_contains(relation, source, target) == expected[source][target]);
        }
    }
}

static void verify_relation(size_t size, unsigned int mask) {
    ReferenceMatrix original = {{false}};
    ReferenceMatrix reflexive = {{false}};
    ReferenceMatrix symmetric = {{false}};
    ReferenceMatrix transitive = {{false}};
    Relation *relation;
    Relation *reflexive_closure;
    Relation *symmetric_closure;
    Relation *transitive_closure;

    matrix_from_mask(original, size, mask);
    relation = relation_from_matrix(original, size);
    assert_analysis_equal(relation_analyze(relation), reference_analysis(original, size));

    compute_reference_closures(original, size, reflexive, symmetric, transitive);
    reflexive_closure = relation_reflexive_closure(relation);
    symmetric_closure = relation_symmetric_closure(relation);
    transitive_closure = relation_transitive_closure(relation);
    assert(reflexive_closure != NULL);
    assert(symmetric_closure != NULL);
    assert(transitive_closure != NULL);

    assert_relation_equals_matrix(reflexive_closure, reflexive, size);
    assert_relation_equals_matrix(symmetric_closure, symmetric, size);
    assert_relation_equals_matrix(transitive_closure, transitive, size);
    assert_relation_equals_matrix(relation, original, size);
    assert(relation_analyze(reflexive_closure).reflexive);
    assert(relation_analyze(symmetric_closure).symmetric);
    assert(relation_analyze(transitive_closure).transitive);

    relation_destroy(reflexive_closure);
    relation_destroy(symmetric_closure);
    relation_destroy(transitive_closure);
    relation_destroy(relation);
}

int main(void) {
    size_t size;

    /* Enumerate every binary relation over sets containing up to three elements. */
    for (size = 0; size <= MAX_EXHAUSTIVE_ELEMENTS; size++) {
        unsigned int relation_count = 1U << (size * size);
        unsigned int mask;

        for (mask = 0; mask < relation_count; mask++) {
            verify_relation(size, mask);
        }
    }
    return 0;
}
