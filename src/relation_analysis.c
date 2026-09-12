#include <stdbool.h>
#include <stddef.h>

#include "relation_analysis.h"

static bool is_transitive(const Relation *relation) {
    size_t size = relation_size(relation);
    size_t source;
    size_t intermediate;
    size_t target;

    for (source = 0; source < size; source++) {
        for (intermediate = 0; intermediate < size; intermediate++) {
            if (!relation_contains(relation, source, intermediate)) {
                continue;
            }
            for (target = 0; target < size; target++) {
                if (relation_contains(relation, intermediate, target) &&
                    !relation_contains(relation, source, target)) {
                    return false;
                }
            }
        }
    }
    return true;
}

RelationAnalysis relation_analyze(const Relation *relation) {
    RelationAnalysis analysis = {0};
    size_t size;
    size_t source;
    size_t target;

    if (relation == NULL) {
        return analysis;
    }

    /* Every universal property starts true, including for the empty set. */
    analysis.reflexive = true;
    analysis.irreflexive = true;
    analysis.symmetric = true;
    analysis.antisymmetric = true;
    analysis.asymmetric = true;
    analysis.transitive = true;
    size = relation_size(relation);

    for (source = 0; source < size; source++) {
        if (relation_contains(relation, source, source)) {
            analysis.irreflexive = false;
            analysis.asymmetric = false;
        } else {
            analysis.reflexive = false;
        }

        for (target = source + 1; target < size; target++) {
            bool forward = relation_contains(relation, source, target);
            bool reverse = relation_contains(relation, target, source);

            if (forward != reverse) {
                analysis.symmetric = false;
            }
            if (forward && reverse) {
                analysis.antisymmetric = false;
                analysis.asymmetric = false;
            }
        }
    }

    analysis.transitive = is_transitive(relation);
    analysis.equivalence = analysis.reflexive && analysis.symmetric && analysis.transitive;
    analysis.partial_order = analysis.reflexive && analysis.antisymmetric && analysis.transitive;
    return analysis;
}
