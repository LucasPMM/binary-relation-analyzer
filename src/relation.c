#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "relation.h"

struct Relation {
    int *elements;
    /* Ordered-pair membership is stored as a row-major square matrix. */
    bool *pairs;
    size_t element_count;
    size_t pair_count;
};

static bool allocation_sizes_are_valid(size_t element_count) {
    size_t pair_capacity;

    /* Validate both multiplications before allocating attacker-controlled sizes. */
    if (element_count > SIZE_MAX / sizeof(int)) {
        return false;
    }
    if (element_count > 0 && element_count > SIZE_MAX / element_count) {
        return false;
    }
    pair_capacity = element_count * element_count;
    return pair_capacity <= SIZE_MAX / sizeof(bool);
}

static bool elements_are_unique(const int *elements, size_t element_count) {
    size_t current;
    size_t previous;

    for (current = 0; current < element_count; current++) {
        for (previous = 0; previous < current; previous++) {
            if (elements[current] == elements[previous]) {
                return false;
            }
        }
    }
    return true;
}

static size_t pair_offset(const Relation *relation, size_t source_index, size_t target_index) {
    return source_index * relation->element_count + target_index;
}

Relation *relation_create(const int *elements, size_t element_count) {
    Relation *relation;

    if ((element_count > 0 && elements == NULL) || !allocation_sizes_are_valid(element_count) ||
        !elements_are_unique(elements, element_count)) {
        return NULL;
    }

    relation = calloc(1, sizeof(*relation));
    if (relation == NULL) {
        return NULL;
    }

    if (element_count > 0) {
        /* Relation owns its labels so callers may release their input array. */
        relation->elements = malloc(element_count * sizeof(*relation->elements));
        relation->pairs = calloc(element_count * element_count, sizeof(*relation->pairs));
        if (relation->elements == NULL || relation->pairs == NULL) {
            relation_destroy(relation);
            return NULL;
        }
        memcpy(relation->elements, elements, element_count * sizeof(*relation->elements));
    }

    relation->element_count = element_count;
    return relation;
}

Relation *relation_clone(const Relation *relation) {
    Relation *clone;

    if (relation == NULL) {
        return NULL;
    }

    clone = relation_create(relation->elements, relation->element_count);
    if (clone == NULL) {
        return NULL;
    }

    if (relation->element_count > 0) {
        memcpy(clone->pairs, relation->pairs,
               relation->element_count * relation->element_count * sizeof(*clone->pairs));
    }
    clone->pair_count = relation->pair_count;
    return clone;
}

void relation_destroy(Relation *relation) {
    if (relation == NULL) {
        return;
    }
    free(relation->pairs);
    free(relation->elements);
    free(relation);
}

size_t relation_size(const Relation *relation) {
    return relation == NULL ? 0 : relation->element_count;
}

size_t relation_pair_count(const Relation *relation) {
    return relation == NULL ? 0 : relation->pair_count;
}

bool relation_element_at(const Relation *relation, size_t index, int *element) {
    if (relation == NULL || element == NULL || index >= relation->element_count) {
        return false;
    }
    *element = relation->elements[index];
    return true;
}

bool relation_index_of(const Relation *relation, int element, size_t *index) {
    size_t current;

    if (relation == NULL || index == NULL) {
        return false;
    }

    for (current = 0; current < relation->element_count; current++) {
        if (relation->elements[current] == element) {
            *index = current;
            return true;
        }
    }
    return false;
}

bool relation_contains(const Relation *relation, size_t source_index, size_t target_index) {
    if (relation == NULL || source_index >= relation->element_count ||
        target_index >= relation->element_count) {
        return false;
    }
    return relation->pairs[pair_offset(relation, source_index, target_index)];
}

bool relation_add_pair(Relation *relation, int source, int target) {
    size_t source_index;
    size_t target_index;

    if (!relation_index_of(relation, source, &source_index) ||
        !relation_index_of(relation, target, &target_index)) {
        return false;
    }
    return relation_add_pair_at(relation, source_index, target_index);
}

bool relation_add_pair_at(Relation *relation, size_t source_index, size_t target_index) {
    size_t offset;

    if (relation == NULL || source_index >= relation->element_count ||
        target_index >= relation->element_count) {
        return false;
    }

    offset = pair_offset(relation, source_index, target_index);
    if (!relation->pairs[offset]) {
        relation->pairs[offset] = true;
        relation->pair_count++;
    }
    return true;
}
