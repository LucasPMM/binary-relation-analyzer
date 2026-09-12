#ifndef BINARY_RELATION_ANALYZER_RELATION_CLOSURE_H
#define BINARY_RELATION_ANALYZER_RELATION_CLOSURE_H

#include "relation.h"

Relation *relation_reflexive_closure(const Relation *relation);
Relation *relation_symmetric_closure(const Relation *relation);
Relation *relation_transitive_closure(const Relation *relation);

#endif
