[← Back to Documentation Index](../start-here.md)

# Testing in truk

**Language Reference:** [Grammar](grammar.md) · [Builtins](builtins.md) · [Maps](maps.md) · [Defer](defer.md) · [Imports](imports.md) · [Lambdas](lambdas.md) · [Privacy](privacy.md) · [Runtime](runtime.md)

---

The `truk test` command provides a built-in testing framework for discovering and running test functions. Tests follow a simple convention-based approach with no external dependencies required.

## Quick Reference: Test Functions

| Function | Parameters | Description |
|----------|------------|-------------|
| `__truk_test_fail` | `t: *__truk_test_context_s`<br>`msg: *u8` | Manually fail the test with a message |
| `__truk_test_log` | `t: *__truk_test_context_s`<br>`msg: *u8` | Log a message during test execution |
| `__truk_test_assert_i8` | `t: *__truk_test_context_s`<br>`expected: i8`<br>`actual: i8`<br>`msg: *u8` | Assert i8 equality |
| `__truk_test_assert_i16` | `t: *__truk_test_context_s`<br>`expected: i16`<br>`actual: i16`<br>`msg: *u8` | Assert i16 equality |
| `__truk_test_assert_i32` | `t: *__truk_test_context_s`<br>`expected: i32`<br>`actual: i32`<br>`msg: *u8` | Assert i32 equality |
| `__truk_test_assert_i64` | `t: *__truk_test_context_s`<br>`expected: i64`<br>`actual: i64`<br>`msg: *u8` | Assert i64 equality |
| `__truk_test_assert_u8` | `t: *__truk_test_context_s`<br>`expected: u8`<br>`actual: u8`<br>`msg: *u8` | Assert u8 equality |
| `__truk_test_assert_u16` | `t: *__truk_test_context_s`<br>`expected: u16`<br>`actual: u16`<br>`msg: *u8` | Assert u16 equality |
| `__truk_test_assert_u32` | `t: *__truk_test_context_s`<br>`expected: u32`<br>`actual: u32`<br>`msg: *u8` | Assert u32 equality |
| `__truk_test_assert_u64` | `t: *__truk_test_context_s`<br>`expected: u64`<br>`actual: u64`<br>`msg: *u8` | Assert u64 equality |
| `__truk_test_assert_f32` | `t: *__truk_test_context_s`<br>`expected: f32`<br>`actual: f32`<br>`msg: *u8` | Assert f32 equality (exact match) |
| `__truk_test_assert_f64` | `t: *__truk_test_context_s`<br>`expected: f64`<br>`actual: f64`<br>`msg: *u8` | Assert f64 equality (exact match) |
| `__truk_test_assert_bool` | `t: *__truk_test_context_s`<br>`expected: bool`<br>`actual: bool`<br>`msg: *u8` | Assert boolean equality |
| `__truk_test_assert_true` | `t: *__truk_test_context_s`<br>`condition: bool`<br>`msg: *u8` | Assert condition is true |
| `__truk_test_assert_false` | `t: *__truk_test_context_s`<br>`condition: bool`<br>`msg: *u8` | Assert condition is false |
| `__truk_test_assert_ptr_ne_nil` | `t: *__truk_test_context_s`<br>`ptr: *void`<br>`msg: *u8` | Assert pointer is not nil |
| `__truk_test_assert_ptr_eq_nil` | `t: *__truk_test_context_s`<br>`ptr: *void`<br>`msg: *u8` | Assert pointer is nil |
| `__truk_test_assert_bytes_eq` | `t: *__truk_test_context_s`<br>`expected: *u8`<br>`actual: *u8`<br>`len: u64`<br>`msg: *u8` | Assert byte arrays are equal (memcmp) |

---

## Test Function Convention

Test functions must follow this signature:

```truk
fn test_<name>(t: *__truk_test_context_s) : void {
  // test implementation
}
```

**Requirements:**
- Function name MUST start with `test_`
- MUST take exactly one parameter of type `*__truk_test_context_s`
- MUST return `void`

## Basic Usage

### Simple Test

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

fn add(a: i32, b: i32) : i32 {
  return a + b;
}

fn test_addition(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 4, add(2, 2), "2+2 should equal 4");
  __truk_test_assert_i32(t, 10, add(7, 3), "7+3 should equal 10");
}
```

### Running Tests

```bash
# Test a single file
truk test math.truk

# Test all files in a directory (recursive)
truk test tests/
```

### Output

```
Running test_addition...
  PASSED (2 assertions)

1/1 tests passed
```

Exit code is the number of failed tests (0 = all passed).

## Assertion Functions

All assertion functions take a test context pointer, expected/actual values, and an optional message.

### Integer Assertions

All integer types are supported: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`.

```truk
extern fn __truk_test_assert_i8(t: *__truk_test_context_s, 
                                expected: i8, actual: i8, 
                                msg: *u8) : void;
extern fn __truk_test_assert_i16(t: *__truk_test_context_s, 
                                 expected: i16, actual: i16, 
                                 msg: *u8) : void;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;
extern fn __truk_test_assert_i64(t: *__truk_test_context_s, 
                                 expected: i64, actual: i64, 
                                 msg: *u8) : void;
extern fn __truk_test_assert_u8(t: *__truk_test_context_s, 
                                expected: u8, actual: u8, 
                                msg: *u8) : void;
extern fn __truk_test_assert_u16(t: *__truk_test_context_s, 
                                 expected: u16, actual: u16, 
                                 msg: *u8) : void;
extern fn __truk_test_assert_u32(t: *__truk_test_context_s, 
                                 expected: u32, actual: u32, 
                                 msg: *u8) : void;
extern fn __truk_test_assert_u64(t: *__truk_test_context_s, 
                                 expected: u64, actual: u64, 
                                 msg: *u8) : void;
```

**Example:**
```truk
fn test_integers(t: *__truk_test_context_s) : void {
  __truk_test_assert_i8(t, 127, 127, "i8 max positive");
  __truk_test_assert_i16(t, 1000, 1000, "i16 test");
  __truk_test_assert_i32(t, 42, 42, "i32 test");
  __truk_test_assert_i64(t, 9999, 9999, "i64 test");
  __truk_test_assert_u8(t, 255, 255, "u8 max");
  __truk_test_assert_u16(t, 65535, 65535, "u16 max");
  __truk_test_assert_u32(t, 1000000, 1000000, "u32 test");
  __truk_test_assert_u64(t, 5000, 5000, "u64 test");
}
```

### Float Assertions

```truk
extern fn __truk_test_assert_f32(t: *__truk_test_context_s, 
                                 expected: f32, actual: f32, 
                                 msg: *u8) : void;
extern fn __truk_test_assert_f64(t: *__truk_test_context_s, 
                                 expected: f64, actual: f64, 
                                 msg: *u8) : void;
```

**⚠️ Warning:** Float assertions use exact equality comparison (`==`). Due to floating-point precision, this may not work as expected for computed values. Consider comparing with a small epsilon for computed results.

**Example:**
```truk
fn test_floats(t: *__truk_test_context_s) : void {
  __truk_test_assert_f32(t, 3.14, 3.14, "pi approximation");
  __truk_test_assert_f64(t, 2.71828, 2.71828, "e approximation");
}
```

### Boolean Assertions

```truk
extern fn __truk_test_assert_bool(t: *__truk_test_context_s, 
                                  expected: bool, actual: bool, 
                                  msg: *u8) : void;
extern fn __truk_test_assert_true(t: *__truk_test_context_s, 
                                  condition: bool, 
                                  msg: *u8) : void;
extern fn __truk_test_assert_false(t: *__truk_test_context_s, 
                                   condition: bool, 
                                   msg: *u8) : void;
```

**Example:**
```truk
fn test_booleans(t: *__truk_test_context_s) : void {
  __truk_test_assert_true(t, 5 > 3, "5 is greater than 3");
  __truk_test_assert_false(t, 2 > 10, "2 is not greater than 10");
  __truk_test_assert_bool(t, true, true, "bool equality");
}
```

### Pointer Assertions

```truk
extern fn __truk_test_assert_ptr_ne_nil(t: *__truk_test_context_s, 
                                        ptr: *void, 
                                        msg: *u8) : void;
extern fn __truk_test_assert_ptr_eq_nil(t: *__truk_test_context_s, 
                                        ptr: *void, 
                                        msg: *u8) : void;
```

**Example:**
```truk
fn test_pointers(t: *__truk_test_context_s) : void {
  var ptr: *i32 = make(@i32);
  __truk_test_assert_ptr_ne_nil(t, ptr, "allocation should succeed");
  
  *ptr = 42;
  __truk_test_assert_i32(t, 42, *ptr, "pointer value");
  
  delete(ptr);
  
  var nil_ptr: *i32 = nil;
  __truk_test_assert_ptr_eq_nil(t, nil_ptr, "nil pointer");
}
```

### Byte Array Assertions

```truk
extern fn __truk_test_assert_bytes_eq(t: *__truk_test_context_s, 
                                      expected: *u8, 
                                      actual: *u8, 
                                      len: u64, 
                                      msg: *u8) : void;
```

Compares raw byte arrays using `memcmp`. Useful for testing binary data, serialization, or memory contents.

**Example:**
```truk
fn test_byte_arrays(t: *__truk_test_context_s) : void {
  var expected: [5]u8;
  expected[0] = 0x01;
  expected[1] = 0x02;
  expected[2] = 0x03;
  expected[3] = 0x04;
  expected[4] = 0x05;
  
  var actual: [5]u8;
  actual[0] = 0x01;
  actual[1] = 0x02;
  actual[2] = 0x03;
  actual[3] = 0x04;
  actual[4] = 0x05;
  
  __truk_test_assert_bytes_eq(t, &expected[0], &actual[0], 5, "byte arrays match");
}

fn test_memory_copy(t: *__truk_test_context_s) : void {
  var src: [10]u8;
  var dst: [10]u8;
  
  for var i: i32 = 0; i < 10; i = i + 1 {
    src[i] = i as u8;
  }
  
  for var i: i32 = 0; i < 10; i = i + 1 {
    dst[i] = src[i];
  }
  
  __truk_test_assert_bytes_eq(t, &src[0], &dst[0], 10, "memory copied correctly");
}
```

### Manual Failure and Logging

```truk
extern fn __truk_test_fail(t: *__truk_test_context_s, msg: *u8) : void;
extern fn __truk_test_log(t: *__truk_test_context_s, msg: *u8) : void;
```

**Example:**
```truk
fn test_custom(t: *__truk_test_context_s) : void {
  __truk_test_log(t, "Starting custom test");
  
  if some_condition {
    __truk_test_fail(t, "Custom failure message");
  }
}
```

## Multiple Tests

You can define multiple test functions in a single file. They run in source order:

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

fn test_first(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 1, 1, "first test");
}

fn test_second(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 2, 2, "second test");
}

fn test_third(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 3, 3, "third test");
}
```

**Output:**
```
Running test_first...
  PASSED (1 assertions)
Running test_second...
  PASSED (1 assertions)
Running test_third...
  PASSED (1 assertions)

3/3 tests passed
```

## Setup and Teardown

Optional setup and teardown functions run before/after each test:

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_log(t: *__truk_test_context_s, msg: *u8) : void;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

var global_counter: i32 = 0;

fn test_setup(t: *__truk_test_context_s) : void {
  global_counter = 0;
  __truk_test_log(t, "Setup: reset counter");
}

fn test_teardown(t: *__truk_test_context_s) : void {
  __truk_test_log(t, "Teardown: cleanup");
}

fn test_increment(t: *__truk_test_context_s) : void {
  global_counter = global_counter + 1;
  __truk_test_assert_i32(t, 1, global_counter, "counter should be 1");
}

fn test_double_increment(t: *__truk_test_context_s) : void {
  global_counter = global_counter + 2;
  __truk_test_assert_i32(t, 2, global_counter, "counter should be 2");
}
```

**Behavior:**
- `test_setup` runs before each test (if defined)
- `test_teardown` runs after each test (if defined)
- Teardown runs even if the test fails
- Both are optional

**Output:**
```
Running test_increment...
    LOG: Setup: reset counter
    LOG: Teardown: cleanup
  PASSED (1 assertions)
Running test_double_increment...
    LOG: Setup: reset counter
    LOG: Teardown: cleanup
  PASSED (1 assertions)

2/2 tests passed
```

## Directory Testing

Test entire directories recursively:

```bash
truk test tests/
```

**Output:**
```
Testing: tests/math.truk
Running test_addition...
  PASSED (2 assertions)
Running test_subtraction...
  PASSED (1 assertions)

2/2 tests passed

Testing: tests/pointers.truk
Running test_alloc...
  PASSED (3 assertions)

1/1 tests passed

========================================
Tested 2 file(s), 0 failure(s)
```

**Behavior:**
- Finds all `.truk` files recursively
- Tests each file independently
- Files without test functions are skipped silently
- Continues testing even if some files fail
- Exit code is total failures across all files

## Test Failures

When assertions fail, detailed error messages are shown:

```truk
fn test_will_fail(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 100, 4, "this will fail");
}
```

**Output:**
```
Running test_will_fail...
    FAIL: Expected 100, got 4 - this will fail
  FAILED (1/1 assertions)

0/1 tests passed
```

Exit code is `1` (one test failed).

## Testing with Memory

Test memory allocation and deallocation:

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_assert_ptr_ne_nil(t: *__truk_test_context_s, 
                                        ptr: *void, 
                                        msg: *u8) : void;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

fn test_memory(t: *__truk_test_context_s) : void {
  var ptr: *i32 = make(@i32);
  __truk_test_assert_ptr_ne_nil(t, ptr, "allocation succeeded");
  
  *ptr = 42;
  __truk_test_assert_i32(t, 42, *ptr, "value stored correctly");
  
  delete(ptr);
}
```

## Testing with Structs

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

struct Point {
  x: i32,
  y: i32
}

fn test_struct(t: *__truk_test_context_s) : void {
  var p: Point;
  p.x = 10;
  p.y = 20;
  
  __truk_test_assert_i32(t, 10, p.x, "x coordinate");
  __truk_test_assert_i32(t, 20, p.y, "y coordinate");
}
```

## Testing with Arrays

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

fn test_arrays(t: *__truk_test_context_s) : void {
  var arr: [3]i32;
  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;
  
  __truk_test_assert_i32(t, 10, arr[0], "first element");
  __truk_test_assert_i32(t, 20, arr[1], "second element");
  __truk_test_assert_i32(t, 30, arr[2], "third element");
}
```

## Best Practices

### Descriptive Test Names

Use clear, descriptive names that indicate what is being tested:

```truk
fn test_addition_positive_numbers(t: *__truk_test_context_s) : void { }
fn test_division_by_zero_returns_error(t: *__truk_test_context_s) : void { }
fn test_empty_array_returns_nil(t: *__truk_test_context_s) : void { }
```

### Meaningful Messages

Provide clear failure messages:

```truk
__truk_test_assert_i32(t, 42, result, "calculation should return 42");
__truk_test_assert_true(t, ptr != nil, "allocation should not fail");
```

### One Concept Per Test

Keep tests focused on a single behavior:

```truk
fn test_addition(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 4, add(2, 2), "2+2=4");
}

fn test_subtraction(t: *__truk_test_context_s) : void {
  __truk_test_assert_i32(t, 0, sub(2, 2), "2-2=0");
}
```

### Clean Up Resources

Always clean up allocated resources:

```truk
fn test_with_cleanup(t: *__truk_test_context_s) : void {
  var ptr: *i32 = make(@i32);
  *ptr = 42;
  __truk_test_assert_i32(t, 42, *ptr, "value check");
  delete(ptr);
}
```

Or use `test_teardown` for shared cleanup:

```truk
var test_resource: *i32 = nil;

fn test_setup(t: *__truk_test_context_s) : void {
  test_resource = make(@i32);
}

fn test_teardown(t: *__truk_test_context_s) : void {
  if test_resource != nil {
    delete(test_resource);
    test_resource = nil;
  }
}
```

## Exit Codes

The test command returns:
- `0` - All tests passed
- `N` - N tests failed (where N > 0)

This follows Unix conventions and integrates well with CI/CD systems:

```bash
truk test tests/ || echo "Tests failed!"
```

## Limitations

- No test filtering (all tests in a file run)
- No test parallelization
- No test coverage reporting
- No parameterized tests
- No test fixtures beyond setup/teardown

## Common Patterns

### Testing Error Conditions

```truk
fn test_error_handling(t: *__truk_test_context_s) : void {
  var result: i32 = divide(10, 0);
  __truk_test_assert_i32(t, -1, result, "division by zero returns -1");
}
```

### Testing with Global State

```truk
var counter: i32 = 0;

fn test_setup(t: *__truk_test_context_s) : void {
  counter = 0;
}

fn test_increment_counter(t: *__truk_test_context_s) : void {
  counter = counter + 1;
  __truk_test_assert_i32(t, 1, counter, "counter incremented");
}
```

### Testing Complex Conditions

```truk
fn test_complex_logic(t: *__truk_test_context_s) : void {
  var result: bool = (5 > 3) && (10 < 20);
  __truk_test_assert_true(t, result, "compound condition");
  
  var result2: bool = (5 < 3) || (10 < 20);
  __truk_test_assert_true(t, result2, "or condition");
}
```

## Summary

The truk testing framework provides:
- Convention-based test discovery
- Rich assertion library
- Setup/teardown support
- Directory testing
- Clear output and exit codes
- No external dependencies

Write tests by defining functions starting with `test_`, use assertions to verify behavior, and run with `truk test`.
