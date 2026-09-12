#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "relation.h"

static void test_empty_relation(void) {
    Relation *relation = relation_create(NULL, 0);

    assert(relation != NULL);
    assert(relation_size(relation) == 0);
    assert(relation_pair_count(relation) == 0);
    assert(!relation_contains(relation, 0, 0));
    relation_destroy(relation);
}

static void test_invalid_elements(void) {
    const int duplicates[] = {10, 10};
    const int element = 10;

    assert(relation_create(NULL, 1) == NULL);
    assert(relation_create(duplicates, 2) == NULL);
    assert(relation_create(&element, SIZE_MAX) == NULL);
    assert(relation_clone(NULL) == NULL);
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
    assert(!relation_element_at(relation, 3, &element));
    assert(relation_index_of(relation, 30, &index));
    assert(index == 2);
    assert(!relation_index_of(relation, 99, &index));

    assert(relation_add_pair(relation, 10, -20));
    assert(relation_contains(relation, 0, 1));
    assert(relation_pair_count(relation) == 1);
    assert(relation_add_pair(relation, 10, -20));
    assert(relation_pair_count(relation) == 1);
    assert(!relation_add_pair(relation, 10, 99));
    assert(!relation_add_pair_at(relation, 3, 0));

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
    test_empty_relation();
    test_invalid_elements();
    test_dynamic_element_storage();
    test_elements_and_pairs();
    test_clone_is_independent();
    relation_destroy(NULL);
    return 0;
}
