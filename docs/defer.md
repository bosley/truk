# Defer Statement

## Overview

The `defer` statement schedules code to execute at the end of the current scope. Deferred code executes in Last-In-First-Out (LIFO) order, meaning the last defer registered is the first to execute.

Defer is useful for cleanup operations, ensuring resources are released, or performing final computations before a scope exits.

## Syntax

Defer has two forms:

**Expression form:**
```truk
defer expression;
```

**Block form:**
```truk
defer {
    statement1;
    statement2;
}
```

From the grammar:
```
defer_stmt ::= "defer" (expression ";" | block)
```

## Execution Model

### Scope-Based Execution

Defers execute at the end of their containing scope. A scope can be:
- A function body
- An anonymous block `{ }`
- A loop body
- An if/else block

When a scope exits (normally or via `return`), all defers registered in that scope execute before control leaves the scope.

### LIFO Ordering

Multiple defers in the same scope execute in reverse order of registration:

```truk
fn main() : i32 {
  var x: i32 = 1;
  defer x = x + 10;
  defer x = x * 2;
  return x;
}
```

Execution order:
1. `x = 1`
2. Register defer: `x = x + 10`
3. Register defer: `x = x * 2`
4. Execute defer (last registered): `x = x * 2` → `x = 2`
5. Execute defer (first registered): `x = x + 10` → `x = 12`
6. Return `12`

This proves LIFO ordering. If defers executed in forward order, the result would be `(1 + 10) * 2 = 22`.

## Examples

### Simple Defer

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 3;
  return x;
}
```

Returns `3`. The defer executes before the return statement, modifying `x` from `0` to `3`.

### Multiple Defers

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 1;
  defer x = x + 2;
  defer x = x + 4;
  return x;
}
```

Returns `7`. Defers execute in reverse order:
- `x = x + 4` → `x = 4`
- `x = x + 2` → `x = 6`
- `x = x + 1` → `x = 7`

### Defer with Block

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer {
    x = x + 5;
    x = x + 5;
  }
  return x;
}
```

Returns `10`. The defer block executes both statements before the return.

## Interaction with Scopes

### Anonymous Blocks

Each anonymous block has its own defer scope:

```truk
fn main() : i32 {
  var x: i32 = 0;
  {
    defer x = x + 3;
    x = x + 2;
  }
  defer x = x + 3;
  return x;
}
```

Execution order:
1. `x = 0`
2. Enter anonymous block
3. `x = x + 2` → `x = 2`
4. Exit block: defer executes `x = x + 3` → `x = 5`
5. Register outer defer: `x = x + 3`
6. Exit function: defer executes `x = x + 3` → `x = 8`
7. Return `8`

### Nested Scopes

Defers in nested scopes execute when their respective scope exits:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 10;
  {
    defer x = x + 5;
    x = x + 1;
  }
  defer x = x + 9;
  return x;
}
```

Execution order:
1. `x = 0`
2. Register outer defer: `x = x + 10`
3. Enter anonymous block
4. Register inner defer: `x = x + 5`
5. `x = x + 1` → `x = 1`
6. Exit block: inner defer executes `x = x + 5` → `x = 6`
7. Register outer defer: `x = x + 9`
8. Exit function: defers execute in reverse
   - `x = x + 9` → `x = 15`
   - `x = x + 10` → `x = 25`
9. Return `25`

## Interaction with Control Flow

### Return Statements

Defers execute at the end of their containing scope, right before the scope exits:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 3;
  return x;
}
```

The defer modifies `x` before the return, so the function returns `3`, not `0`.

### Early Returns

When a `return` statement executes, ALL defers from ALL enclosing scopes execute before the function returns:

```truk
fn example() : i32 {
  var x: i32 = 0;
  defer x = x + 5;
  
  if true {
    return x;
  }
  
  return x + 100;
}
```

Returns `5`. Even though the return is inside the if block, the defer from the function scope executes before returning.

This works for deeply nested returns as well:

```truk
fn nested_example() : i32 {
  var x: i32 = 0;
  defer x = x + 100;
  {
    defer x = x + 10;
    if true {
      defer x = x + 1;
      return x;
    }
  }
  return x;
}
```

Returns `111`. All three defers execute in LIFO order: `x + 1` (innermost), then `x + 10`, then `x + 100`.

### Loops

In loops, defers execute at the end of each iteration (since the loop body is a scope):

```truk
fn main() : i32 {
  var x: i32 = 0;
  var i: i32 = 0;
  while i < 5 {
    defer x = x + 1;
    i = i + 1;
  }
  return x;
}
```

Returns `5`. The defer executes once per iteration, at the end of each loop iteration.

## Restrictions

Deferred code cannot contain control flow statements:
- No `return` statements
- No `break` statements
- No `continue` statements

These restrictions ensure defers are used for cleanup and finalization, not for altering control flow.

## Best Practices

### Use Defer for Cleanup

Defer is ideal for resource cleanup. All defers execute before any return, regardless of where the return occurs:

```truk
fn process_file(filename: *i8) : i32 {
  var file: *File = open_file(filename);
  defer close_file(file);
  
  if !validate_file(file) {
    return -1;
  }
  
  return process_data(file);
}
```

The file will be closed whether the function returns early (`-1`) or normally. The defer executes before both return paths.

### Keep Defers Simple

Defers should be simple, focused operations. Complex logic in defers can make code harder to understand.

### Order Matters

Remember that defers execute in LIFO order. If order matters for your cleanup operations, register them in reverse order of desired execution.

### Scope Awareness

Be aware of which scope your defer is in. Defers in inner scopes execute before defers in outer scopes.

## Summary

- Defer schedules code to execute at scope exit
- Defers execute in LIFO (Last-In-First-Out) order
- Each scope has its own defer stack
- Defers execute before return statements
- Defers cannot contain control flow statements
- Use defer for cleanup and finalization operations
