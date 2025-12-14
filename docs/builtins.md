# truk Builtin Functions

This document describes the builtin functions available in truk. Builtins provide essential functionality for memory management, array operations, and error handling.

## Type Parameter Syntax

Builtins that require type parameters accept them using the **@ prefix** followed by a type expression. This allows passing complex types including pointers, arrays, and user-defined types.

Examples:
- `@i32` - primitive type
- `@*i32` - pointer to i32
- `@[5]i32` - sized array of 5 i32s
- `@[]i32` - unsized array (slice) of i32
- `@Point` - user-defined struct type
- `@**Point` - pointer to pointer to Point

## Memory Management

### `alloc(@type) -> *T`

Allocates memory on the heap for a single value of the specified type.

**Parameters:**
- `@type`: Type parameter specifying the type to allocate

**Returns:** Pointer to the allocated memory

**Example:**
```truk
var ptr: *i32 = alloc(@i32);
*ptr = 42;
free(ptr);
```

**Complex types:**
```truk
var ptr_to_ptr: **i32 = alloc(@*i32);
```

### `free(ptr: *T) -> void`

Frees memory previously allocated with `alloc`.

**Parameters:**
- `ptr`: Pointer to the memory to free

**Returns:** void

**Example:**
```truk
var ptr: *i32 = alloc(@i32);
free(ptr);
```

**Note:** Calling `free` on already-freed memory or non-heap memory results in undefined behavior.

### `alloc_array(@type, count: u64) -> []T`

Allocates a dynamic array (slice) on the heap.

**Parameters:**
- `@type`: Type parameter specifying the element type
- `count`: Number of elements to allocate

**Returns:** Unsized array (slice) of the specified type

**Example:**
```truk
var count: u64 = 100;
var arr: []i32 = alloc_array(@i32, count);
arr[0] = 42;
free_array(arr);
```

**Complex element types:**
```truk
var count: u64 = 10;
var arr: [][5]i32 = alloc_array(@[5]i32, count);
```

### `free_array(arr: []T) -> void`

Frees memory previously allocated with `alloc_array`.

**Parameters:**
- `arr`: Array to free

**Returns:** void

**Example:**
```truk
var count: u64 = 100;
var arr: []i32 = alloc_array(@i32, count);
free_array(arr);
```

## Array Operations

### `len(arr: []T) -> u64`

Returns the length of an unsized array (slice).

**Parameters:**
- `arr`: Array to get the length of

**Returns:** Number of elements in the array as `u64`

**Example:**
```truk
var count: u64 = 100;
var arr: []i32 = alloc_array(@i32, count);
var size: u64 = len(arr);
```

**Note:** `len` only works with unsized arrays (`[]T`). Sized arrays (`[N]T`) have a compile-time known size.

## Type Information

### `sizeof(@type) -> u64`

Returns the size in bytes of the specified type.

**Parameters:**
- `@type`: Type parameter specifying the type

**Returns:** Size of the type in bytes as `u64`

**Example:**
```truk
var size: u64 = sizeof(@i32);
var ptr_size: u64 = sizeof(@*i32);
```

## Program Arguments

### `argc() -> i32`

Returns the number of command-line arguments passed to the program.

**Parameters:** None

**Returns:** Number of arguments as `i32` (includes program name as argv[0])

**Example:**
```truk
fn main() : i32 {
  var count: i32 = argc();
  printf("Program received %d arguments\n", count);
  return 0;
}
```

**Note:** The argument count includes the program name itself at index 0, so `argc()` is always at least 1.

### `argv(index: i32) -> []u8`

Returns the command-line argument at the specified index as a byte slice.

**Parameters:**
- `index`: Zero-based index of the argument to retrieve

**Returns:** Unsized array (slice) of bytes containing the argument string

**Example:**
```truk
fn main() : i32 {
  var count: i32 = argc();
  
  if count > 1 {
    var first_arg: []u8 = argv(1);
    var arg_len: u64 = len(first_arg);
    printf("First argument length: %llu\n", arg_len);
  }
  
  return 0;
}
```

**Bounds checking:** If `index` is negative or >= `argc()`, the program will panic with an out-of-bounds error.

**Complete example:**
```truk
fn print_all_args() : void {
  var count: i32 = argc();
  var i: i32 = 0;
  
  while i < count {
    var arg: []u8 = argv(i);
    printf("argv[%d]: length %llu\n", i, len(arg));
    i = i + 1;
  }
}

fn main() : i32 {
  print_all_args();
  return 0;
}
```

**Usage with truk CLI:**
```bash
truk run program.truk -- arg1 arg2 "arg with spaces"

truk program.truk -o myprogram
./myprogram arg1 arg2 "arg with spaces"
```

## Error Handling

### `panic(message: []u8) -> void`

Aborts program execution with an error message.

**Parameters:**
- `message`: Array of bytes containing the error message

**Returns:** Does not return (program terminates)

**Example:**
```truk
fn divide(a: i32, b: i32) : i32 {
  if b == 0 {
    var count: u64 = 18;
    var msg: []u8 = alloc_array(@u8, count);
    panic(msg);
  }
  return a / b;
}
```

**Note:** On panic, the program terminates immediately. No destructors or cleanup code is run.

## Memory Model

truk follows a **C-style manual memory management** model:

### Stack Allocation (Automatic)
- Local variables with sized arrays: `var arr: [10]i32;`
- Structs: `var p: Point;`
- Primitives: `var x: i32;`

Memory is automatically freed when the variable goes out of scope.

### Heap Allocation (Manual)
- Single values: `var ptr: *i32 = alloc(@i32);`
- Arrays: `var arr: []i32 = alloc_array(@i32, count);`

Memory must be explicitly freed with `free` or `free_array`.

### Ownership and Safety

**Current semantics (undefined behavior on misuse):**
- Double-free: undefined behavior
- Use-after-free: undefined behavior
- Memory leaks: your responsibility to track and free
- Null pointer dereference: undefined behavior

**Best practices:**
1. Always pair `alloc` with `free`
2. Always pair `alloc_array` with `free_array`
3. Set pointers to `nil` after freeing
4. Check for `nil` before dereferencing

## Bounds Checking

Array indexing operations perform **runtime bounds checking**. Accessing an array out of bounds will cause a panic.

Example:
```truk
var count: u64 = 10;
var arr: []i32 = alloc_array(@i32, count);
arr[10] = 42;
```

This will panic at runtime with an out-of-bounds error.

## Allocation Failure

If memory allocation fails (out of memory), the program will **panic** (similar to Go). There is no way to handle allocation failure gracefully in the current implementation.

## Complete Example

```truk
struct Point {
  x: i32,
  y: i32
}

fn main() : void {
  var ptr: *i32 = alloc(@i32);
  *ptr = 42;
  
  var count: u64 = 10;
  var arr: []Point = alloc_array(@Point, count);
  var size: u64 = len(arr);
  
  arr[0] = Point{x: 10, y: 20};
  
  var type_size: u64 = sizeof(@Point);
  
  free_array(arr);
  free(ptr);
}
```

## Implementation Notes

- Builtins are type-checked at compile time
- Type parameters are parsed and validated
- Return types are inferred from the type parameter
- All builtins are registered in the global scope
- Builtins cannot be shadowed or redefined
