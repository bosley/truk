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

Maps:
- Hash tables with typed keys and values: `map[K, V]`
- Key types: primitives (i8-i64, u8-u64, f32, f64, bool) or string pointers (*u8, *i8)
- Indexing returns pointer: `m["key"]` returns `*V` (nil if key doesn't exist)
- Nil checking: `if m["key"] != nil { ... }`

Pointers:
- Single: `*i32`
- Multiple indirection: `**i32`

User-defined types:
- Structs with named fields

### Control Flow

- `if`/`else if`/`else` statements
- `while` loops
- `for` loops with C-style syntax
- `break` and `continue`
- `return` statements

### Memory Management

Manual memory management with explicit allocation and deallocation:

```truk
var ptr: *i32 = make(@i32);
*ptr = 42;
delete(ptr);

var count: u64 = 100;
var arr: []i32 = make(@i32, count);
arr[0] = 10;
delete(arr);
```

Runtime bounds checking on all array accesses. Out-of-bounds access causes a panic.

### Builtin Functions

- `make(@type)` - allocate single value on heap
- `make(@type, count)` - allocate array on heap
- `make(@map[K, V])` - allocate and initialize map with key type K and value type V
- `delete(ptr)` - free allocated memory (single value, array, or map)
- `delete(m[key])` - remove key-value pair from map
- `len(arr)` - get array length
- `sizeof(@type)` - get type size in bytes
- `panic(message: []u8)` - abort with error message
- `each(collection, context, callback)` - iterate over maps or slices

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
  var points: []Point = make(@Point, count);
  points[0] = p1;
  points[1] = p2;
  
  var cache: map[*u8, Point] = make(@map[*u8, Point]);
  cache["origin"] = p1;
  cache["target"] = p2;
  
  var ptr: *Point = cache["origin"];
  if ptr != nil {
    var x: i32 = (*ptr).x;
  }
  
  delete(cache);
  delete(points);
  return 0;
}
```

### C Interoperability

truk provides seamless interoperability with C through `cimport` and `extern` declarations.

**Importing C headers:**
```truk
cimport <stdio.h>;
cimport <math.h>;
cimport "myheader.h";
```

**Declaring C functions:**
```truk
extern fn printf(fmt: *i8, ...args): i32;
extern fn sqrt(x: f64): f64;
extern fn fopen(filename: *i8, mode: *i8): *FILE;
```

**Opaque structs (forward declarations):**
```truk
extern struct FILE;

extern fn fopen(filename: *i8, mode: *i8): *FILE;
extern fn fclose(file: *FILE): i32;
```

**Defined structs (with layout):**
```truk
cimport <time.h>;

extern struct tm {
    tm_sec: i32,
    tm_min: i32,
    tm_hour: i32,
    tm_mday: i32,
    tm_mon: i32,
    tm_year: i32
}

extern fn time(timer: *i64): i64;
extern fn localtime(timer: *i64): *tm;
```

**C variables:**
```truk
cimport <errno.h>;

extern var errno: i32;
extern var stdin: *FILE;
```

**Complete example:**
```truk
cimport <stdio.h>;
cimport <math.h>;

extern struct FILE;
extern fn fopen(filename: *i8, mode: *i8): *FILE;
extern fn fclose(file: *FILE): i32;
extern fn fprintf(file: *FILE, fmt: *i8, ...args): i32;
extern fn sqrt(x: f64): f64;

fn main(): i32 {
    var result: f64 = sqrt(16.0);
    
    var f: *FILE = fopen("output.txt", "w");
    if f != nil {
        fprintf(f, "sqrt(16) = %f\n", result);
        fclose(f);
    }
    
    return 0;
}
```

See `docs/language/imports.md` for complete details on C interop.

## Compilation

truk compiles to C and uses TCC (Tiny C Compiler) internally as the backend. The compiler performs type checking and validation before emitting C code.

## Documentation

- `docs/language/grammar.md` - Complete language grammar
- `docs/language/builtins.md` - Builtin function reference
- `docs/language/maps.md` - Map types and usage
- `docs/language/defer.md` - Defer statement semantics
- `docs/language/imports.md` - Import system
- `docs/language/lambdas.md` - Lambda functions
- `docs/language/privacy.md` - Privacy and shards
- `docs/language/runtime.md` - Runtime architecture

## Memory Model

Stack allocation for local variables with sized arrays and structs. Heap allocation through explicit `make` calls. No garbage collection. Memory must be manually freed with `delete`. Double-free and use-after-free result in undefined behavior.
