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
    '1. Reflexiva: V' \
    '3. Simetrica: V' \
    '4. Anti-simetrica: F' \
    '6. Transitiva: V' \
    'Relaçao de equivalencia: V' \
    'Relaçao de ordem parcial: F'

run_success empty_set \
    '1. Reflexiva: V' \
    '2. Irreflexiva: V' \
    '3. Simetrica: V' \
    '4. Anti-simetrica: V' \
    '5. Asimetrica: V' \
    '6. Transitiva: V' \
    'Relaçao de equivalencia: V' \
    'Relaçao de ordem parcial: V' \
    'Fecho reflexivo da relaçao = {}' \
    'Fecho simetrico da relaçao = {}' \
    'Fecho transitivo da relaçao = {}'

run_success empty_relation \
    '1. Reflexiva: F' \
    '2. Irreflexiva: V' \
    '3. Simetrica: V' \
    '4. Anti-simetrica: V' \
    '5. Asimetrica: V' \
    '6. Transitiva: V' \
    'Relaçao de equivalencia: F' \
    'Relaçao de ordem parcial: F' \
    'Fecho reflexivo da relaçao = {(1,1),(2,2)}'

run_success equivalence_multi_digit \
    '1. Reflexiva: V' \
    '2. Irreflexiva: F' \
    '3. Simetrica: V' \
    '4. Anti-simetrica: F' \
    '5. Asimetrica: F' \
    '6. Transitiva: V' \
    'Relaçao de equivalencia: V' \
    'Relaçao de ordem parcial: F'

run_success partial_order \
    '1. Reflexiva: V' \
    '3. Simetrica: F' \
    '4. Anti-simetrica: V' \
    '6. Transitiva: V' \
    'Relaçao de equivalencia: F' \
    'Relaçao de ordem parcial: V'

run_success asymmetric_transitive \
    '1. Reflexiva: F' \
    '2. Irreflexiva: V' \
    '3. Simetrica: F' \
    '4. Anti-simetrica: V' \
    '5. Asimetrica: V' \
    '6. Transitiva: V'

run_success symmetric_non_transitive \
    '2. Irreflexiva: V' \
    '3. Simetrica: V' \
    '4. Anti-simetrica: F' \
    '5. Asimetrica: F' \
    '6. Transitiva: F'

run_success non_transitive_closure \
    '6. Transitiva: F' \
    '(1,3);' \
    'Fecho transitivo da relaçao = {(1,2),(2,3),(1,3)}'

run_success duplicate_pair \
    'Fecho simetrico da relaçao = {(10,20),(20,10)}'

run_failure malformed_header 'quantidade de elementos incorreta'
run_failure duplicate_element 'elementos duplicados'
run_failure unknown_element 'elemento desconhecido'
run_failure malformed_pair 'par ordenado invalido'
run_failure invalid_count 'numero de elementos invalido'
run_failure missing_file 'nao foi possivel abrir o arquivo dados.txt'

printf '\n%d passed, %d failed\n' "${passed}" "${failed}"
((failed == 0))
