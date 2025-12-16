# Defer Statement

## Overview

The `defer` statement schedules code to execute when the current **scope** exits. Deferred code executes in Last-In-First-Out (LIFO) order, meaning the last defer registered is the first to execute.

Defer is useful for cleanup operations, ensuring resources are released, or performing final computations. Defers execute when exiting:
- **Blocks** (anonymous scopes)
- **Loops** (while, for)
- **Functions**
- **Lambdas**

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

**Defers execute when exiting their declaring scope**. Each scope (block, loop, function, lambda) maintains its own defer stack. When a scope exits normally, its defers execute in LIFO order.

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 1;  // Function scope defer
  {
    defer x = x + 2;  // Block scope defer
    x = x + 10;
  }  // Block defer executes here: x=10, then x=12
  return x;  // Function defer executes here: x=12, then x=13
}
```

Returns `13`. The block defer executes when exiting the block, then the function defer executes at function exit.

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

### Block Scope Defer

```truk
fn main() : i32 {
  var x: i32 = 0;
  {
    defer x = x + 5;
    x = x + 1;
  }  // Defer executes here: x=1, then x=6
  x = x + 4;
  return x;  // Returns 10
}
```

Returns `10`. The defer executes when exiting the block, not at function exit.

### Nested Block Defers

```truk
fn main() : i32 {
  var x: i32 = 0;
  {
    defer x = x + 1;  // Outer block defer
    {
      defer x = x + 2;  // Inner block defer
    }  // Inner defer executes: x=0, then x=2
    defer x = x + 3;  // Another outer block defer
  }  // Outer defers execute in LIFO: x=2, then x=5, then x=6
  return x;  // Returns 6
}
```

Returns `6`. Defers execute when their declaring scope exits, in LIFO order within each scope.

### Loop Scope Defer

```truk
fn main() : i32 {
  var x: i32 = 0;
  var i: i32 = 0;
  while i < 3 {
    defer x = x + 1;
    x = x + 10;
    i = i + 1;
  }  // Defer executes at end of each iteration
  return x;
}
```

Returns `33`. Each iteration: x+=10, then defer executes x+=1. After 3 iterations: (10+1) + (10+1) + (10+1) = 33.

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

Defers in loops execute at the end of each loop iteration. Each time the loop body executes, the defer is registered and executes when the iteration completes:

```truk
fn main() : i32 {
  var x: i32 = 0;
  var i: i32 = 0;
  while i < 5 {
    defer x = x + 1;  // Executes at end of each iteration
    i = i + 1;
  }
  return x;  // Returns 5
}
```

Returns `5`. The defer executes 5 times, once at the end of each loop iteration.

**Important**: Loop scopes are special - defers registered in the loop body execute at the end of each iteration, not just once at loop exit.

### Break and Continue

Break and continue statements execute defers from the current scope up to (but not including) the enclosing loop scope:

```truk
fn main() : i32 {
  var x: i32 = 0;
  defer x = x + 100;  // Function scope defer
  var i: i32 = 0;
  while i < 10 {
    defer x = x + 10;  // Loop scope defer
    {
      defer x = x + 1;  // Block scope defer
      x = x + 1;
      i = i + 1;
      if i == 5 {
        break;  // Executes block defer, then breaks (loop defer NOT executed)
      }
    }  // Block defer executes here on normal exit
  }
  return x;  // Function defer executes here
}
```

On break at i==5:
1. Block defer executes: `x = x + 1`
2. Break exits loop (loop defer does NOT execute)
3. At function return, function defer executes: `x = x + 100`

The loop defer (`x = x + 10`) executes at the end of each normal iteration, but NOT when breaking.

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
  }  // If registered, executes here when exiting if block
  
  defer x = x + 1;  // Always registered
  
  return x;
}
```

If `condition` is true, the block defer executes when exiting the if block, then the function defer executes at return. If false, only the function defer executes.

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

- Defer schedules code to execute at **scope exit** (blocks, loops, functions, lambdas)
- Defers execute in LIFO (Last-In-First-Out) order within each scope
- Block and loop defers execute when exiting their scope normally
- Return statements execute all defers from current scope up to function scope
- Break/continue execute defers from current scope up to (but not including) loop scope
- Defers cannot contain control flow statements (return, break, continue)
- Use defer for cleanup and finalization operations
- Defer registration is dynamic (happens during execution)

## Comparison with Other Languages

**Go**: Defer executes at function exit only

**C++**: Destructors execute at scope exit (same as Truk)

**Rust**: Drop executes at scope exit (same as Truk)

**Zig**: Defer executes at scope exit (same as Truk)

Truk's defer follows the Zig model: defers execute when exiting their declaring scope, providing fine-grained control over cleanup and finalization.
