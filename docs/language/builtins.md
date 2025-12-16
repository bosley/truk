# truk Builtin Functions

This document describes the builtin functions available in truk. Builtins provide essential functionality for memory management, array operations, and error handling.

## Type Parameter Syntax

Builtins that require type parameters accept them using the **@ prefix** followed by a type expression. This allows passing complex types including pointers, arrays, and user-defined types.

Examples:
- `@i32` - primitive type
- `@*i32` - pointer to i32
- `@[5]i32` - sized array of 5 i32s
- `@[]i32` - unsized array (slice) of i32
- `@map[*u8, i32]` - map with string keys and i32 values
- `@map[i32, *u8]` - map with i32 keys and string values
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

### `make(@map[K, V]) -> map[K, V]`

Allocates and initializes a map (hash table) with typed keys and values.

**Parameters:**
- `@map[K, V]`: Type parameter specifying the map key type and value type

**Returns:** Initialized map

**Example with string keys:**
```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
m["key"] = 42;

var ptr: *i32 = m["key"];
if ptr != nil {
  var value: i32 = *ptr;
}

delete(m);
```

**Example with integer keys:**
```truk
var m: map[i32, *u8] = make(@map[i32, *u8]);
m[1] = "one";
m[2] = "two";

var ptr: **u8 = m[1];
if ptr != nil {
  var value: *u8 = *ptr;
}

delete(m);
```

**Supported key types:** Primitives (i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool) and string pointers (*i8, *u8)

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

### `delete(m: map[K, V]) -> void`

Frees memory previously allocated with `make` for maps.

**Parameters:**
- `m`: Map to free

**Returns:** void

**Example:**
```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
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

## Iteration

### `each(collection: map[K, V] | []T, context: *C, callback: fn(...) : bool) -> void`

Iterates over a map or slice, calling a callback function for each element.

**For Maps:**

**Parameters:**
- `collection`: Map to iterate over
- `context`: Pointer to user-provided context data
- `callback`: Function with signature `fn(key: K, val: *V, ctx: *C) : bool`
  - `key`: The key (type matches map's key type)
  - `val`: Pointer to the value
  - `ctx`: Context pointer (same as `context` parameter)
  - Returns `bool`: `true` to continue iteration, `false` to stop early

**Example with string keys:**
```truk
fn main() : i32 {
  var m: map[*u8, i32] = make(@map[*u8, i32]);
  m["one"] = 1;
  m["two"] = 2;
  m["three"] = 3;
  
  var count: i32 = 0;
  each(m, &count, fn(key: *u8, val: *i32, ctx: *i32) : bool {
    *ctx = *ctx + 1;
    return true;
  });
  
  delete(m);
  return count;
}
```

**Example with integer keys:**
```truk
fn main() : i32 {
  var m: map[i32, i32] = make(@map[i32, i32]);
  m[1] = 10;
  m[2] = 20;
  m[3] = 30;
  
  var sum: i32 = 0;
  each(m, &sum, fn(key: i32, val: *i32, ctx: *i32) : bool {
    *ctx = *ctx + *val;
    return true;
  });
  
  delete(m);
  return sum;
}
```

**For Slices:**

**Parameters:**
- `collection`: Slice to iterate over
- `context`: Pointer to user-provided context data
- `callback`: Function with signature `fn(elem: *T, ctx: *C) : bool`
  - `elem`: Pointer to the element
  - `ctx`: Context pointer (same as `context` parameter)
  - Returns `bool`: `true` to continue iteration, `false` to stop early

**Example:**
```truk
fn main() : i32 {
  var count: u64 = 5;
  var slice: []i32 = make(@i32, count);
  slice[0] = 1;
  slice[1] = 2;
  slice[2] = 3;
  slice[3] = 4;
  slice[4] = 5;
  
  var sum: i32 = 0;
  each(slice, &sum, fn(elem: *i32, ctx: *i32) : bool {
    *ctx = *ctx + *elem;
    return true;
  });
  
  delete(slice);
  return sum;
}
```

**Early Exit:**

Return `false` from the callback to stop iteration early:

```truk
fn main() : i32 {
  var count: u64 = 10;
  var slice: []i32 = make(@i32, count);
  
  var i: u64 = 0;
  while i < count {
    slice[i] = i as i32;
    i = i + 1;
  }
  
  var found: i32 = 0;
  each(slice, &found, fn(elem: *i32, ctx: *i32) : bool {
    if *elem == 5 {
      *ctx = *elem;
      return false;
    }
    return true;
  });
  
  delete(slice);
  return found;
}
```

**Modifying Values:**

Since the callback receives pointers, you can modify values in place:

```truk
fn main() : i32 {
  var count: u64 = 3;
  var slice: []i32 = make(@i32, count);
  slice[0] = 1;
  slice[1] = 2;
  slice[2] = 3;
  
  var unused: i32 = 0;
  each(slice, &unused, fn(elem: *i32, ctx: *i32) : bool {
    *elem = *elem * 2;
    return true;
  });
  
  var sum: i32 = slice[0] + slice[1] + slice[2];
  delete(slice);
  return sum;
}
```

**Complex Context:**

Use structs for complex context data:

```truk
struct Context {
  sum: i32,
  count: i32,
  max: i32
}

fn main() : i32 {
  var m: map[*u8, i32] = make(@map[*u8, i32]);
  m["a"] = 10;
  m["b"] = 20;
  m["c"] = 5;
  
  var ctx: Context = Context{sum: 0, count: 0, max: 0};
  each(m, &ctx, fn(key: *u8, val: *i32, context: *Context) : bool {
    (*context).sum = (*context).sum + *val;
    (*context).count = (*context).count + 1;
    if *val > (*context).max {
      (*context).max = *val;
    }
    return true;
  });
  
  delete(m);
  return ctx.sum;
}
```

**Note:** The `each` builtin uses lambdas for the callback. See [lambdas.md](lambdas.md) for more details on lambda syntax and semantics.

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
