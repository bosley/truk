[← Back to Documentation Index](../start-here.md)

# Generic Structs in truk

**Language Reference:** [Grammar](grammar.md) · [Builtins](builtins.md) · [Maps](maps.md) · [Defer](defer.md) · [Imports](imports.md) · [Lambdas](lambdas.md) · [Privacy](privacy.md) · [Runtime](runtime.md)

---

Generic structs enable type parameterization, allowing you to write reusable data structures that work with any type. Generics are implemented using compile-time monomorphization, which means zero runtime overhead.

## Syntax

```truk
struct Name<T> {
  field: T
}

struct Name<T, U> {
  first: T,
  second: U
}
```

Type parameters are specified in angle brackets after the struct name. You can have one or more type parameters.

## Basic Usage

### Single Type Parameter

```truk
struct Box<T> {
  value: T
}

fn main() : i32 {
  var b: Box<i32> = Box<i32>{value: 42};
  return b.value;
}
```

### Multiple Type Parameters

```truk
struct Pair<K, V> {
  key: K,
  val: V
}

fn main() : i32 {
  var p: Pair<i32, *u8> = Pair<i32, *u8>{key: 1, val: "hello"};
  return p.key;
}
```

## Type Instantiation

When you use a generic struct, you must provide concrete types for all type parameters:

```truk
var b1: Box<i32> = Box<i32>{value: 10};
var b2: Box<u8> = Box<u8>{value: 255};
var b3: Box<*i32> = Box<*i32>{value: &x};
```

Each unique combination of type arguments creates a distinct concrete type at compile time.

## Supported Type Arguments

Generic structs accept any valid truk type as arguments:

### Primitive Types

```truk
var b1: Box<i32> = Box<i32>{value: 42};
var b2: Box<f64> = Box<f64>{value: 3.14};
var b3: Box<bool> = Box<bool>{value: true};
```

### Pointer Types

```truk
struct Container<T> {
  data: T
}

fn main() : i32 {
  var x: i32 = 100;
  var c: Container<*i32> = Container<*i32>{data: &x};
  return *c.data;
}
```

### Array Types

```truk
struct ArrayWrapper<T> {
  items: T
}

fn main() : i32 {
  var w: ArrayWrapper<[5]i32> = ArrayWrapper<[5]i32>{
    items: [1, 2, 3, 4, 5]
  };
  return w.items[0];
}
```

### User-Defined Types

```truk
struct Point {
  x: i32,
  y: i32
}

struct Box<T> {
  value: T
}

fn main() : i32 {
  var p: Point = Point{x: 10, y: 20};
  var b: Box<Point> = Box<Point>{value: p};
  return b.value.x;
}
```

## Nested Generics

Generic structs can be nested - a generic struct can contain another generic struct:

```truk
struct Box<T> {
  value: T
}

fn main() : i32 {
  var inner: Box<i32> = Box<i32>{value: 5};
  var outer: Box<Box<i32>> = Box<Box<i32>>{value: inner};
  return outer.value.value;
}
```

**Note:** When writing nested generics like `Box<Box<i32>>`, you can add a space before the closing `>>` if preferred: `Box<Box<i32> >`. Both forms are supported.

## Multiple Instantiations

You can create multiple instances of the same generic with different type arguments in the same program:

```truk
struct Box<T> {
  value: T
}

fn main() : i32 {
  var b1: Box<i32> = Box<i32>{value: 10};
  var b2: Box<i64> = Box<i64>{value: 20};
  var b3: Box<u8> = Box<u8>{value: 30};
  
  return b1.value + b2.value as i32 + b3.value as i32;
}
```

Each instantiation (`Box<i32>`, `Box<i64>`, `Box<u8>`) generates a separate concrete struct in the compiled C code.

## Advanced Examples

### Generic Pair/Tuple

```truk
struct Pair<A, B> {
  first: A,
  second: B
}

fn main() : i32 {
  var p: Pair<i32, *u8> = Pair<i32, *u8>{
    first: 42,
    second: "answer"
  };
  return p.first;
}
```

### Generic with Three Parameters

```truk
struct Triple<A, B, C> {
  first: A,
  second: B,
  third: C
}

fn main() : i32 {
  var t: Triple<i32, i32, i32> = Triple<i32, i32, i32>{
    first: 1,
    second: 2,
    third: 3
  };
  return t.first + t.second + t.third;
}
```

### Generic Container with Pointers

```truk
struct Ref<T> {
  ptr: *T
}

fn main() : i32 {
  var x: i32 = 100;
  var r: Ref<i32> = Ref<i32>{ptr: &x};
  return *r.ptr;
}
```

## Implementation Details

### Monomorphization

truk implements generics using **monomorphization** (compile-time template expansion). When you use `Box<i32>`, the compiler generates a concrete C struct:

```c
typedef struct Box_i32 Box_i32;
struct Box_i32 {
  __truk_i32 value;
};
```

This approach provides:
- **Zero runtime overhead** - no vtables or type information at runtime
- **Type safety** - all type checking happens at compile time
- **Optimal performance** - each instantiation is a regular C struct

### Name Mangling

Generic instantiations are given unique names based on their type arguments:

| truk Type | Generated C Name |
|-----------|------------------|
| `Box<i32>` | `Box_i32` |
| `Box<*u8>` | `Box_ptr_u8` |
| `Pair<i32, *u8>` | `Pair_i32_ptr_u8` |
| `Box<Box<i32>>` | `Box_Box_i32` |
| `Container<[10]i32>` | `Container_arr10_i32` |

### Code Size

Each unique instantiation generates a separate struct definition. If you instantiate `Box<i32>` in 10 different places, only one `Box_i32` struct is generated. However, `Box<i32>` and `Box<i64>` generate two separate structs.

## Limitations

### No Generic Functions (Yet)

Currently, only structs can be generic. Generic functions are planned for a future release:

```truk
fn identity<T>(x: T) : T {
  return x;
}
```

### No Type Constraints

Type parameters accept any type. There's no way to constrain type parameters to specific traits or interfaces:

```truk
struct Box<T: Numeric> {
  value: T
}
```

### No Default Type Parameters

All type parameters must be explicitly specified:

```truk
struct Box<T = i32> {
  value: T
}
```

## Common Patterns

### Optional Value

```truk
struct Option<T> {
  has_value: bool,
  value: T
}

fn main() : i32 {
  var some: Option<i32> = Option<i32>{has_value: true, value: 42};
  var none: Option<i32> = Option<i32>{has_value: false, value: 0};
  
  if some.has_value {
    return some.value;
  }
  return 0;
}
```

### Result Type

```truk
struct Result<T, E> {
  is_ok: bool,
  value: T,
  error: E
}

fn divide(a: i32, b: i32) : Result<i32, *u8> {
  if b == 0 {
    return Result<i32, *u8>{
      is_ok: false,
      value: 0,
      error: "division by zero"
    };
  }
  return Result<i32, *u8>{
    is_ok: true,
    value: a / b,
    error: ""
  };
}

fn main() : i32 {
  var r: Result<i32, *u8> = divide(10, 2);
  if r.is_ok {
    return r.value;
  }
  return -1;
}
```

### Generic Linked List Node

```truk
struct Node<T> {
  value: T,
  next: *Node<T>
}

fn main() : i32 {
  var node1: Node<i32> = Node<i32>{value: 10, next: nil};
  var node2: Node<i32> = Node<i32>{value: 20, next: &node1};
  
  return node2.value + node2.next.value;
}
```

## Best Practices

### 1. Use Descriptive Type Parameter Names

```truk
struct Map<K, V> {
  key: K,
  value: V
}
```

Single letters like `T`, `K`, `V`, `E` are conventional for simple cases.

### 2. Keep Generic Structs Simple

Generic structs work best for simple data containers. Complex logic should be in functions that operate on the generic types.

### 3. Consider Code Size

Each instantiation generates code. If you need many different instantiations, consider whether a generic is the right approach.

### 4. Explicit Type Arguments

Always specify type arguments explicitly - type inference is not supported:

```truk
var b: Box<i32> = Box<i32>{value: 42};
```

## Error Messages

### Unknown Generic Type

```truk
var b: Foo<i32> = Foo<i32>{value: 42};
```

```
error: Unknown generic type: Foo
```

### Wrong Number of Type Arguments

```truk
struct Pair<K, V> { key: K, val: V }

var p: Pair<i32> = Pair<i32>{key: 1, val: 2};
```

```
error: Generic type 'Pair' expects 2 type arguments, got 1
```

### Non-Generic Type Used as Generic

```truk
struct Point { x: i32, y: i32 }

var p: Point<i32> = Point<i32>{x: 1, y: 2};
```

```
error: Type 'Point' is not generic
```

### Unknown Type Argument

```truk
var b: Box<UnknownType> = Box<UnknownType>{value: 42};
```

```
error: Unknown type argument: UnknownType
```

## Comparison with Other Languages

### Rust

truk generics are similar to Rust's generics - both use monomorphization:

```rust
// Rust
struct Box<T> {
    value: T
}
```

```truk
// truk
struct Box<T> {
  value: T
}
```

### Go

Go uses a hybrid approach with some runtime type information. truk uses pure monomorphization like Rust and C++.

### C++

truk generics are conceptually similar to C++ templates, but simpler - no template specialization or SFINAE.

## Performance

Generic structs have **zero runtime overhead**:
- No vtables
- No runtime type information
- No indirection
- Direct memory layout like hand-written structs

A `Box<i32>` is exactly as efficient as a manually-written struct with an `i32` field.

## Future Extensions

Planned features for future releases:

- **Generic functions:** `fn identity<T>(x: T) : T`
- **Type constraints:** `struct Box<T: Numeric>`
- **Default type parameters:** `struct Box<T = i32>`
- **Associated types:** More complex type relationships

---

[← Back to Documentation Index](../start-here.md)
