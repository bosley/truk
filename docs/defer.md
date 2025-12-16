# Defer Statement

## Overview

The `defer` statement schedules code to execute when the current **function** exits. Deferred code executes in Last-In-First-Out (LIFO) order, meaning the last defer registered is the first to execute.

Defer is useful for cleanup operations, ensuring resources are released, or performing final computations before a function returns.

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

### Function-Scope Execution

**Defers execute ONLY when exiting the function**, not at block or loop scope exits. All defers registered anywhere in the function will execute when the function returns, regardless of where they were declared.

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 1;
  {
    defer x = x + 2;  // Still executes at function exit
    x = x + 10;
  }
  return x;  // Defers execute here: x=10, then x=12, then x=13
}
```

Returns `13`. Both defers execute at function exit in LIFO order, even though one was declared in a nested block.

### LIFO Ordering

Multiple defers in the same function execute in reverse order of registration:

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

## Interaction with Control Flow

### Return Statements

Defers execute immediately before any return statement:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 3;
  return x;
}
```

The defer modifies `x` before the return, so the function returns `3`, not `0`.

### Early Returns

When a `return` statement executes anywhere in the function, ALL defers execute before the function returns:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 5;
  
  if true {
    return x;
  }
  
  return x + 100;
}
```

Returns `5`. Even though the return is inside the if block, all function-level defers execute before returning.

### Multiple Early Returns

All defers execute regardless of which return path is taken:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 100;
  defer x = x + 10;
  defer x = x + 1;
  
  if condition1 {
    return x;  // Defers execute: x=1, x=11, x=111
  }
  
  if condition2 {
    return x;  // Defers execute: x=1, x=11, x=111
  }
  
  return x;    // Defers execute: x=1, x=11, x=111
}
```

All three defers execute in LIFO order before ANY return, ensuring consistent cleanup.

### Loops

Defers in loops are registered during code generation (compile time), not during execution (runtime). A defer statement in a loop body is only registered ONCE:

```truk
fn main() : i32 {
  var x: i32 = 0;
  var i: i32 = 0;
  while i < 5 {
    defer x = x + 1;  // Registered ONCE at compile time
    i = i + 1;
  }
  return x;  // One defer executes: x becomes 1
}
```

Returns `1`. The defer statement is visited once during code generation, so only one defer is registered, which executes once at function exit.

**Important**: Defer registration happens at compile time (when the code is generated), not at runtime (when the loop executes). The defer statement itself is not executed multiple times just because it's in a loop.

### Break and Continue

Break and continue statements do NOT trigger defer execution. Defers only execute at function exit:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 100;
  var i: i32 = 0;
  while i < 10 {
    x = x + 1;
    i = i + 1;
    if i == 5 {
      break;  // Defer does NOT execute here
    }
  }
  return x;  // Defer executes here: x=5, then x=105
}
```

Returns `105`. The defer executes at function exit, not when the break occurs.

## Restrictions

Deferred code cannot contain control flow statements:
- No `return` statements
- No `break` statements  
- No `continue` statements

These restrictions ensure defers are used for cleanup and finalization, not for altering control flow.

Attempting to use control flow in a defer will result in a compile-time error:

```truk
fn main() : i32 {
  defer {
    return 5;  // ERROR: Defer cannot contain return statements
  }
  return 0;
}
```

## Best Practices

### Use Defer for Cleanup

Defer is ideal for resource cleanup. All defers execute before any return, regardless of where the return occurs:

```truk
fn process_file(filename: *i8) : i32 {
  var file: *File = open_file(filename);
  defer close_file(file);
  
  if !validate_file(file) {
    return -1;  // File closed before return
  }
  
  if !process_data(file) {
    return -2;  // File closed before return
  }
  
  return 0;  // File closed before return
}
```

The file will be closed before ANY return path, ensuring no resource leaks.

### Keep Defers Simple

Defers should be simple, focused operations. Complex logic in defers can make code harder to understand.

### Order Matters

Remember that defers execute in LIFO order. If order matters for your cleanup operations, register them in reverse order of desired execution:

```truk
fn main() : i32 {
  defer close_connection();  // Executes LAST
  defer flush_buffer();      // Executes SECOND
  defer stop_timer();        // Executes FIRST
  
  // ... function body ...
  
  return 0;
}
```

Execution order: `stop_timer()`, then `flush_buffer()`, then `close_connection()`.

### Defer Registration is Dynamic

Defers are registered during execution, so conditional registration is possible:

```truk
fn main() : i32 {
  var x: i32 = 0;
  
  if condition {
    defer x = x + 10;  // Only registered if condition is true
  }
  
  defer x = x + 1;  // Always registered
  
  return x;
}
```

If `condition` is true, two defers execute. If false, only one defer executes.

### Defers Don't Execute on Panic

If the program panics (via `panic()` builtin), defers do NOT execute. Panics immediately terminate the program.

## Common Patterns

### Multiple Resource Cleanup

```truk
fn process() : i32 {
  var file1: *File = open_file("data.txt");
  defer close_file(file1);
  
  var file2: *File = open_file("output.txt");
  defer close_file(file2);

  var buffer: []u8 = make(@u8, 1024);
  defer delete(buffer);

  // Process files...
  
  return 0;  // All resources cleaned up automatically
}
```

### Accumulator Pattern

```truk
fn calculate() : i32 {
  var result: i32 = 0;
  defer result = result * 2;
  defer result = result + 10;
  
  result = 5;
  
  return result;  // Returns 30: (5+10)*2
}
```

### Conditional Cleanup

```truk
fn conditional_process(needs_cleanup: bool) : i32 {
  var resource: *Resource = nil;
  
  if needs_cleanup {
    resource = acquire_resource();
    defer release_resource(resource);
  }
  
  // ... processing ...
  
  return 0;  // Resource released only if acquired
}
```

## Summary

- Defer schedules code to execute at **function exit** (not block or loop exit)
- Defers execute in LIFO (Last-In-First-Out) order
- All defers execute before ANY return statement
- Defers do NOT execute on break, continue, or panic
- Defers cannot contain control flow statements (return, break, continue)
- Use defer for cleanup and finalization operations
- Defer registration is dynamic (happens during execution)

## Comparison with Other Languages

**Go**: Defer executes at function exit (same as Truk)

**C++**: Destructors execute at scope exit (different from Truk - Truk is function-scope only)

**Rust**: Drop executes at scope exit (different from Truk - Truk is function-scope only)

**Zig**: Defer executes at scope exit (different from Truk - Truk is function-scope only)

Truk's defer is simpler and more predictable: it ALWAYS executes at function exit, never at intermediate scope exits.
