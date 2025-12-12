# Getting to Know Truk

Truk is a simple, statically-typed systems programming language with C-style manual memory management. It compiles to C and provides low-level control with a clean, minimal syntax.

## Philosophy

- Explicit over implicit
- Manual memory management
- No hidden allocations
- Compiles to C
- Simple and predictable

## Hello World

Every Truk program starts with a `main` function:

```truk
fn main() : i32 {
  return 0;
}
```

The `main` function must return an `i32` (32-bit signed integer), which serves as the program's exit code.

## Variables and Types

### Declaring Variables

Variables are declared with `var` and require explicit type annotations:

```truk
fn main() : i32 {
  var x: i32 = 5;
  return x;
}
```

Constants use `const` and must be initialized:

```truk
const MAX_SIZE: i32 = 100;
```

### Primitive Types

**Signed integers:** `i8`, `i16`, `i32`, `i64`

**Unsigned integers:** `u8`, `u16`, `u32`, `u64`

**Floating point:** `f32`, `f64`

**Boolean:** `bool` (values: `true`, `false`)

**Void:** `void` (for functions that don't return a value)

### Type Examples

```truk
var byte: i8 = 127;
var count: u64 = 1000000000;
var pi: f32 = 3.14;
var active: bool = true;
```

## Expressions and Operators

### Arithmetic Operators

```truk
var sum: i32 = 3 + 7;
var diff: i32 = 10 - 4;
var prod: i32 = 5 * 6;
var quot: i32 = 20 / 4;
var rem: i32 = 10 % 3;
```

### Bitwise Operators

```truk
var and_result: i32 = 7 & 8;
var or_result: i32 = 7 | 8;
var xor_result: i32 = 7 ^ 8;
var left_shift: i32 = 2 << 3;
var right_shift: i32 = 8 >> 2;
var not_result: i32 = ~5;
```

### Comparison Operators

```truk
var equal: bool = x == y;
var not_equal: bool = x != y;
var less: bool = x < y;
var less_equal: bool = x <= y;
var greater: bool = x > y;
var greater_equal: bool = x >= y;
```

### Logical Operators

```truk
var and_logic: bool = true && false;
var or_logic: bool = true || false;
var not_logic: bool = !true;
```

### Compound Assignment

```truk
x += 5;
x -= 3;
x *= 2;
x /= 4;
x %= 3;
```

### Operator Precedence

Parentheses can be used to control evaluation order:

```truk
var result: i32 = (2 + 3) * 4;
```

## Control Flow

### If Statements

```truk
fn main() : i32 {
  var x: i32 = 20;
  if x <= 15 {
    return 5;
  }
  return 0;
}
```

### If-Else

```truk
if condition {
  return 1;
} else {
  return 0;
}
```

### While Loops

```truk
fn main() : i32 {
  var sum: i32 = 0;
  var i: i32 = 0;
  while i < 5 {
    sum = sum + 2;
    i = i + 1;
  }
  return sum;
}
```

### For Loops

```truk
fn main() : i32 {
  var sum: i32 = 0;
  var i: i32 = 0;
  for ; i < 5; i = i + 1 {
    sum = sum + 3;
  }
  return sum;
}
```

The for loop syntax is: `for init; condition; increment { body }`

Any section can be omitted (as shown above where init is empty).

### Break and Continue

```truk
while i < 5 {
  i = i + 1;
  if i == 2 {
    continue;
  }
  sum = sum + 1;
}
```

## Functions

### Function Declaration

```truk
fn returns_zero() : i32 {
  return 0;
}

fn main() : i32 {
  return returns_zero();
}
```

### Parameters

```truk
fn multiply(a: i32, b: i32) : i32 {
  return a * b;
}

fn main() : i32 {
  var x: i32 = multiply(2, 3);
  return x - 1;
}
```

### Void Functions

Functions other than `main` can return `void`:

```truk
fn do_something() : void {
  var x: i32 = 5;
}

fn main() : i32 {
  do_something();
  return 0;
}
```

Note: The `main` function must always return `i32`.

### Variadic Functions

Functions can accept a variable number of arguments using the `...` syntax:

```truk
fn get_first(x: i32, ...args) : i32 {
  var value: i32 = __TRUK_VA_ARG_I32();
  return value;
}

fn main() : i32 {
  return get_first(0, 42, 100);
}
```

Variadic arguments are accessed using builtin functions:
- `__TRUK_VA_ARG_I32()` - Get next i32 argument
- `__TRUK_VA_ARG_I64()` - Get next i64 argument
- `__TRUK_VA_ARG_F64()` - Get next f64 argument
- `__TRUK_VA_ARG_PTR()` - Get next pointer argument

The `...` must be the last parameter. You must know the types and count of variadic arguments at the call site.

## Structs

### Defining Structs

```truk
struct Point {
  x: i32,
  y: i32
}
```

Fields are separated by commas. Trailing comma is optional.

### Creating Struct Instances

```truk
var p: Point = Point{x: 10, y: 20};
```

### Accessing Fields

```truk
fn main() : i32 {
  var p: Point = Point{x: 10, y: 20};
  return p.x;
}
```

### Nested Structs

```truk
struct Vec3 {
  x: f64,
  y: f64,
  z: f64
}

struct Container {
  id: i32,
  active: bool
}
```

## Arrays

### Sized Arrays (Stack Allocation)

Sized arrays have a compile-time known size and are allocated on the stack:

```truk
var arr: [5]i32;
arr[0] = 10;
arr[1] = 20;
arr[4] = 100;
```

### Array Indexing

Arrays are zero-indexed. Out-of-bounds access causes a runtime panic.

```truk
var value: i32 = arr[0];
```

### Array Literals

Sized arrays can be initialized with array literal syntax:

```truk
struct Container {
  value: i32,
  data: [3]i32
}

fn main() : i32 {
  var container: Container = Container{
    value: 42,
    data: [10, 20, 30]
  };
  return container.data[1];
}
```

Array literals are primarily used for initializing struct fields with array types.

### Arrays of Different Types

```truk
var arr_i8: [5]i8;
var arr_u64: [10]u64;
var arr_f32: [3]f32;
var arr_bool: [5]bool;
```

### Arrays of Structs

```truk
struct Point {
  x: i32,
  y: i32
}

fn main() : i32 {
  var arr_point: [5]Point;
  arr_point[0] = Point{x: 10, y: 20};
  arr_point[1] = Point{x: 30, y: 40};
  return 0;
}
```

### Unsized Arrays (Dynamic/Heap Allocation)

Unsized arrays are allocated on the heap and have runtime-determined size:

```truk
var count: u64 = 5;
var arr: []i32 = alloc_array(@i32, count);
arr[0] = 10;
arr[1] = 20;
free_array(arr);
```

Note the `@i32` syntax - this is a type parameter telling `alloc_array` what type to allocate.

## Pointers and Memory Management

### Pointer Basics

Pointers are declared with `*` prefix:

```truk
var ptr: *i32;
```

### Taking Address

Use `&` to get the address of a variable:

```truk
var x: i32 = 42;
var ptr: *i32 = &x;
```

### Dereferencing

Use `*` to access the value a pointer points to:

```truk
var value: i32 = *ptr;
*ptr = 100;
```

### Null Pointers

The `nil` keyword represents a null pointer:

```truk
var ptr: *i32 = nil;
if ptr == nil {
  return 1;
}
```

`nil` compiles to `NULL` in C and can be assigned to any pointer type.

### Heap Allocation

Allocate single values on the heap with `alloc`:

```truk
fn main() : i32 {
  var ptr: *i32 = alloc(@i32);
  *ptr = 42;
  free(ptr);
  return 0;
}
```

**Important:** Every `alloc` must be paired with a `free`.

### Array Allocation

Allocate arrays on the heap with `alloc_array`:

```truk
var count: u64 = 100;
var arr: []i32 = alloc_array(@i32, count);
arr[0] = 42;
free_array(arr);
```

**Important:** Every `alloc_array` must be paired with a `free_array`.

### Complex Type Parameters

Type parameters support complex types:

```truk
var ptr_to_ptr: **i32 = alloc(@*i32);
var arr_of_arrays: [][5]i32 = alloc_array(@[5]i32, count);
var arr_of_structs: []Point = alloc_array(@Point, count);
```

## Builtins

Truk provides essential builtin functions for memory management and utilities.

### Memory Management

**`alloc(@type) -> *T`** - Allocate single value on heap

```truk
var ptr: *i32 = alloc(@i32);
free(ptr);
```

**`free(ptr: *T) -> void`** - Free allocated memory

**`alloc_array(@type, count: u64) -> []T`** - Allocate array on heap

```truk
var arr: []i32 = alloc_array(@i32, 10);
free_array(arr);
```

**`free_array(arr: []T) -> void`** - Free allocated array

### Array Operations

**`len(arr: []T) -> u64`** - Get length of unsized array

```truk
var count: u64 = 5;
var arr: []i32 = alloc_array(@i32, count);
var size: u64 = len(arr);
free_array(arr);
```

### Type Information

**`sizeof(@type) -> u64`** - Get size of type in bytes

```truk
var size: u64 = sizeof(@i32);
var ptr_size: u64 = sizeof(@*i32);
```

### Error Handling

**`panic(message: []u8) -> void`** - Abort program with error message

```truk
if b == 0 {
  var count: u64 = 18;
  var msg: []u8 = alloc_array(@u8, count);
  panic(msg);
}
```

### External Functions

**`printf(format: *u8, ...) -> void`** - Print formatted output (variadic)

```truk
fn main() : i32 {
  printf("Simple test\n");
  return 0;
}
```

Note: `printf` is an external C function. The format parameter is a pointer to u8, not an array.

## Memory Model

### Stack vs Heap

**Stack (automatic):**
- Local variables with sized arrays: `var arr: [10]i32;`
- Structs: `var p: Point;`
- Primitives: `var x: i32;`
- Automatically freed when out of scope

**Heap (manual):**
- Single values: `var ptr: *i32 = alloc(@i32);`
- Arrays: `var arr: []i32 = alloc_array(@i32, count);`
- Must be explicitly freed

### Safety Considerations

Truk has C-style undefined behavior on:
- Double-free
- Use-after-free
- Memory leaks
- Null pointer dereference
- Out-of-bounds access (runtime panic)

### Best Practices

1. Always pair `alloc` with `free`
2. Always pair `alloc_array` with `free_array`
3. Set pointers to `nil` after freeing
4. Check for `nil` before dereferencing
5. Be mindful of array bounds

## Type Casting

Use `as` to cast between types:

```truk
var x: i32 = 42;
var y: i64 = x as i64;
var z: f32 = x as f32;
```

## Comments

Truk supports C-style comments:

```truk
// Single line comment

/*
  Multi-line
  comment
*/
```

## Complete Example

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
  var p1: Point = Point{x: 0, y: 0};
  var p2: Point = Point{x: 3, y: 4};
  
  var dist_sq: i32 = distance_squared(p1, p2);
  
  var count: u64 = 10;
  var points: []Point = alloc_array(@Point, count);
  
  var i: i32 = 0;
  while i < 10 {
    points[i as u64] = Point{x: i, y: i * 2};
    i = i + 1;
  }
  
  free_array(points);
  
  return 0;
}
```

## String Literals

String literals exist for C interop but are not general-purpose strings:

```truk
fn main() : i32 {
  printf("Hello, world!\n");
  return 0;
}
```

String literals are compile-time constants that compile to C string literals. They are primarily used for:
- Calling external C functions like `printf`
- Error messages in `panic`

For runtime string manipulation, use `[]u8` arrays.

## What's Missing

Truk is intentionally minimal. Notable omissions:

- No string type (use `[]u8` arrays for runtime strings)
- No standard library
- No generics
- No closures
- No garbage collection
- No exceptions (use return codes or panic)
- No modules/imports (single file compilation)
- No enums
- No pattern matching
- No operator overloading

## Next Steps

1. Read `docs/grammar.md` for complete syntax specification
2. Read `docs/builtins.md` for detailed builtin documentation
3. Explore `tests/` directory for more examples
4. Write simple programs and compile them

Truk is designed to be simple and predictable. If you understand C, you understand Truk's memory model. The syntax is cleaner, but the semantics are familiar.
