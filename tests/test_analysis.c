#include <assert.h>
#include <stdbool.h>

#include "relation.h"
#include "relation_analysis.h"
#include "relation_closure.h"

static Relation *create_relation(const int *elements, size_t element_count,
                                 const int (*pairs)[2], size_t pair_count) {
    Relation *relation = relation_create(elements, element_count);
    size_t index;

    assert(relation != NULL);
    for (index = 0; index < pair_count; index++) {
        assert(relation_add_pair(relation, pairs[index][0], pairs[index][1]));
    }
    return relation;
}

static void test_empty_set_properties(void) {
    Relation *relation = relation_create(NULL, 0);
    RelationAnalysis analysis;

    assert(relation != NULL);
    analysis = relation_analyze(relation);
    assert(analysis.reflexive);
    assert(analysis.irreflexive);
    assert(analysis.symmetric);
    assert(analysis.antisymmetric);
    assert(analysis.asymmetric);
    assert(analysis.transitive);
    assert(analysis.equivalence);
    assert(analysis.partial_order);
    relation_destroy(relation);
}

static void test_equivalence_properties(void) {
    const int elements[] = {10, 20};
    const int pairs[][2] = {{10, 10}, {10, 20}, {20, 10}, {20, 20}};
    Relation *relation = create_relation(elements, 2, pairs, 4);
    RelationAnalysis analysis = relation_analyze(relation);

    assert(analysis.reflexive);
    assert(!analysis.irreflexive);
    assert(analysis.symmetric);
    assert(!analysis.antisymmetric);
    assert(!analysis.asymmetric);
    assert(analysis.transitive);
    assert(analysis.equivalence);
    assert(!analysis.partial_order);
    relation_destroy(relation);
}

static void test_partial_order_properties(void) {
    const int elements[] = {1, 2, 3};
    const int pairs[][2] = {{1, 1}, {2, 2}, {3, 3}, {1, 2}, {2, 3}, {1, 3}};
    Relation *relation = create_relation(elements, 3, pairs, 6);
    RelationAnalysis analysis = relation_analyze(relation);

    assert(analysis.reflexive);
    assert(!analysis.irreflexive);
    assert(!analysis.symmetric);
    assert(analysis.antisymmetric);
    assert(!analysis.asymmetric);
    assert(analysis.transitive);
    assert(!analysis.equivalence);
    assert(analysis.partial_order);
    relation_destroy(relation);
}

static void test_asymmetric_and_non_transitive_properties(void) {
    const int elements[] = {1, 2, 3};
    const int asymmetric_pairs[][2] = {{1, 2}, {2, 3}, {1, 3}};
    const int non_transitive_pairs[][2] = {{1, 2}, {2, 3}};
    Relation *asymmetric = create_relation(elements, 3, asymmetric_pairs, 3);
    Relation *non_transitive = create_relation(elements, 3, non_transitive_pairs, 2);
    RelationAnalysis analysis = relation_analyze(asymmetric);

    assert(analysis.irreflexive);
    assert(analysis.antisymmetric);
    assert(analysis.asymmetric);
    assert(analysis.transitive);

    analysis = relation_analyze(non_transitive);
    assert(!analysis.transitive);
    relation_destroy(asymmetric);
    relation_destroy(non_transitive);
}

static void test_closures_are_independent_and_minimal(void) {
    const int elements[] = {1, 2, 3};
    const int pairs[][2] = {{1, 2}, {2, 3}};
    Relation *relation = create_relation(elements, 3, pairs, 2);
    Relation *reflexive = relation_reflexive_closure(relation);
    Relation *symmetric = relation_symmetric_closure(relation);
    Relation *transitive = relation_transitive_closure(relation);

    assert(reflexive != NULL);
    assert(symmetric != NULL);
    assert(transitive != NULL);

    assert(relation_pair_count(reflexive) == 5);
    assert(relation_contains(reflexive, 0, 0));
    assert(relation_contains(reflexive, 1, 1));
    assert(relation_contains(reflexive, 2, 2));

    assert(relation_pair_count(symmetric) == 4);
    assert(relation_contains(symmetric, 1, 0));
    assert(relation_contains(symmetric, 2, 1));

    assert(relation_pair_count(transitive) == 3);
    assert(relation_contains(transitive, 0, 2));

    assert(relation_pair_count(relation) == 2);
    assert(!relation_contains(relation, 0, 0));
    assert(!relation_contains(relation, 1, 0));
    assert(!relation_contains(relation, 0, 2));

    relation_destroy(reflexive);
    relation_destroy(symmetric);
    relation_destroy(transitive);
    relation_destroy(relation);
}

int main(void) {
    test_empty_set_properties();
    test_equivalence_properties();
    test_partial_order_properties();
    test_asymmetric_and_non_transitive_properties();
    test_closures_are_independent_and_minimal();
    return 0;
}
