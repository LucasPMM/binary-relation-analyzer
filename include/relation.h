#ifndef BINARY_RELATION_ANALYZER_RELATION_H
#define BINARY_RELATION_ANALYZER_RELATION_H

#include <stdbool.h>
#include <stddef.h>

typedef struct Relation Relation;

Relation *relation_create(const int *elements, size_t element_count);
Relation *relation_clone(const Relation *relation);
void relation_destroy(Relation *relation);

size_t relation_size(const Relation *relation);
size_t relation_pair_count(const Relation *relation);
bool relation_element_at(const Relation *relation, size_t index, int *element);
bool relation_index_of(const Relation *relation, int element, size_t *index);
bool relation_contains(const Relation *relation, size_t source_index, size_t target_index);
bool relation_add_pair(Relation *relation, int source, int target);
bool relation_add_pair_at(Relation *relation, size_t source_index, size_t target_index);

#endif
