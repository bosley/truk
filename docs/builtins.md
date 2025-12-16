# truk Builtin Functions

This document describes the builtin functions available in truk. Builtins provide essential functionality for memory management, array operations, and error handling.

## Type Parameter Syntax

Builtins that require type parameters accept them using the **@ prefix** followed by a type expression. This allows passing complex types including pointers, arrays, and user-defined types.

Examples:
- `@i32` - primitive type
- `@*i32` - pointer to i32
- `@[5]i32` - sized array of 5 i32s
- `@[]i32` - unsized array (slice) of i32
- `@map[i32]` - map with i32 values
- `@Point` - user-defined struct type
- `@**Point` - pointer to pointer to Point

## Memory Management

### `make(@type) -> *T`

Allocates memory on the heap for a single value of the specified type.

**Parameters:**
- `@type`: Type parameter specifying the type to allocate

**Returns:** Pointer to the allocated memory

**Example:**
```truk
var ptr: *i32 = make(@i32);
*ptr = 42;
delete(ptr);
```

**Complex types:**
```truk
var ptr_to_ptr: **i32 = make(@*i32);
```

### `make(@type, count: u64) -> []T`

Allocates a dynamic array (slice) on the heap.

**Parameters:**
- `@type`: Type parameter specifying the element type
- `count`: Number of elements to allocate

**Returns:** Unsized array (slice) of the specified type

**Example:**
```truk
var count: u64 = 100;
var arr: []i32 = make(@i32, count);
arr[0] = 42;
delete(arr);
```

**Complex element types:**
```truk
var count: u64 = 10;
var arr: [][5]i32 = make(@[5]i32, count);
```

### `make(@map[V]) -> map[V]`

Allocates and initializes a map (hash table) with string keys and values of type V.

**Parameters:**
- `@map[V]`: Type parameter specifying the map value type

**Returns:** Initialized map

**Example:**
```truk
var m: map[i32] = make(@map[i32]);
m["key"] = 42;

var ptr: *i32 = m["key"];
if ptr != nil {
  var value: i32 = *ptr;
}

delete(m);
```

**Key types:** Maps accept string-like keys: `*i8`, `*u8`, `[]i8`, `[]u8`

**Indexing semantics:** Map indexing returns `*V` (pointer to value), which is `nil` if the key doesn't exist.

**Note:** The `make` builtin is polymorphic - it allocates a single value, an array, or a map depending on the type parameter. This is similar to Go's `make` function.

### `delete(ptr: *T) -> void`

Frees memory previously allocated with `make`.

**Parameters:**
- `ptr`: Pointer to the memory to free

**Returns:** void

**Example:**
```truk
var ptr: *i32 = make(@i32);
delete(ptr);
```

### `delete(arr: []T) -> void`

Frees memory previously allocated with `make` for arrays.

**Parameters:**
- `arr`: Array to free

**Returns:** void

**Example:**
```truk
var count: u64 = 100;
var arr: []i32 = make(@i32, count);
delete(arr);
```

### `delete(m: map[V]) -> void`

Frees memory previously allocated with `make` for maps.

**Parameters:**
- `m`: Map to free

**Returns:** void

**Example:**
```truk
var m: map[i32] = make(@map[i32]);
m["key"] = 42;
delete(m);
```

**Note:** `delete` frees the map structure and all internal nodes. If map values contain pointers to heap-allocated data, you must free that data separately before deleting the map.

**Note:** The `delete` builtin automatically determines whether to free a single value, an array, or a map based on the type of its argument. Calling `delete` on already-freed memory or non-heap memory results in undefined behavior.

## Array Operations

### `len(arr: []T) -> u64`

Returns the length of an unsized array (slice).

**Parameters:**
- `arr`: Array to get the length of

**Returns:** Number of elements in the array as `u64`

**Example:**
```truk
var count: u64 = 100;
var arr: []i32 = make(@i32, count);
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
    var msg: []u8 = make(@u8, count);
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
- Single values: `var ptr: *i32 = make(@i32);`
- Arrays: `var arr: []i32 = make(@i32, count);`

Memory must be explicitly freed with `delete`.

### Ownership and Safety

**Current semantics (undefined behavior on misuse):**
- Double-free: undefined behavior
- Use-after-free: undefined behavior
- Memory leaks: your responsibility to track and free
- Null pointer dereference: undefined behavior

**Best practices:**
1. Always pair `make` with `delete`
2. Set pointers to `nil` after deleting
3. Check for `nil` before dereferencing

## Bounds Checking

Array indexing operations perform **runtime bounds checking**. Accessing an array out of bounds will cause a panic.

Example:
```truk
var count: u64 = 10;
var arr: []i32 = make(@i32, count);
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
  var ptr: *i32 = make(@i32);
  *ptr = 42;
  
  var count: u64 = 10;
  var arr: []Point = make(@Point, count);
  var size: u64 = len(arr);
  
  arr[0] = Point{x: 10, y: 20};
  
  var type_size: u64 = sizeof(@Point);
  
  delete(arr);
  delete(ptr);
}
```

## Implementation Notes

- Builtins are type-checked at compile time
- Type parameters are parsed and validated
- Return types are inferred from the type parameter
- All builtins are registered in the global scope
- Builtins cannot be shadowed or redefined
