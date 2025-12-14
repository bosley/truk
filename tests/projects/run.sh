#!/bin/bash

set -e

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
TRUK_BIN="$(cd "${TEST_DIR}/../.." && pwd)/build/apps/truk/truk"
TEMP_DIR="${TEST_DIR}/.tmp"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

total_tests=0
passed_tests=0
failed_tests=0

mkdir -p "${TEMP_DIR}"

if [ ! -f "${TRUK_BIN}" ]; then
    echo -e "${RED}Error: truk compiler not found at ${TRUK_BIN}${NC}"
    echo "Please build the project first: cmake --build build"
    exit 1
fi

run_test() {
    local test_name="$1"
    local command="$2"
    local expected_exit="$3"
    local working_dir="${4:-.}"
    
    total_tests=$((total_tests + 1))
    
    set +e
    (cd "${working_dir}" && eval "${command}") > /dev/null 2>&1
    actual_exit=$?
    set -e
    
    if [ "${actual_exit}" -eq "${expected_exit}" ]; then
        echo -e "${GREEN}PASS${NC} ${test_name} (exit code: ${actual_exit})"
        passed_tests=$((passed_tests + 1))
        return 0
    else
        echo -e "${RED}FAIL${NC} ${test_name} (expected: ${expected_exit}, got: ${actual_exit})"
        failed_tests=$((failed_tests + 1))
        return 1
    fi
}

test_manual_cimport() {
    echo -e "${BLUE}Testing Manual C Interop Project...${NC}"
    
    local project_dir="${TEST_DIR}/manual_cimport"
    local temp_project="${TEMP_DIR}/manual_cimport"
    
    cp -r "${project_dir}" "${temp_project}"
    
    run_test "manual_cimport: truk build" "${TRUK_BIN} build" 0 "${temp_project}"
    
    run_test "manual_cimport: truk test fileutil" "${TRUK_BIN} test fileutil" 0 "${temp_project}"
    
    run_test "manual_cimport: truk test" "${TRUK_BIN} test" 0 "${temp_project}"
    
    run_test "manual_cimport: ./build/main" "./build/main" 42 "${temp_project}"
    
    run_test "manual_cimport: truk clean" "${TRUK_BIN} clean" 0 "${temp_project}"
    
    echo ""
}

test_generated_libs() {
    echo -e "${BLUE}Testing Generated Project with Dependencies...${NC}"
    
    local gen_dir="${TEMP_DIR}/generated"
    mkdir -p "${gen_dir}"
    
    run_test "generated_libs: truk new testproject" "${TRUK_BIN} new testproject" 0 "${gen_dir}"
    
    local project_dir="${gen_dir}/testproject"
    
    if [ ! -d "${project_dir}" ] || [ ! -f "${project_dir}/truk.kit" ]; then
        echo -e "${RED}FAIL${NC} generated_libs: project structure not created"
        failed_tests=$((failed_tests + 1))
        total_tests=$((total_tests + 1))
        return 1
    else
        echo -e "${GREEN}PASS${NC} generated_libs: project structure created"
        passed_tests=$((passed_tests + 1))
        total_tests=$((total_tests + 1))
    fi
    
    mkdir -p "${project_dir}/libs/math"
    mkdir -p "${project_dir}/libs/calc"
    
    cat > "${project_dir}/libs/math/lib.truk" << 'EOF'
extern fn add(a: i32, b: i32): i32;
extern fn multiply(a: i32, b: i32): i32;
EOF
    
    cat > "${project_dir}/libs/math/lib_impl.truk" << 'EOF'
import "libs/math/lib.truk";

fn add(a: i32, b: i32): i32 {
    return a + b;
}

fn multiply(a: i32, b: i32): i32 {
    return a * b;
}
EOF
    
    cat > "${project_dir}/libs/math/test.truk" << 'EOF'
import "libs/math/lib_impl.truk";

fn main(): i32 {
    if (add(2, 3) != 5) {
        return 1;
    }
    if (multiply(4, 5) != 20) {
        return 1;
    }
    return 0;
}
EOF
    
    cat > "${project_dir}/libs/calc/lib.truk" << 'EOF'
extern fn square(x: i32): i32;
EOF
    
    cat > "${project_dir}/libs/calc/lib_impl.truk" << 'EOF'
import "libs/math/lib.truk";
import "libs/calc/lib.truk";

fn square(x: i32): i32 {
    return multiply(x, x);
}
EOF
    
    cat > "${project_dir}/libs/calc/test.truk" << 'EOF'
import "libs/math/lib_impl.truk";
import "libs/calc/lib_impl.truk";

fn main(): i32 {
    if (square(5) != 25) {
        return 1;
    }
    if (square(10) != 100) {
        return 1;
    }
    return 0;
}
EOF
    
    cat > "${project_dir}/apps/main/main.truk" << 'EOF'
cimport <stdio.h>;

extern fn printf(fmt: *i8, ...args): i32;

import "../../libs/math/lib.truk";
import "../../libs/calc/lib.truk";

fn main(): i32 {
    var sum: i32 = add(10, 20);
    var prod: i32 = multiply(5, 6);
    var sq: i32 = square(7);
    
    printf("sum=%d prod=%d square=%d\n", sum, prod, sq);
    
    return 0;
}
EOF
    
    cat > "${project_dir}/truk.kit" << 'EOF'
project testproject

library math {
    source = libs/math/lib_impl.truk
    test = libs/math/test.truk
}

library calc {
    source = libs/calc/lib_impl.truk
    test = libs/calc/test.truk
    depends = math
}

application main {
    source = apps/main/main.truk
    output = build/main
    libraries = math calc
}
EOF
    
    run_test "generated_libs: truk build" "${TRUK_BIN} build" 0 "${project_dir}"
    
    run_test "generated_libs: truk test math" "${TRUK_BIN} test math" 0 "${project_dir}"
    
    run_test "generated_libs: truk test calc" "${TRUK_BIN} test calc" 0 "${project_dir}"
    
    run_test "generated_libs: truk test" "${TRUK_BIN} test" 0 "${project_dir}"
    
    run_test "generated_libs: ./build/main" "./build/main" 0 "${project_dir}"
    
    run_test "generated_libs: truk clean" "${TRUK_BIN} clean" 0 "${project_dir}"
    
    echo ""
}

echo "Running all test suites..."
echo ""

echo "=== Running projects tests ==="
test_manual_cimport

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
