# Issue 004: Cannot Compare u64 with Integer Literals

## Status
Type checker fails when comparing `u64` values with integer literals

## Symptom
```
Error: Type check failed
  [line] Comparison operation type mismatch
```

## Failing Example
```truk
fn main() : i32 {
  var size: u64 = sizeof(@i32);
  if size != 4 {
    return 1;
  }
  return 0;
}
```

## Working Workaround
Cast the literal to `u64` or use separate variable:
```truk
fn main() : i32 {
  var size: u64 = sizeof(@i32);
  var expected: u64 = 4;
  if size != expected {
    return 1;
  }
  return 0;
}
```

Or just return the value for external checking:
```truk
fn main() : i32 {
  var size: u64 = sizeof(@i32);
  return size as i32;
}
```

## Observation
- `sizeof` returns `u64`
- `len` returns `u64`
- Integer literals appear to default to `i32`
- Type checker requires exact type match for comparisons
- No implicit conversion from integer literal to `u64`

## Hypothesis
The type checker does not perform implicit type coercion for integer literals in comparison operations. Literals may be typed as `i32` by default and require explicit casting or variable declaration with the correct type.

## Impact
- Cannot directly compare results of `sizeof` or `len` with literals
- Must use intermediate variables with explicit `u64` type
- Reduces code readability for simple assertions
- Makes testing more verbose

## Related
This likely affects all operations mixing `u64` with other integer types, not just comparisons.
