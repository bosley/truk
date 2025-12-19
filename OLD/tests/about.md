# Testing

We have three categories of tests, each with their own `run.sh` script:

## 1. `return_code_assertions/`

Bootstrap tests using return-code validation (no internal assertions). Around 700 tests at the time of writing.

**Naming Convention:** `*_N.truk` where `N` is a `u8` sized expected return value (truk `i32` truncates to `u8` for C returns).

**Usage:**
```bash
cd return_code_assertions
./run.sh              # Run all tests
./run.sh <subdir>     # Run tests in specific subdirectory
```

## 2. `meta_test_testing_fw/`

Tests written in truk utilizing the internal testing framework (test functions, assertions, setup/teardown).

**Naming Convention:** `test_*.truk` files containing test functions (`fn test_*`).

**Behavior:** Runs all `test_*.truk` files and asserts exit code is `0`. The `test_*` prefix filtering allows auxiliary files that aren't tests themselves.

**Usage:**
```bash
cd meta_test_testing_fw
./run.sh              # Run all tests
./run.sh <subdir>     # Run tests in specific subdirectory
```

## 3. `expects/`

Tests that validate exact output matching byte-by-byte.

**Naming Convention:** Each `test_*.truk` file has a corresponding `test_*.expect` file containing the expected output.

**Behavior:** Compiles and runs each test, then uses `cmp` for byte-by-byte comparison of actual output against expected output. Useful for testing printf output, error messages, and stdout/stderr behavior.

**Failure Tests:** Subdirectories with names ending in `_failure` (e.g., `basic_failure/`) are treated as compilation failure tests. These tests are expected to fail during the `truk toc` compilation phase, and the `.expect` file should contain the expected error output.

**Usage:**
```bash
cd expects
./run.sh              # Run all tests
./run.sh <subdir>     # Run tests in specific subdirectory
```

## Running All Tests

From the `tests/` directory:
```bash
./run.sh                        # Run all test categories
./run.sh return_code_assertions # Run specific category
./run.sh meta_test_testing_fw   # Run specific category
./run.sh expects                # Run specific category
```
