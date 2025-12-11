# Issue 003: Sized Arrays Not Supported

## Status
Parser fails on sized array declarations `var name: [N]T;`

## Symptom
```
Error: Parse failed
```

## Failing Example
```truk
fn main() : i32 {
  var arr: [5]i8;
  arr[0] = 10;
  return 0;
}
```

## Working Workaround
Use dynamic arrays (slices) with `alloc_array`:
```truk
fn main() : i32 {
  var count: u64 = 5;
  var arr: []i8 = alloc_array(@i8, count);
  arr[0] = 10;
  free_array(arr);
  return 0;
}
```

## Grammar Reference
According to grammar.md:
```
array_type ::= "[" expression? "]" type
```

This should allow:
- `[]T` - unsized array (slice)
- `[N]T` - sized array with compile-time size

## Observation
- No sized arrays appear in any of the existing test files in `/tests`
- All array tests use unsized arrays `[]T` with `alloc_array`
- Sized arrays are mentioned in builtins.md documentation but not actually used

## Hypothesis
Sized array support may not be fully implemented in the parser or may be intentionally disabled. The grammar suggests they should work, but the implementation may not support them yet.

## Impact
- Cannot create stack-allocated arrays with fixed size
- All arrays must be heap-allocated with `alloc_array` and manually freed
- Increases memory management burden for simple fixed-size arrays
- Cannot use array literals for initialization (e.g., `var arr: [3]i32 = [1, 2, 3];`)

## Related
- This also affects struct fields - cannot have sized array fields in structs
- Multi-dimensional arrays would need to use `[][N]T` pattern (dynamic outer, sized inner)
