#!/bin/bash

set -e

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

if [ $# -eq 0 ]; then
    echo -e "${BLUE}Running all test suites...${NC}"
    echo ""
    
    total_failed=0
    
    if [ -f "${TEST_DIR}/projects/run.sh" ]; then
        echo -e "${BLUE}=== Running projects tests ===${NC}"
        if "${TEST_DIR}/projects/run.sh"; then
            echo -e "${GREEN}Projects tests passed${NC}"
        else
            echo -e "${RED}Projects tests failed${NC}"
            total_failed=$((total_failed + 1))
        fi
        echo ""
    fi
    
    if [ -f "${TEST_DIR}/targeted/run.sh" ]; then
        echo -e "${BLUE}=== Running targeted tests ===${NC}"
        if "${TEST_DIR}/targeted/run.sh"; then
            echo -e "${GREEN}Targeted tests passed${NC}"
        else
            echo -e "${RED}Targeted tests failed${NC}"
            total_failed=$((total_failed + 1))
        fi
        echo ""
    fi
    
    echo "=========================================="
    echo "Overall Test Summary"
    echo "=========================================="
    if [ "${total_failed}" -eq 0 ]; then
        echo -e "${GREEN}All test suites passed${NC}"
        exit 0
    else
        echo -e "${RED}${total_failed} test suite(s) failed${NC}"
        exit 1
    fi
else
    test_subdir="$1"
    test_script="${TEST_DIR}/${test_subdir}/run.sh"
    
    if [ ! -f "${test_script}" ]; then
        echo -e "${RED}Error: Test script not found: ${test_script}${NC}"
        echo "Available test suites:"
        for dir in "${TEST_DIR}"/*/; do
            if [ -f "${dir}run.sh" ]; then
                echo "  - $(basename "${dir}")"
            fi
        done
        exit 1
    fi
    
    echo -e "${BLUE}Running ${test_subdir} tests...${NC}"
    exec "${test_script}"
fi
