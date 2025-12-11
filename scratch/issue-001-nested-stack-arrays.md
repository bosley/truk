# Issue 001: Nested Stack Arrays Parse Error

## Status
Parser fails to parse nested stack-allocated arrays with syntax `[N][M]T`

## Symptom
```
Error: Parse failed
```

## Failing Example
```truk
fn main() : i32 {
  var nested_arr: [3][4]i32;
  nested_arr[0][0] = 1;
  return 0;
}
```

## Working Workaround
Use dynamic allocation for outer array:
```truk
fn main() : i32 {
  var count: u64 = 3;
  var arr: [][4]i32 = alloc_array(@[4]i32, count);
  arr[0][0] = 1;
  free_array(arr);
  return 0;
}
```

## Grammar Reference
According to grammar.md:
```
array_type ::= "[" expression? "]" type
```

This should theoretically allow `[3][4]i32` to parse as:
- `[3]` followed by type `[4]i32`
- Where `[4]i32` is `[4]` followed by type `i32`

## Hypothesis
The parser may not be correctly handling recursive array type parsing for sized arrays, or there may be an intentional restriction against stack-allocated multi-dimensional arrays.

## Impact
- Cannot create stack-allocated multi-dimensional arrays
- Must use heap allocation for any multi-dimensional array structures
- Affects test coverage for nested array operations
