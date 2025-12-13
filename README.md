# truk

<p align="center">
  <img src="tools/syntax/icon.png" alt="truk" width="200"/>
</p>

truk is a statically-typed systems programming language that compiles to C. It provides manual memory management with runtime bounds checking and a C-like syntax

## Language Features

### Type System

Primitive types:
- Signed integers: `i8`, `i16`, `i32`, `i64`
- Unsigned integers: `u8`, `u16`, `u32`, `u64`
- Floating point: `f32`, `f64`
- Boolean: `bool`
- Void: `void`

Arrays:
- Sized arrays: `[10]i32` (stack allocated)
- Unsized arrays: `[]i32` (heap allocated slices)
- Multi-dimensional: `[5][10]i32`

Pointers:
- Single: `*i32`
- Multiple indirection: `**i32`

User-defined types:
- Structs with named fields

### Control Flow

- `if`/`else` statements
- `while` loops
- `for` loops with C-style syntax
- `break` and `continue`
- `return` statements

### Memory Management

Manual memory management with explicit allocation and deallocation:

```truk
var ptr: *i32 = alloc(@i32);
*ptr = 42;
free(ptr);

var count: u64 = 100;
var arr: []i32 = alloc_array(@i32, count);
arr[0] = 10;
free_array(arr);
```

Runtime bounds checking on all array accesses. Out-of-bounds access causes a panic.

### Builtin Functions

- `alloc(@type)` - allocate single value on heap
- `free(ptr)` - free allocated memory
- `alloc_array(@type, count)` - allocate array on heap
- `free_array(arr)` - free allocated array
- `len(arr)` - get array length
- `sizeof(@type)` - get type size in bytes
- `panic(message)` - abort with error message

Type parameters use `@` prefix syntax to pass types to builtins.

### Example

```truk
struct Point {
  x: i32,
  y: i32
}

fn distance(p1: Point, p2: Point) : f32 {
  var dx: i32 = p2.x - p1.x;
  var dy: i32 = p2.y - p1.y;
  var sum: i32 = dx * dx + dy * dy;
  return sum as f32;
}

fn main() : i32 {
  var p1: Point = Point{x: 0, y: 0};
  var p2: Point = Point{x: 3, y: 4};
  
  var dist: f32 = distance(p1, p2);
  
  var count: u64 = 10;
  var points: []Point = alloc_array(@Point, count);
  points[0] = p1;
  points[1] = p2;
  
  free_array(points);
  return 0;
}
```

## Compilation

truk compiles to C and uses TCC (Tiny C Compiler) internally as the backend. The compiler performs type checking and validation before emitting C code.

## Documentation

- `docs/grammar.md` - Complete language grammar
- `docs/builtins.md` - Builtin function reference

## Memory Model

Stack allocation for local variables with sized arrays and structs. Heap allocation through explicit `alloc` and `alloc_array` calls. No garbage collection. Memory must be manually freed. Double-free and use-after-free result in undefined behavior.
