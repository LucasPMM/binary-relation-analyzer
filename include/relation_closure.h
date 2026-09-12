#ifndef BINARY_RELATION_ANALYZER_RELATION_CLOSURE_H
#define BINARY_RELATION_ANALYZER_RELATION_CLOSURE_H

#include "relation.h"

/** Returns a new minimal reflexive superset of relation. */
Relation *relation_reflexive_closure(const Relation *relation);

/** Returns a new minimal symmetric superset of relation. */
Relation *relation_symmetric_closure(const Relation *relation);

/** Returns a new minimal transitive superset of relation. */
Relation *relation_transitive_closure(const Relation *relation);

#endif
