#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "relation.h"
#include "relation_io.h"
#include "relation_report.h"

static void test_load_valid_relation(void) {
    Relation *relation =
        relation_load_from_file("tests/fixtures/equivalence_multi_digit/input.txt");
    int first = 0;
    int second = 0;

    assert(relation != NULL);
    assert(relation_size(relation) == 2);
    assert(relation_pair_count(relation) == 4);
    assert(relation_element_at(relation, 0, &first));
    assert(relation_element_at(relation, 1, &second));
    assert(first == 10);
    assert(second == -20);
    relation_destroy(relation);
}

static void test_report_content_and_immutability(void) {
    const int elements[] = {1, 2};
    Relation *relation = relation_create(elements, 2);
    FILE *output = tmpfile();
    char report[4096];
    size_t bytes_read;

    assert(relation != NULL);
    assert(output != NULL);
    assert(relation_add_pair(relation, 1, 2));
    assert(relation_write_report(relation, output));
    assert(relation_pair_count(relation) == 1);
    assert(relation_contains(relation, 0, 1));
    assert(!relation_contains(relation, 1, 0));

    rewind(output);
    bytes_read = fread(report, 1, sizeof(report) - 1, output);
    assert(!ferror(output));
    report[bytes_read] = '\0';
    assert(strstr(report, "Relation properties:") != NULL);
    assert(strstr(report, "3. Symmetric: false") != NULL);
    assert(strstr(report, "Missing symmetric pairs: (2,1);") != NULL);
    assert(strstr(report, "Symmetric closure = {(1,2),(2,1)}") != NULL);

    assert(!relation_write_report(NULL, output));
    assert(!relation_write_report(relation, NULL));
    fclose(output);
    relation_destroy(relation);
}

int main(void) {
    test_load_valid_relation();
    test_report_content_and_immutability();
    return 0;
}
