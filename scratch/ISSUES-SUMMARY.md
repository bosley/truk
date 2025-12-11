# Truk Language Issues Summary

This document summarizes all discovered issues during comprehensive stress testing.

## Critical Issues

### Issue 005: u64 Comparison Not Supported
**Status**: CRITICAL - Blocks most defensive programming

**Problem**: Cannot compare u64 variables with any comparison operator

**Impact**: 
- Cannot use `len()` or `sizeof()` in conditionals
- Cannot validate array bounds
- Cannot perform size checks

**Workaround**: Cast to i64/i32 before comparison

**File**: `issue-005-u64-comparison-not-supported.md`

---

## Major Issues

### Issue 003: Sized Arrays Not Supported
**Status**: MAJOR - Feature appears unimplemented

**Problem**: Parser fails on sized array declarations `var arr: [N]T;`

**Impact**:
- Cannot create stack-allocated arrays
- All arrays must be heap-allocated
- Increases memory management burden

**Workaround**: Use `alloc_array` for all arrays

**File**: `issue-003-sized-arrays-not-supported.md`

---

## Moderate Issues

### Issue 002: Chained Comparisons Parse Error
**Status**: MODERATE - Affects code readability

**Problem**: Parser fails on patterns like `a < b && b < c`

**Impact**:
- Cannot write natural range checks
- Must use intermediate boolean variables

**Workaround**: Use separate boolean variables or restructure conditions

**File**: `issue-002-chained-comparisons.md`

### Issue 001: Nested Stack Arrays Not Supported
**Status**: MODERATE - Multi-dimensional array limitation

**Problem**: Parser fails on `[N][M]T` syntax

**Impact**:
- Cannot create stack-allocated multi-dimensional arrays
- Must use `[][M]T` with heap allocation for outer dimension

**Workaround**: Use `alloc_array(@[M]T, count)` pattern

**File**: `issue-001-nested-stack-arrays.md`

---

## Minor Issues

### Issue 004: u64 Literal Comparison Type Mismatch
**Status**: MINOR - Subsumed by Issue 005

**Problem**: Type checker fails comparing u64 with integer literals

**Note**: This is actually part of the broader Issue 005 where all u64 comparisons fail

**File**: `issue-004-u64-literal-comparison.md`

---

## Testing Strategy

Given these limitations, working tests must:

1. **Avoid u64 comparisons entirely**
   - Use `len()` and `sizeof()` only for assignment or casting
   - Never use in conditional expressions

2. **Use only dynamic arrays**
   - Always use `[]T` with `alloc_array`
   - Never declare `[N]T` sized arrays

3. **Avoid chained comparisons**
   - Use separate boolean variables
   - Or restructure to avoid pattern `x < y && y < z`

4. **For multi-dimensional arrays**
   - Use `[][N]T` pattern only
   - Never `[N][M]T`

---

## Confirmed Working Features

- Basic arithmetic on all integer types (i8, i16, i32, i64, u8, u16, u32, u64)
- Pointer allocation and dereferencing
- Dynamic array allocation with `alloc_array` / `free_array`
- Struct definition and field access
- Function calls and returns
- if/else control flow
- while loops
- for loops
- Type casting with `as`
- Comparison operators on non-u64 types
- Logical operators `&&`, `||`, `!`
- Bitwise operators
