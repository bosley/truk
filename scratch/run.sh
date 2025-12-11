#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TRUK_BIN="${SCRIPT_DIR}/../build/apps/truk/truk"

if [ ! -f "$TRUK_BIN" ]; then
    echo "Error: truk binary not found at $TRUK_BIN"
    echo "Please build the project first: cd ../build && make"
    exit 1
fi

TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

TOTAL=0
PASSED=0
FAILED=0

echo "Running Truk regression tests..."
echo "================================"
echo ""

for truk_file in "$SCRIPT_DIR"/test_*.truk; do
    if [ ! -f "$truk_file" ]; then
        continue
    fi
    
    TOTAL=$((TOTAL + 1))
    filename=$(basename "$truk_file")
    output_file="$TEMP_DIR/${filename%.truk}.c"
    
    echo -n "Testing $filename ... "
    
    if "$TRUK_BIN" "$truk_file" -o "$output_file" > /dev/null 2>&1; then
        echo "✓ PASS"
        PASSED=$((PASSED + 1))
    else
        echo "✗ FAIL"
        FAILED=$((FAILED + 1))
        echo "  Error compiling $truk_file"
        "$TRUK_BIN" "$truk_file" -o "$output_file" 2>&1 | sed 's/^/  /'
        exit 1
    fi
done

echo ""
echo "================================"
echo "Results: $PASSED/$TOTAL passed"

if [ $FAILED -gt 0 ]; then
    echo "FAILED: $FAILED test(s) failed"
    exit 1
else
    echo "SUCCESS: All tests passed!"
    exit 0
fi
