#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "relation.h"

static void test_null_inputs(void) {
    int element = 0;
    size_t index = 0;

    assert(relation_size(NULL) == 0);
    assert(relation_pair_count(NULL) == 0);
    assert(relation_clone(NULL) == NULL);
    assert(!relation_element_at(NULL, 0, &element));
    assert(!relation_index_of(NULL, 1, &index));
    assert(!relation_contains(NULL, 0, 0));
    assert(!relation_add_pair(NULL, 1, 1));
    assert(!relation_add_pair_at(NULL, 0, 0));
    relation_destroy(NULL);
}

static void test_empty_relation(void) {
    Relation *relation = relation_create(NULL, 0);
    Relation *clone;
    int element = 0;
    size_t index = 0;

    assert(relation != NULL);
    assert(relation_size(relation) == 0);
    assert(relation_pair_count(relation) == 0);
    assert(!relation_element_at(relation, 0, &element));
    assert(!relation_index_of(relation, 0, &index));
    assert(!relation_contains(relation, 0, 0));
    assert(!relation_add_pair(relation, 0, 0));
    assert(!relation_add_pair_at(relation, 0, 0));

    clone = relation_clone(relation);
    assert(clone != NULL);
    assert(relation_size(clone) == 0);
    assert(relation_pair_count(clone) == 0);
    relation_destroy(clone);
    relation_destroy(relation);
}

static void test_invalid_elements(void) {
    const int duplicates[] = {10, 10};
    const int nonadjacent_duplicates[] = {10, 20, 10};
    const int element = 10;

    assert(relation_create(NULL, 1) == NULL);
    assert(relation_create(duplicates, 2) == NULL);
    assert(relation_create(nonadjacent_duplicates, 3) == NULL);
    assert(relation_create(&element, SIZE_MAX) == NULL);
}

static void test_dynamic_element_storage(void) {
    int elements[64];
    Relation *relation;
    size_t index;

    for (index = 0; index < 64; index++) {
        elements[index] = (int)index;
    }

    relation = relation_create(elements, 64);
    assert(relation != NULL);
    assert(relation_size(relation) == 64);
    assert(relation_add_pair_at(relation, 63, 63));
    assert(relation_contains(relation, 63, 63));
    relation_destroy(relation);
}

static void test_elements_and_pairs(void) {
    const int elements[] = {10, -20, 30};
    Relation *relation = relation_create(elements, 3);
    size_t index = 99;
    int element = 0;

    assert(relation != NULL);
    assert(relation_size(relation) == 3);
    assert(relation_element_at(relation, 1, &element));
    assert(element == -20);
    assert(relation_element_at(relation, 0, &element));
    assert(element == 10);
    assert(relation_element_at(relation, 2, &element));
    assert(element == 30);
    assert(!relation_element_at(relation, 3, &element));
    assert(!relation_element_at(relation, 0, NULL));
    assert(relation_index_of(relation, 10, &index));
    assert(index == 0);
    assert(relation_index_of(relation, 30, &index));
    assert(index == 2);
    assert(!relation_index_of(relation, 99, &index));
    assert(!relation_index_of(relation, 10, NULL));

    assert(relation_add_pair(relation, 10, -20));
    assert(relation_contains(relation, 0, 1));
    assert(relation_pair_count(relation) == 1);
    assert(relation_add_pair(relation, 10, -20));
    assert(relation_pair_count(relation) == 1);
    assert(relation_add_pair(relation, 30, 30));
    assert(relation_contains(relation, 2, 2));
    assert(relation_pair_count(relation) == 2);
    assert(!relation_add_pair(relation, 99, 10));
    assert(!relation_add_pair(relation, 10, 99));
    assert(!relation_add_pair_at(relation, 3, 0));
    assert(!relation_add_pair_at(relation, 0, 3));
    assert(!relation_contains(relation, 3, 0));
    assert(!relation_contains(relation, 0, 3));

    relation_destroy(relation);
}

static void test_clone_is_independent(void) {
    const int elements[] = {1, 2};
    Relation *original = relation_create(elements, 2);
    Relation *clone;

    assert(original != NULL);
    assert(relation_add_pair_at(original, 0, 1));
    clone = relation_clone(original);
    assert(clone != NULL);
    assert(relation_contains(clone, 0, 1));
    assert(relation_pair_count(clone) == 1);

    assert(relation_add_pair_at(clone, 1, 0));
    assert(relation_pair_count(clone) == 2);
    assert(!relation_contains(original, 1, 0));
    assert(relation_pair_count(original) == 1);

    relation_destroy(clone);
    relation_destroy(original);
}

int main(void) {
    test_null_inputs();
    test_empty_relation();
    test_invalid_elements();
    test_dynamic_element_storage();
    test_elements_and_pairs();
    test_clone_is_independent();
    return 0;
}
