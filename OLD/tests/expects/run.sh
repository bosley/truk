#!/bin/bash

set -e

TRUK_BIN="../../build/apps/truk/truk"
TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
TEMP_DIR="${TEST_DIR}/.tmp"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

mkdir -p "${TEMP_DIR}"

if [ ! -f "${TRUK_BIN}" ]; then
    echo -e "${RED}Error: truk compiler not found at ${TRUK_BIN}${NC}"
    echo "Please build the project first: cmake --build build"
    exit 1
fi

if [ $# -eq 1 ]; then
    SUBDIR="$1"
    if [ ! -d "${TEST_DIR}/${SUBDIR}" ]; then
        echo -e "${RED}Error: subdirectory '${SUBDIR}' not found in ${TEST_DIR}${NC}"
        exit 1
    fi
    TEST_DIRS=("${TEST_DIR}/${SUBDIR}/")
    echo "Running tests in subdirectory: ${SUBDIR}"
else
    TEST_DIRS=("${TEST_DIR}"/*/)
    echo "Running all tests"
fi

total_tests=0
passed_tests=0
failed_tests=0

for test_category_dir in "${TEST_DIRS[@]}" ; do
    if [ ! -d "${test_category_dir}" ]; then
        continue
    fi
    
    category_name=$(basename "${test_category_dir}")
    
    if [ "${category_name}" = ".tmp" ]; then
        continue
    fi
    
    is_failure_test=false
    if [[ "${category_name}" == *"_failure" ]]; then
        is_failure_test=true
    fi
    
    for test_file in "${test_category_dir}"test_*.truk ; do
        if [ ! -f "${test_file}" ]; then
            continue
        fi
        
        test_name=$(basename "${test_file}" .truk)
        expect_file="${test_category_dir}${test_name}.expect"
        
        if [ ! -f "${expect_file}" ]; then
            echo -e "${YELLOW}SKIP${NC} ${category_name}/${test_name} (no .expect file)"
            continue
        fi
        
        total_tests=$((total_tests + 1))
        
        c_file="${TEMP_DIR}/${test_name}.c"
        bin_file="${TEMP_DIR}/${test_name}.out"
        actual_output="${TEMP_DIR}/${test_name}.actual"
        
        if [ "${is_failure_test}" = true ]; then
            set +e
            "${TRUK_BIN}" toc "${test_file}" -o "${c_file}" > "${actual_output}" 2>&1
            compile_exit_code=$?
            set -e
            
            if [ ${compile_exit_code} -eq 0 ]; then
                echo -e "${RED}FAIL${NC} ${category_name}/${test_name} (expected compilation to fail, but it succeeded)"
                failed_tests=$((failed_tests + 1))
                continue
            fi
            
            normalized_actual="${TEMP_DIR}/${test_name}.normalized_actual"
            normalized_expect="${TEMP_DIR}/${test_name}.normalized_expect"
            
            sed "s|${TEST_DIR}/||g" "${actual_output}" > "${normalized_actual}"
            sed "s|${TEST_DIR}/||g" "${expect_file}" > "${normalized_expect}"
            
            if cmp -s "${normalized_expect}" "${normalized_actual}" ; then
                echo -e "${GREEN}PASS${NC} ${category_name}/${test_name}"
                passed_tests=$((passed_tests + 1))
            else
                echo -e "${RED}FAIL${NC} ${category_name}/${test_name} (error output mismatch)"
                echo "  Expected error output:"
                cat "${normalized_expect}" | sed 's/^/    /'
                echo "  Actual error output:"
                cat "${normalized_actual}" | sed 's/^/    /'
                echo "  Diff:"
                diff -u "${normalized_expect}" "${normalized_actual}" | tail -n +3 | sed 's/^/    /' || true
                failed_tests=$((failed_tests + 1))
            fi
        else
            if ! "${TRUK_BIN}" toc "${test_file}" -o "${c_file}" > /dev/null 2>&1 ; then
                echo -e "${RED}FAIL${NC} ${category_name}/${test_name} (truk→C compilation failed)"
                failed_tests=$((failed_tests + 1))
                continue
            fi
            
            if ! "${TRUK_BIN}" tcc "${c_file}" -o "${bin_file}" > /dev/null 2>&1 ; then
                echo -e "${RED}FAIL${NC} ${category_name}/${test_name} (TCC C→binary compilation failed)"
                failed_tests=$((failed_tests + 1))
                continue
            fi
            
            set +e
            "${bin_file}" > "${actual_output}" 2>&1
            exit_code=$?
            set -e
            
            if cmp -s "${expect_file}" "${actual_output}" ; then
                echo -e "${GREEN}PASS${NC} ${category_name}/${test_name}"
                passed_tests=$((passed_tests + 1))
            else
                echo -e "${RED}FAIL${NC} ${category_name}/${test_name} (output mismatch)"
                echo "  Expected output:"
                cat "${expect_file}" | sed 's/^/    /'
                echo "  Actual output:"
                cat "${actual_output}" | sed 's/^/    /'
                echo "  Diff:"
                diff -u "${expect_file}" "${actual_output}" | tail -n +3 | sed 's/^/    /' || true
                failed_tests=$((failed_tests + 1))
            fi
        fi
    done
done

echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Total:  ${total_tests}"
echo -e "Passed: ${GREEN}${passed_tests}${NC}"
echo -e "Failed: ${RED}${failed_tests}${NC}"
echo "=========================================="

rm -rf "${TEMP_DIR}"

if [ "${failed_tests}" -gt 0 ]; then
    exit 1
fi

exit 0
