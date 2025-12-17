#!/bin/bash

set -e

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TEST_CATEGORIES=(
    "return_code_assertions"
    "meta_test_testing_fw"
    "expects"
)

if [ $# -eq 1 ]; then
    CATEGORY="$1"
    if [ ! -d "${TEST_DIR}/${CATEGORY}" ]; then
        echo -e "${RED}Error: test category '${CATEGORY}' not found${NC}"
        echo "Available categories:"
        for cat in "${TEST_CATEGORIES[@]}"; do
            echo "  - ${cat}"
        done
        exit 1
    fi
    CATEGORIES_TO_RUN=("${CATEGORY}")
    echo -e "${BLUE}Running test category: ${CATEGORY}${NC}"
    echo ""
else
    CATEGORIES_TO_RUN=("${TEST_CATEGORIES[@]}")
    echo -e "${BLUE}Running all test categories${NC}"
    echo ""
fi

total_categories=0
passed_categories=0
failed_categories=0

for category in "${CATEGORIES_TO_RUN[@]}"; do
    category_dir="${TEST_DIR}/${category}"
    run_script="${category_dir}/run.sh"
    
    if [ ! -f "${run_script}" ]; then
        echo -e "${YELLOW}SKIP${NC} ${category} (no run.sh found)"
        continue
    fi
    
    total_categories=$((total_categories + 1))
    
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}Category: ${category}${NC}"
    echo -e "${BLUE}========================================${NC}"
    
    set +e
    (cd "${category_dir}" && ./run.sh)
    exit_code=$?
    set -e
    
    echo ""
    
    if [ "${exit_code}" -eq 0 ]; then
        echo -e "${GREEN}✓ ${category} PASSED${NC}"
        passed_categories=$((passed_categories + 1))
    else
        echo -e "${RED}✗ ${category} FAILED${NC}"
        failed_categories=$((failed_categories + 1))
        echo ""
        echo -e "${RED}Stopping test execution due to failure in ${category}${NC}"
        exit 1
    fi
    
    echo ""
done

echo -e "${BLUE}==========================================${NC}"
echo -e "${BLUE}Overall Test Summary${NC}"
echo -e "${BLUE}==========================================${NC}"
echo "Total categories:  ${total_categories}"
echo -e "Passed:            ${GREEN}${passed_categories}${NC}"
echo -e "Failed:            ${RED}${failed_categories}${NC}"
echo -e "${BLUE}==========================================${NC}"

if [ "${failed_categories}" -gt 0 ]; then
    exit 1
fi

exit 0
