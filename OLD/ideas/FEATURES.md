# Truk Language Features Roadmap

This document tracks planned builtin functions and language features for truk.

## Planned Builtins

### Memory & Type Introspection

#### `alignof(@type) -> u64`
Get the alignment requirement of a type in bytes. Companion to `sizeof`, essential for custom allocators and FFI.

**Example:**
```truk
var align: u64 = alignof(@MyStruct);
var ptr_align: u64 = alignof(@*i32);
```

**Use Cases:**
- Custom memory allocators
- FFI with C libraries requiring specific alignment
- Packed vs aligned structure layouts

---

#### `offsetof(@StructType, field_name) -> u64`
Get the byte offset of a field within a struct. Critical for unsafe code, FFI, and serialization.

**Example:**
```truk
struct Point {
    x: i32,
    y: i32,
    z: i32
}

var x_offset: u64 = offsetof(@Point, x);
var z_offset: u64 = offsetof(@Point, z);
```

**Use Cases:**
- FFI interop with C structs
- Custom serialization/deserialization
- Memory layout debugging
- Offset-based field access in unsafe code

---

#### `type_id(@type) -> u64`
Get a unique identifier for a type. Foundation for runtime type information.

**Example:**
```truk
var id: u64 = type_id(@*MyStruct);
var same: bool = type_id(@i32) == type_id(@i32);
```

**Use Cases:**
- Type erasure patterns
- Dynamic dispatch mechanisms
- Reflection capabilities
- Runtime type checking
- Generic container implementations

---

### Array/Memory Operations

#### `memcpy(dst: *void, src: *void, size: u64) -> void`
Bulk memory copy operation. Can be optimized to platform-specific SIMD instructions.

**Example:**
```truk
var src: [10]i32 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
var dst: [10]i32;
memcpy(&dst as *void, &src as *void, sizeof(@[10]i32));
```

**Benefits:**
- Controlled optimization opportunities
- Hooks for bounds checking in debug builds
- Future instrumentation for memory tracking

---

#### `memset(ptr: *void, value: u8, size: u64) -> void`
Initialize a block of memory to a specific byte value.

**Example:**
```truk
var buffer: [100]u8;
memset(&buffer as *void, 0, sizeof(@[100]u8));
```

**Benefits:**
- Platform-optimized implementations
- Memory safety hooks
- Debug mode tracking

---

#### `array_copy(dst: []T, src: []T) -> void`
Type-safe array copy using slice metadata. Automatically bounds-checks using the slice `len` field.

**Example:**
```truk
var src: []i32 = make(@i32, 10);
var dst: []i32 = make(@i32, 10);
array_copy(dst, src);
```

**Benefits:**
- Cannot accidentally copy wrong size (unlike raw memcpy)
- Leverages slice metadata for safety
- Type-safe at compile time

---

### Bit Manipulation

These operations map to single CPU instructions on most architectures, providing efficient low-level bit operations.

#### `popcount(value: u64) -> u32`
Count the number of set bits (1s) in a value.

**Example:**
```truk
var bits: u32 = popcount(0b10110011);
```

**Use Cases:**
- Bitmap operations
- Hash functions
- Compression algorithms

---

#### `clz(value: u64) -> u32`
Count leading zeros - number of zero bits before the first set bit from the most significant bit.

**Example:**
```truk
var leading: u32 = clz(0b00001000);
```

**Use Cases:**
- Finding highest set bit
- Integer log2 calculations
- Normalization operations

---

#### `ctz(value: u64) -> u32`
Count trailing zeros - number of zero bits after the last set bit from the least significant bit.

**Example:**
```truk
var trailing: u32 = ctz(0b10110000);
```

**Use Cases:**
- Finding lowest set bit
- Power-of-2 detection
- Bit scan operations

---

#### `bswap(value: u64) -> u64`
Byte swap for endianness conversion (big-endian ↔ little-endian).

**Example:**
```truk
var big_endian: u64 = bswap(little_endian_value);
```

**Use Cases:**
- Network protocol implementations
- File format parsing
- Cross-platform binary data

---

### Atomic Operations

Essential primitives for concurrent programming and the future runtime. All operations are lock-free on supporting architectures.

#### `atomic_load(@type, ptr: *T) -> T`
Atomically load a value from memory with acquire semantics.

**Example:**
```truk
var ptr: *i32 = make(@i32);
var value: i32 = atomic_load(@i32, ptr);
```

---

#### `atomic_store(@type, ptr: *T, value: T) -> void`
Atomically store a value to memory with release semantics.

**Example:**
```truk
var ptr: *i32 = make(@i32);
atomic_store(@i32, ptr, 42);
```

---

#### `atomic_add(@type, ptr: *T, value: T) -> T`
Atomically add to a value and return the old value.

**Example:**
```truk
var counter: *i32 = make(@i32);
var old: i32 = atomic_add(@i32, counter, 1);
```

---

#### `atomic_compare_exchange(@type, ptr: *T, expected: T, desired: T) -> bool`
Atomically compare and exchange (CAS operation). Returns true if exchange occurred.

**Example:**
```truk
var ptr: *i32 = make(@i32);
*ptr = 10;
var success: bool = atomic_compare_exchange(@i32, ptr, 10, 20);
```

**Use Cases:**
- Lock-free data structures
- Reference counting
- Spin locks and mutexes
- Thread synchronization

---

## Priority Recommendations

The following builtins provide the most immediate value and should be implemented first:

1. **`alignof`** - Pairs with existing `sizeof`, needed for custom allocators and FFI work
2. **`offsetof`** - Critical for FFI interop and low-level struct manipulation
3. **`type_id`** - Foundation for future reflection and runtime type information features
4. **`memcpy`/`memset`** - Common operations that can be optimized with platform-specific implementations
5. **Bit manipulation suite** - Efficient primitives for systems programming, all map to single instructions
6. **`array_copy`** - Safer alternative to raw memcpy for slice types

## Design Principles

All builtin functions follow these principles:

1. **Type Safety** - Builtins use truk's type system (e.g., `@type` parameters) rather than unsafe C patterns
2. **Optimization Hooks** - Builtins can be replaced with optimized implementations without changing user code
3. **Future Flexibility** - Builtins provide abstraction points for runtime features, instrumentation, and custom allocators
4. **Safety by Default** - Bounds checking and validation in debug builds, performance in release builds
5. **Platform Portability** - Builtins abstract over platform-specific implementations
