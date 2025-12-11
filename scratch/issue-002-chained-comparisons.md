# Issue 002: Chained Comparisons Parse Error

## Status
Parser fails when using logical AND with comparisons that share a variable in the middle

## Symptom
```
Error: Parse failed
```

## Failing Example
```truk
fn main() : i32 {
  var a: i32 = 10;
  var b: i32 = 20;
  var c: i32 = 30;
  
  if a < b && b < c {
    return 0;
  }
  
  return 1;
}
```

## Working Workaround
Use parentheses or separate the comparisons differently:
```truk
fn main() : i32 {
  var a: i32 = 10;
  var b: i32 = 20;
  var c: i32 = 30;
  
  var first: bool = a < b;
  var second: bool = b < c;
  if first && second {
    return 0;
  }
  
  return 1;
}
```

Or use a different pattern:
```truk
fn main() : i32 {
  var a: i32 = 10;
  var b: i32 = 20;
  
  if a < b && a > 5 {
    return 0;
  }
  
  return 1;
}
```

## Grammar Reference
According to grammar.md:
```
logical_and    ::= equality ("&&" equality)*
equality       ::= comparison (("==" | "!=") comparison)*
comparison     ::= bitwise_or (("<" | "<=" | ">" | ">=") bitwise_or)*
```

This should allow `a < b && b < c` to parse as:
- `logical_and` with two `equality` expressions
- First: `a < b` (comparison)
- Second: `b < c` (comparison)

## Hypothesis
The parser may have an issue with operator precedence or associativity when the same variable appears on the right side of one comparison and the left side of the next comparison within a logical AND expression.

## Impact
- Cannot write natural chained comparisons like `x < y && y < z`
- Must use intermediate boolean variables or restructure conditions
- Affects readability of range checks and similar logic
