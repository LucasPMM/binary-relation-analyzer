#include <stddef.h>

#include "relation_closure.h"

Relation *relation_reflexive_closure(const Relation *relation) {
    Relation *closure = relation_clone(relation);
    size_t index;

    if (closure == NULL) {
        return NULL;
    }

    for (index = 0; index < relation_size(closure); index++) {
        (void)relation_add_pair_at(closure, index, index);
    }
    return closure;
}

Relation *relation_symmetric_closure(const Relation *relation) {
    Relation *closure = relation_clone(relation);
    size_t size = relation_size(relation);
    size_t source;
    size_t target;

    if (closure == NULL) {
        return NULL;
    }

    for (source = 0; source < size; source++) {
        for (target = 0; target < size; target++) {
            if (relation_contains(relation, source, target)) {
                (void)relation_add_pair_at(closure, target, source);
            }
        }
    }
    return closure;
}

Relation *relation_transitive_closure(const Relation *relation) {
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
