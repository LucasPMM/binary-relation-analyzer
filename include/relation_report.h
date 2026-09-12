#ifndef BINARY_RELATION_ANALYZER_RELATION_REPORT_H
#define BINARY_RELATION_ANALYZER_RELATION_REPORT_H

#include <stdbool.h>
#include <stdio.h>

#include "relation.h"

/** Writes the matrix, analysis and closures without modifying relation. */
bool relation_write_report(const Relation *relation, FILE *output);

#endif
