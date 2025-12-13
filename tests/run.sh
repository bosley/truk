#!/bin/bash

set -e

TRUK_BIN="../build/apps/truk/truk"
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

total_tests=0
passed_tests=0
failed_tests=0

for test_category_dir in "${TEST_DIR}"/*/ ; do
    if [ ! -d "${test_category_dir}" ]; then
        continue
    fi
    
    category_name=$(basename "${test_category_dir}")
    
    if [ "${category_name}" = ".tmp" ]; then
        continue
    fi
    
    for test_file in "${test_category_dir}"*.truk ; do
        if [ ! -f "${test_file}" ]; then
            continue
        fi
        
        total_tests=$((total_tests + 1))
        
        test_name=$(basename "${test_file}" .truk)
        
        expected_code=$(echo "${test_name}" | grep -oE '[0-9]+$')
        
        if [ -z "${expected_code}" ]; then
            echo -e "${YELLOW}SKIP${NC} ${category_name}/${test_name} (no exit code in filename)"
            continue
        fi
        
        c_file="${TEMP_DIR}/${test_name}.c"
        bin_file="${TEMP_DIR}/${test_name}.out"
        
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
        "${bin_file}" > /dev/null 2>&1
        actual_code=$?
        set -e
        
        if [ "${actual_code}" -eq "${expected_code}" ]; then
            echo -e "${GREEN}PASS${NC} ${category_name}/${test_name} (exit code: ${actual_code})"
            passed_tests=$((passed_tests + 1))
        else
            echo -e "${RED}FAIL${NC} ${category_name}/${test_name} (expected: ${expected_code}, got: ${actual_code})"
            failed_tests=$((failed_tests + 1))
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
