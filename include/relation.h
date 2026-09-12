#ifndef BINARY_RELATION_ANALYZER_RELATION_H
#define BINARY_RELATION_ANALYZER_RELATION_H

#include <stdbool.h>
#include <stddef.h>

typedef struct Relation Relation;

/**
 * Creates a relation over a copy of the supplied unique elements.
 *
 * Passing NULL is valid only when element_count is zero. Returns NULL when the
 * arguments are invalid or storage cannot be allocated.
 */
Relation *relation_create(const int *elements, size_t element_count);

/** Creates a deep copy whose elements and pairs can be changed independently. */
Relation *relation_clone(const Relation *relation);

/** Releases a relation and accepts NULL as a no-op. */
void relation_destroy(Relation *relation);

/** Returns the number of elements, or zero when relation is NULL. */
size_t relation_size(const Relation *relation);

/** Returns the number of distinct ordered pairs, or zero for NULL. */
size_t relation_pair_count(const Relation *relation);

/** Copies the element at index into element and returns whether the index is valid. */
bool relation_element_at(const Relation *relation, size_t index, int *element);

/** Looks up an element and writes its matrix index when found. */
bool relation_index_of(const Relation *relation, int element, size_t *index);

/** Tests membership of an index-based ordered pair. Invalid indices return false. */
bool relation_contains(const Relation *relation, size_t source_index, size_t target_index);

/** Adds a pair identified by element values. Duplicate pairs are idempotent. */
bool relation_add_pair(Relation *relation, int source, int target);

/** Adds a pair identified by matrix indices. Duplicate pairs are idempotent. */
bool relation_add_pair_at(Relation *relation, size_t source_index, size_t target_index);

#endif
