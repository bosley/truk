#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TRUK_BIN="${SCRIPT_DIR}/../build/apps/truk/truk"

if [ ! -f "$TRUK_BIN" ]; then
    echo "Error: truk binary not found at $TRUK_BIN"
    echo "Please build the project first: cd ../build && make"
    exit 1
fi

CC="${CC:-cc}"

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
    c_file="$TEMP_DIR/${filename%.truk}.c"
    exe_file="$TEMP_DIR/${filename%.truk}"
    
    echo -n "Testing $filename ... "
    
    if ! "$TRUK_BIN" "$truk_file" -o "$c_file" > /dev/null 2>&1; then
        echo "✗ FAIL (truk compilation)"
        FAILED=$((FAILED + 1))
        echo "  Error compiling $truk_file with truk"
        "$TRUK_BIN" "$truk_file" -o "$c_file" 2>&1 | sed 's/^/  /'
        exit 1
    fi
    
    if ! "$CC" "$c_file" -o "$exe_file" 2>&1 | tee "$TEMP_DIR/${filename%.truk}.cc_err" | sed 's/^/  /'; then
        echo "✗ FAIL (C compilation)"
        FAILED=$((FAILED + 1))
        echo "  Error compiling generated C code"
        echo "  Generated C file: $c_file"
        cat "$TEMP_DIR/${filename%.truk}.cc_err" | sed 's/^/  /'
        exit 1
    fi
    
    if ! "$exe_file" > /dev/null 2>&1; then
        exit_code=$?
        echo "✗ FAIL (execution: exit code $exit_code)"
        FAILED=$((FAILED + 1))
        echo "  Executable returned non-zero exit code: $exit_code"
        exit 1
    fi
    
    echo "✓ PASS"
    PASSED=$((PASSED + 1))
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
