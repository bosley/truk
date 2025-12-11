# Issue 005: u64 Comparison Not Supported

## Status
**CRITICAL** - Parser fails when comparing u64 variables

## Symptom
```
Error: Parse failed
```

## Failing Example
```truk
fn main() : i32 {
  var a: u64 = 10;
  var b: u64 = 10;
  if a != b {
    return 1;
  }
  return 0;
}
```

## Working Workaround
Cast to i64 or i32 for comparison:
```truk
fn main() : i32 {
  var a: u64 = 10;
  var b: u64 = 10;
  var a_cmp: i64 = a as i64;
  var b_cmp: i64 = b as i64;
  if a_cmp != b_cmp {
    return 1;
  }
  return 0;
}
```

Or avoid comparisons entirely:
```truk
fn main() : i32 {
  var size: u64 = sizeof(@i32);
  return size as i32;
}
```

## Observation
- u64 variables can be declared and assigned
- u64 values can be cast to other types
- u64 values CANNOT be compared with `==`, `!=`, `<`, `>`, `<=`, `>=`
- This affects `len()` and `sizeof()` return values
- No existing tests compare u64 values - they only cast to i32 for return

## Hypothesis
The parser or type checker may not have comparison operators implemented for u64 type. This could be a missing feature in the operator overloading or type resolution system.

## Impact
- **CRITICAL**: Cannot use `len()` or `sizeof()` results in conditional logic
- Cannot validate array lengths
- Cannot check type sizes
- Cannot perform bounds checking with u64 indices
- Severely limits usefulness of u64 type
- Makes defensive programming nearly impossible

## Related
- Issue 004: Cannot compare u64 with literals (now understood as part of this broader issue)
- All u64 comparisons fail, not just with literals
