# Welcome to truk

truk is a statically-typed systems programming language that compiles to C. It combines manual memory management with runtime safety checks, providing a C-like development experience with modern language features.

## Work Effort, and AI

As I work on truk things are likely to change fast. I am solidifying the documents now (16, Dec, 2025), but as I discover bugs or add features there may be some document drift. I usually try to keep docs in sync with development, but every once in a while things slip through. My work strategy is "code like hell, test, refine, test, refactor, refine, test, then document," and I've been utilizing AI to assist in test generation, document generation, and large refactoring efforts. I've found that utilizing the clankers this way is generally pretty safe, and the ability to generate a handful of weird tests as I'm working something through has been immeasurably valuable. That said, some of the docs will obviously have the AI stink on them, but they will be orders of magnitude better than if I begrudgingly flesh them out entirely by hand. I am _very_ hesitant in the use of AI on code bases such as this so please, if you decide to contribute and use AI, make sure the contributions are high quality and be honest about the usage. Thats all I care about. 

## Quick Start

Choose your path:

- **[I want to build and use truk](#getting-started)** → Start here to compile truk programs
- **[I want to learn the language](#language-reference)** → Explore language features and syntax
- **[I want to contribute to the compiler](#compiler-internals)** → Understand the implementation

## What Makes truk Different?

**Systems Programming with Safety:**
- Manual memory management like C
- Runtime bounds checking on array access
- Explicit allocation and deallocation
- No garbage collection overhead

**Modern Language Features:**
- Strong static type system
- First-class functions (lambdas)
- Hash maps built into the language
- Defer statements for cleanup
- Privacy through naming conventions

**C Interoperability:**
- Seamless C function calls
- Import C headers directly
- Compile to readable C code
- Use any C library

**Fast Compilation:**
- Compiles to C using TCC backend
- JIT execution mode for rapid iteration
- Transpile to C for custom build systems

## Language Overview

### Type System

truk provides a rich type system:

- **Primitives**: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`, `void`
- **Arrays**: Sized `[10]i32` and unsized `[]i32` (slices)
- **Pointers**: `*i32`, `**i32` with nil checking
- **Maps**: `map[K, V]` hash tables with typed keys and values
- **Structs**: User-defined types with named fields

See [grammar.md](language/grammar.md) for complete syntax.

### Memory Management

Explicit allocation and deallocation with runtime safety:

```truk
var ptr: *i32 = make(@i32);
*ptr = 42;
delete(ptr);

var arr: []i32 = make(@i32, 100);
arr[0] = 10;
delete(arr);

var m: map[*u8, i32] = make(@map[*u8, i32]);
m["key"] = 42;
delete(m);
```

See [builtins.md](language/builtins.md) for memory management details.

### Control Flow

Standard control structures with defer for cleanup:

```truk
if condition {
    defer cleanup();
    process();
}

while i < count {
    work();
    i = i + 1;
}

for i = 0; i < 10; i = i + 1 {
    iterate();
}
```

See [defer.md](language/defer.md) for defer semantics.

### Functions and Lambdas

First-class functions with non-capturing lambdas:

```truk
fn process(x: i32, op: fn(i32) : i32) : i32 {
    return op(x);
}

var result: i32 = process(10, fn(x: i32) : i32 {
    return x * 2;
});
```

See [lambdas.md](language/lambdas.md) for lambda details.

### Maps and Iteration

Built-in hash maps with the `each` builtin for iteration:

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
m["one"] = 1;
m["two"] = 2;

var sum: i32 = 0;
each(m, &sum, fn(key: *u8, val: *i32, ctx: *i32) : bool {
    *ctx = *ctx + *val;
    return true;
});

delete(m);
```

See [maps.md](language/maps.md) for map usage.

### C Interoperability

Import and use C libraries directly:

```truk
cimport <stdio.h>;
cimport <math.h>;

extern fn printf(fmt: *i8, ...args): i32;
extern fn sqrt(x: f64): f64;

fn main(): i32 {
    printf("sqrt(16) = %f\n", sqrt(16.0));
    return 0;
}
```

See [imports.md](language/imports.md) for C interop details.

## Documentation Index

### Getting Started

**For users who want to build and run truk programs:**

- [Building truk Programs](getting-started/building.md) - Compilation commands and workflows

### Language Reference

**For users learning to write truk code:**

- [Grammar](language/grammar.md) - Complete language syntax
- [Builtin Functions](language/builtins.md) - Memory management, arrays, type operations
- [Maps](language/maps.md) - Hash table types and operations
- [Defer Statements](language/defer.md) - Scope-based cleanup and finalization
- [Imports](language/imports.md) - Module system and C interoperability
- [Lambdas](language/lambdas.md) - First-class functions and callbacks
- [Privacy](language/privacy.md) - File and shard-based privacy system
- [Runtime Architecture](language/runtime.md) - How truk programs execute

### Compiler Internals

**For contributors working on the truk compiler:**

- [Error Handling](compiler-internals/error-handling.md) - Unified error reporting system
- [Error Flow Diagrams](compiler-internals/error-flow-diagram.md) - Error flow through compilation stages
- [C Emitter](compiler-internals/emitter.md) - Code generation architecture
- [Type Checker](compiler-internals/typechecker.md) - Type validation and semantic analysis

## Example Program

Here's a complete truk program demonstrating key features:

```truk
struct Point {
    x: i32,
    y: i32
}

fn distance_squared(p1: Point, p2: Point) : i32 {
    var dx: i32 = p2.x - p1.x;
    var dy: i32 = p2.y - p1.y;
    return dx * dx + dy * dy;
}

fn main() : i32 {
    var points: []Point = make(@Point, 3);
    defer delete(points);
    
    points[0] = Point{x: 0, y: 0};
    points[1] = Point{x: 3, y: 4};
    points[2] = Point{x: 6, y: 8};
    
    var cache: map[*u8, i32] = make(@map[*u8, i32]);
    defer delete(cache);
    
    cache["first"] = distance_squared(points[0], points[1]);
    cache["second"] = distance_squared(points[1], points[2]);
    
    var ptr: *i32 = cache["first"];
    if ptr != nil {
        return *ptr;
    }
    
    return 0;
}
```

This example shows:
- Struct definitions
- Array allocation with `make`
- Defer for automatic cleanup
- Map creation and indexing
- Nil checking for map access
- Proper memory management

## Next Steps

1. **Build your first program**: Start with [building.md](getting-started/building.md)
2. **Learn the syntax**: Read through [grammar.md](language/grammar.md)
3. **Explore examples**: Check the `tests/` directory for more examples
4. **Contribute**: See [compiler-internals/](compiler-internals/) to understand the implementation

## Appendix: Complete Documentation Map

### Getting Started
- [getting-started/building.md](getting-started/building.md)

### Language Reference
- [language/builtins.md](language/builtins.md)
- [language/defer.md](language/defer.md)
- [language/grammar.md](language/grammar.md)
- [language/imports.md](language/imports.md)
- [language/lambdas.md](language/lambdas.md)
- [language/maps.md](language/maps.md)
- [language/privacy.md](language/privacy.md)
- [language/runtime.md](language/runtime.md)

### Compiler Internals
- [compiler-internals/emitter.md](compiler-internals/emitter.md)
- [compiler-internals/error-flow-diagram.md](compiler-internals/error-flow-diagram.md)
- [compiler-internals/error-handling.md](compiler-internals/error-handling.md)
- [compiler-internals/typechecker.md](compiler-internals/typechecker.md)
