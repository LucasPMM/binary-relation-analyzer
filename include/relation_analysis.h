#ifndef BINARY_RELATION_ANALYZER_RELATION_ANALYSIS_H
#define BINARY_RELATION_ANALYZER_RELATION_ANALYSIS_H

#include <stdbool.h>

#include "relation.h"

typedef struct {
    bool reflexive;
    bool irreflexive;
    bool symmetric;
    bool antisymmetric;
    bool asymmetric;
    bool transitive;
    bool equivalence;
    bool partial_order;
} RelationAnalysis;

/**
 * Computes every supported property without modifying the relation.
 *
 * Properties over an empty set follow vacuous-truth semantics. Passing NULL
 * returns an all-false result to make invalid use distinguishable from empty.
 */
RelationAnalysis relation_analyze(const Relation *relation);

#endif
