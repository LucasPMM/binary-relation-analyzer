#ifndef BINARY_RELATION_ANALYZER_RELATION_IO_H
#define BINARY_RELATION_ANALYZER_RELATION_IO_H

#include "relation.h"

/**
 * Loads a relation from the documented whitespace-separated text format.
 * Returns NULL and writes a diagnostic to stderr when input is invalid.
 */
Relation *relation_load_from_file(const char *filename);

#endif
