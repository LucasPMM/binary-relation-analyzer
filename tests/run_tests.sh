#!/usr/bin/env bash

set -u

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
fixtures_dir="${script_dir}/fixtures"
binary_argument="${1:-./binary-relation-analyzer}"

if [[ "${binary_argument}" = /* ]]; then
    binary="${binary_argument}"
else
    binary="$(cd -- "$(dirname -- "${binary_argument}")" && pwd)/$(basename -- "${binary_argument}")"
fi

temporary_dir="$(mktemp -d)"
trap 'rm -rf -- "${temporary_dir}"' EXIT

passed=0
failed=0

run_success() {
    local fixture="$1"
    shift
    local output
    local expected
    local case_failed=0

    if ! output="$(cd -- "${fixtures_dir}/${fixture}" && "${binary}" input.txt 2>&1)"; then
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

    if output="$(cd -- "${fixtures_dir}/${fixture}" && "${binary}" input.txt 2>&1)"; then
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

run_cli_success() {
    local label="$1"
    local expected="$2"
    shift 2
    local output

    if ! output="$("${binary}" "$@" 2>&1)"; then
        printf 'FAIL %s: expected a successful exit\n%s\n' "${label}" "${output}"
        failed=$((failed + 1))
        return
    fi
    if [[ "${output}" == *'ERROR: AddressSanitizer'* ||
          "${output}" == *'runtime error:'* ||
          "${output}" == *'LeakSanitizer'* ]]; then
        printf 'FAIL %s: sanitizer diagnostic detected\n%s\n' "${label}" "${output}"
        failed=$((failed + 1))
        return
    fi
    if [[ "${output}" != *"${expected}"* ]]; then
        printf 'FAIL %s: missing output: %s\n%s\n' "${label}" "${expected}" "${output}"
        failed=$((failed + 1))
        return
    fi

    printf 'PASS %s\n' "${label}"
    passed=$((passed + 1))
}

run_cli_failure() {
    local label="$1"
    local expected="$2"
    shift 2
    local output

    if output="$("${binary}" "$@" 2>&1)"; then
        printf 'FAIL %s: expected a non-zero exit\n%s\n' "${label}" "${output}"
        failed=$((failed + 1))
        return
    fi
    if [[ "${output}" == *'ERROR: AddressSanitizer'* ||
          "${output}" == *'runtime error:'* ||
          "${output}" == *'LeakSanitizer'* ]]; then
        printf 'FAIL %s: sanitizer diagnostic detected\n%s\n' "${label}" "${output}"
        failed=$((failed + 1))
        return
    fi
    if [[ "${output}" != *"${expected}"* ]]; then
        printf 'FAIL %s: missing error: %s\n%s\n' "${label}" "${expected}" "${output}"
        failed=$((failed + 1))
        return
    fi

    printf 'PASS %s\n' "${label}"
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
run_failure negative_count 'invalid element count'
run_failure invalid_count_token 'invalid element count'
run_failure overflowing_element 'wrong number of elements'
run_failure extra_header_element 'wrong number of elements'
run_failure nonadjacent_duplicate_element 'duplicate elements'
run_failure incomplete_pair 'invalid ordered pair'
run_failure invalid_pair_token 'invalid ordered pair'
run_failure overflowing_pair 'invalid ordered pair'
run_failure unknown_source 'unknown element'
run_failure blank_header 'invalid element count'

run_success whitespace_and_blank_lines \
    '1. Reflexive: false' \
    'Symmetric closure = {(10,-20),(-20,10)}'

run_success integer_boundaries \
    'Missing symmetric pairs: (2147483647,-2147483648);' \
    'Symmetric closure = {(-2147483648,2147483647),(2147483647,-2147483648)}'

run_success maximum_element_count \
    'Relation properties:' \
    '1. Reflexive: false' \
    'Partial order: false'

run_cli_success help_long 'Usage: binary-relation-analyzer [--] INPUT_FILE' --help
run_cli_success help_short 'Analyze the properties and closures' -h
run_cli_success explicit_path 'Equivalence relation: true' \
    "${fixtures_dir}/legacy_sample/input.txt"
run_cli_success option_separator 'Relation properties:' -- \
    "${fixtures_dir}/empty_set/input.txt"
run_cli_failure no_arguments 'expected exactly one input file'
run_cli_failure too_many_arguments 'expected exactly one input file' \
    "${fixtures_dir}/empty_set/input.txt" extra
run_cli_failure unknown_option 'unknown option: --version' --version
run_cli_failure missing_after_separator 'expected an input file after --' --
run_cli_failure help_with_argument 'expected exactly one input file' --help extra
run_cli_failure empty_path 'could not open input file' ''

: >"${temporary_dir}/empty.txt"
run_cli_failure empty_file 'missing or overlong header' "${temporary_dir}/empty.txt"

{
    for ((character = 0; character < 5000; character++)); do
        printf '0'
    done
    printf '\n'
} >"${temporary_dir}/overlong-header.txt"
run_cli_failure overlong_header 'missing or overlong header' \
    "${temporary_dir}/overlong-header.txt"

{
    printf '1 1\n'
    for ((character = 0; character < 5000; character++)); do
        printf ' '
    done
    printf '\n'
} >"${temporary_dir}/overlong-pair.txt"
run_cli_failure overlong_pair 'line is too long' "${temporary_dir}/overlong-pair.txt"

printf '\n%d passed, %d failed\n' "${passed}" "${failed}"
((failed == 0))
