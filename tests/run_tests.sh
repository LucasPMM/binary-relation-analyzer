#!/usr/bin/env bash

set -u

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
fixtures_dir="${script_dir}/fixtures"
binary_argument="${1:-./a.out}"

if [[ "${binary_argument}" = /* ]]; then
    binary="${binary_argument}"
else
    binary="$(cd -- "$(dirname -- "${binary_argument}")" && pwd)/$(basename -- "${binary_argument}")"
fi

passed=0
failed=0

run_success() {
    local fixture="$1"
    shift
    local output
    local expected
    local case_failed=0

    if ! output="$(cd -- "${fixtures_dir}/${fixture}" && "${binary}" 2>&1)"; then
        printf 'FAIL %s: expected a successful exit\n%s\n' "${fixture}" "${output}"
        failed=$((failed + 1))
        return
    fi

    if [[ "${output}" == *'ERROR: AddressSanitizer'* ||
          "${output}" == *'runtime error:'* ||
          "${output}" == *'LeakSanitizer'* ]]; then
        printf 'FAIL %s: sanitizer diagnostic detected\n%s\n' "${fixture}" "${output}"
        failed=$((failed + 1))
        return
    fi

    for expected in "$@"; do
        if [[ "${output}" != *"${expected}"* ]]; then
            printf 'FAIL %s: missing output: %s\n' "${fixture}" "${expected}"
            case_failed=1
        fi
    done

    if ((case_failed)); then
        printf '%s\n' "${output}"
        failed=$((failed + 1))
    else
        printf 'PASS %s\n' "${fixture}"
        passed=$((passed + 1))
    fi
}

run_failure() {
    local fixture="$1"
    local expected="$2"
    local output

    if output="$(cd -- "${fixtures_dir}/${fixture}" && "${binary}" 2>&1)"; then
        printf 'FAIL %s: expected a non-zero exit\n%s\n' "${fixture}" "${output}"
        failed=$((failed + 1))
        return
    fi

    if [[ "${output}" == *'ERROR: AddressSanitizer'* ||
          "${output}" == *'runtime error:'* ||
          "${output}" == *'LeakSanitizer'* ]]; then
        printf 'FAIL %s: sanitizer diagnostic detected\n%s\n' "${fixture}" "${output}"
        failed=$((failed + 1))
        return
    fi

    if [[ "${output}" != *"${expected}"* ]]; then
        printf 'FAIL %s: missing error: %s\n%s\n' "${fixture}" "${expected}" "${output}"
        failed=$((failed + 1))
        return
    fi

    printf 'PASS %s\n' "${fixture}"
    passed=$((passed + 1))
}

run_success legacy_sample \
    '1. Reflexive: true' \
    '3. Symmetric: true' \
    '4. Antisymmetric: false' \
    '6. Transitive: true' \
    'Equivalence relation: true' \
    'Partial order: false'

run_success empty_set \
    '1. Reflexive: true' \
    '2. Irreflexive: true' \
    '3. Symmetric: true' \
    '4. Antisymmetric: true' \
    '5. Asymmetric: true' \
    '6. Transitive: true' \
    'Equivalence relation: true' \
    'Partial order: true' \
    'Reflexive closure = {}' \
    'Symmetric closure = {}' \
    'Transitive closure = {}'

run_success empty_relation \
    '1. Reflexive: false' \
    '2. Irreflexive: true' \
    '3. Symmetric: true' \
    '4. Antisymmetric: true' \
    '5. Asymmetric: true' \
    '6. Transitive: true' \
    'Equivalence relation: false' \
    'Partial order: false' \
    'Reflexive closure = {(1,1),(2,2)}'

run_success equivalence_multi_digit \
    '1. Reflexive: true' \
    '2. Irreflexive: false' \
    '3. Symmetric: true' \
    '4. Antisymmetric: false' \
    '5. Asymmetric: false' \
    '6. Transitive: true' \
    'Equivalence relation: true' \
    'Partial order: false'

run_success partial_order \
    '1. Reflexive: true' \
    '3. Symmetric: false' \
    '4. Antisymmetric: true' \
    '6. Transitive: true' \
    'Equivalence relation: false' \
    'Partial order: true'

run_success asymmetric_transitive \
    '1. Reflexive: false' \
    '2. Irreflexive: true' \
    '3. Symmetric: false' \
    '4. Antisymmetric: true' \
    '5. Asymmetric: true' \
    '6. Transitive: true'

run_success symmetric_non_transitive \
    '2. Irreflexive: true' \
    '3. Symmetric: true' \
    '4. Antisymmetric: false' \
    '5. Asymmetric: false' \
    '6. Transitive: false'

run_success non_transitive_closure \
    '6. Transitive: false' \
    '(1,3);' \
    'Transitive closure = {(1,2),(2,3),(1,3)}'

run_success duplicate_pair \
    'Symmetric closure = {(10,20),(20,10)}'

run_failure malformed_header 'wrong number of elements'
run_failure duplicate_element 'duplicate elements'
run_failure unknown_element 'unknown element'
run_failure malformed_pair 'invalid ordered pair'
run_failure invalid_count 'invalid element count'
run_failure missing_file 'could not open input file input.txt'

printf '\n%d passed, %d failed\n' "${passed}" "${failed}"
((failed == 0))
