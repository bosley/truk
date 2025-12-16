# Maps in truk

Maps are hash tables with typed keys and values. They provide O(1) average-case lookup, insertion, and deletion.

## Syntax

```truk
map[K, V]
```

Where `K` is the key type and `V` is the value type. Keys can be primitives (integers, floats, bool) or string pointers (*u8, *i8).

## Basic Usage

### Creating a Map

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
```

### Inserting Values

```truk
m["key"] = 42;
m["another"] = 100;
```

### Accessing Values

Map indexing returns a **pointer to the value** (`*V`), which is `nil` if the key doesn't exist:

```truk
var ptr: *i32 = m["key"];
if ptr != nil {
  var value: i32 = *ptr;
}
```

### Checking if Key Exists

```truk
var ptr: *i32 = m["missing"];
if ptr == nil {
  // Key doesn't exist
}
```

### Updating Values

You can update values in two ways:

**Direct assignment:**
```truk
m["key"] = 50;
```

**Through pointer:**
```truk
var ptr: *i32 = m["key"];
if ptr != nil {
  *ptr = 50;
}
```

### Deleting a Map

```truk
delete(m);
```

This frees all internal map structures. See Memory Model section for details on value ownership.

## Key Types

Maps support multiple key types:

### Primitive Keys

**Integers:**
```truk
var m1: map[i32, *u8] = make(@map[i32, *u8]);
m1[42] = "answer";

var m2: map[u64, bool] = make(@map[u64, bool]);
m2[12345] = true;
```

**Floats:**
```truk
var m: map[f32, i32] = make(@map[f32, i32]);
m[3.14] = 100;
```

**Booleans:**
```truk
var m: map[bool, *u8] = make(@map[bool, *u8]);
m[true] = "yes";
m[false] = "no";
```

### String Keys

**String Literals:**
```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
m["literal"] = 42;
```

String literals are `*u8` type.

**String Variables:**
```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
var key1: *u8 = "hello";
var key2: *i8 = "world";
m[key1] = 1;
```

Both `*u8` and `*i8` work as keys.

**Slice Keys:**
```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
var size: u64 = 10;
var key: []u8 = make(@u8, size);
m[key] = 42;
delete(key);
```

Slices (`[]u8`, `[]i8`) automatically convert to string pointers.

### Key Type Examples

**Integer keys for lookup tables:**
```truk
var status_codes: map[i32, *u8] = make(@map[i32, *u8]);
status_codes[200] = "OK";
status_codes[404] = "Not Found";
status_codes[500] = "Internal Server Error";
```

**Float keys for scientific data:**
```truk
var measurements: map[f64, i32] = make(@map[f64, i32]);
measurements[98.6] = 1;
measurements[99.1] = 2;
```

**Boolean keys for binary flags:**
```truk
var settings: map[bool, *u8] = make(@map[bool, *u8]);
settings[true] = "enabled";
settings[false] = "disabled";
```

## Value Types

Maps can store any type as values.

### Primitives

```truk
var m1: map[*u8, i32] = make(@map[*u8, i32]);
var m2: map[*u8, f64] = make(@map[*u8, f64]);
var m3: map[*u8, bool] = make(@map[*u8, bool]);

m1["count"] = 42;
m2["pi"] = 3.14;
m3["flag"] = true;
```

### Pointers

```truk
var m: map[*u8, *i32] = make(@map[*u8, *i32]);

var x: i32 = 100;
m["ptr"] = &x;

var ptr: **i32 = m["ptr"];
if ptr != nil {
  if *ptr != nil {
    var value: i32 = **ptr;
  }
}
```

### Structs

```truk
struct Point {
  x: i32,
  y: i32
}

var m: map[*u8, Point] = make(@map[*u8, Point]);

var p: Point = Point{x: 10, y: 20};
m["origin"] = p;

var ptr: *Point = m["origin"];
if ptr != nil {
  var x: i32 = (*ptr).x;
  var y: i32 = (*ptr).y;
}
```

## Memory Model

### Value Storage: Copy Semantics

**Critical:** Maps store values **by copy** using `memcpy`. When you insert a value, the map copies its bytes into internal storage.

#### Stack Values are Safe

```truk
var m: map[*u8, Point] = make(@map[*u8, Point]);

var stack_point: Point = Point{x: 10, y: 20};
m["key"] = stack_point;

delete(m);
```

**Safe!** The map copied `stack_point`'s bytes. Deleting the map frees the copy, not the original.

#### Modifications Don't Affect Original

```truk
var m: map[*u8, Point] = make(@map[*u8, Point]);

var original: Point = Point{x: 10, y: 20};
m["key"] = original;

original.x = 99;

var ptr: *Point = m["key"];
```

The map's copy still has `x = 10`. Modifying `original` doesn't affect the map.

### Pointer Values: Manual Management Required

When storing pointers, **the map copies the pointer value** (the address), not what it points to.

#### Correct Pattern: Manual Cleanup

```truk
var m: map[*u8, *Point] = make(@map[*u8, *Point]);

m["data"] = make(@Point);

var ptr: **Point = m["data"];
if ptr != nil {
  delete(*ptr);
}

delete(m);
```

**You must free pointed-to data before deleting the map!**

#### Memory Leak Example

```truk
var m: map[*u8, *i32] = make(@map[*u8, *i32]);

m["key"] = make(@i32);

delete(m);
```

**MEMORY LEAK!** The `i32` allocation is never freed. The map only frees its internal structures, not the pointed-to data.

### What `delete(map)` Frees

When you call `delete(m)`, the map implementation:

1. ✓ Frees all hash table nodes
2. ✓ Frees the bucket array
3. ✓ Frees copied value data (stored inline in nodes)
4. ✗ Does NOT free data that values point to

**Rule:** If your map values contain pointers to heap-allocated data, you must free that data yourself.

## Modifying Values Through Pointers

Since map indexing returns a pointer, you can modify values in place:

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);

m["counter"] = 0;

var ptr: *i32 = m["counter"];
if ptr != nil {
  *ptr = *ptr + 1;
}

ptr = m["counter"];
if ptr != nil {
  *ptr = *ptr + 1;
}
```

The modifications persist because you're modifying the map's internal storage through the returned pointer.

## Common Patterns

### Check-Then-Insert

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);

var ptr: *i32 = m["key"];
if ptr == nil {
  m["key"] = 42;
}
```

### Accumulator Pattern

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);

m["total"] = 0;

var ptr: *i32 = m["total"];
if ptr != nil {
  *ptr = *ptr + 10;
}
```

### Multiple Maps

```truk
var cache: map[*u8, i32] = make(@map[*u8, i32]);
var index: map[*u8, *Data] = make(@map[*u8, *Data]);

cache["count"] = 42;
index["user"] = make(@Data);

var data_ptr: **Data = index["user"];
if data_ptr != nil {
  delete(*data_ptr);
}

delete(cache);
delete(index);
```

## Nested Structures

### Structs with Pointers

```truk
struct Container {
  data: *i32,
  size: i32
}

var m: map[*u8, Container] = make(@map[*u8, Container]);

var heap_data: *i32 = make(@i32);
*heap_data = 100;

var container: Container = Container{data: heap_data, size: 1};
m["item"] = container;

var ptr: *Container = m["item"];
if ptr != nil {
  if (*ptr).data != nil {
    delete((*ptr).data);
  }
}

delete(m);
```

**Important:** The map copies the `Container` struct, including the pointer value. You must free `data` before deleting the map.

### Nested Structs

```truk
struct Inner {
  value: i32
}

struct Outer {
  inner: Inner,
  extra: i32
}

var m: map[*u8, Outer] = make(@map[*u8, Outer]);

var inner: Inner = Inner{value: 10};
var outer: Outer = Outer{inner: inner, extra: 20};

m["data"] = outer;

var ptr: *Outer = m["data"];
if ptr != nil {
  var total: i32 = (*ptr).inner.value + (*ptr).extra;
}

delete(m);
```

The entire nested structure is copied into the map.

## Performance Characteristics

- **Lookup**: O(1) average case
- **Insert**: O(1) average case (amortized)
- **Delete key**: O(1) average case
- **Memory**: O(n) where n is number of entries
- **Resize**: Automatic when load factor exceeds 1.0 (doubles bucket count)

## Limitations

### Supported Key Types

Maps support the following key types:
- **Primitives**: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool
- **String pointers**: *i8, *u8
- **String slices**: []i8, []u8 (automatically converted to string pointers)

Struct keys, array keys, and other complex types are not supported.

### Iteration with each

Maps can be iterated using the `each` builtin. See [each builtin documentation](builtins.md#each) for full details.

**Example with string keys:**
```truk
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
```

**Example with integer keys:**
```truk
var m: map[i32, i32] = make(@map[i32, i32]);
m[1] = 10;
m[2] = 20;

var sum: i32 = 0;
each(m, &sum, fn(key: i32, val: *i32, ctx: *i32) : bool {
  *ctx = *ctx + *val;
  return true;
});

delete(m);
```

The callback receives:
- `key`: The key (type matches the map's key type K)
- `val`: Pointer to the value (*V)
- `ctx`: Context pointer for passing state
- Returns `bool`: `true` to continue, `false` to stop early

### Key Removal

You can remove individual keys from a map using `delete` on the indexed value:

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);

m["key"] = 42;
delete(m["key"]);

var ptr: *i32 = m["key"];
if ptr == nil {
  // Key was removed
}
```

This frees the map entry for that key.

## Comparison with Go

truk maps are similar to Go maps with some differences:

| Feature | Go | truk |
|---------|-----|------|
| Syntax | `map[K]V` | `map[K, V]` |
| Keys | Any comparable type | Primitives + string pointers |
| Indexing | Returns value + ok | Returns `*V` (nil if missing) |
| Creation | `make(map[K]V)` | `make(@map[K, V])` |
| Deletion | `delete(m, key)` | `delete(m["key"])` for keys, `delete(m)` for map |
| Iteration | `for k, v := range m` | `each(m, ctx, callback)` |
| Memory | Garbage collected | Manual with `delete(m)` |

Examples: `map[*u8, i32]` (string keys), `map[i32, i32]` (integer keys), `map[bool, *u8]` (boolean keys)

## Best Practices

### 1. Always Check for Nil

```truk
var ptr: *i32 = m["key"];
if ptr != nil {
  var value: i32 = *ptr;
}
```

### 2. Free Pointed-To Data

```truk
var m: map[*u8, *Data] = make(@map[*u8, *Data]);

m["item"] = make(@Data);

var ptr: **Data = m["item"];
if ptr != nil {
  delete(*ptr);
}

delete(m);
```

### 3. Use Copy Semantics Wisely

If you need to share data between map and other code, use pointers as values:

```truk
var shared: *Point = make(@Point);
var m: map[*u8, *Point] = make(@map[*u8, *Point]);
m["shared"] = shared;

delete(shared);
delete(m);
```

### 4. Pair make with delete

```truk
var m: map[*u8, i32] = make(@map[*u8, i32]);
defer delete(m);
```

Using `defer` ensures the map is deleted even if you return early.

## Complete Example

```truk
struct User {
  id: i32,
  score: i32
}

fn main() : i32 {
  var users: map[*u8, User] = make(@map[*u8, User]);
  
  var alice: User = User{id: 1, score: 95};
  var bob: User = User{id: 2, score: 87};
  
  users["alice"] = alice;
  users["bob"] = bob;
  
  var alice_ptr: *User = users["alice"];
  var bob_ptr: *User = users["bob"];
  
  var total: i32 = 0;
  if alice_ptr != nil {
    total = total + (*alice_ptr).score;
  }
  if bob_ptr != nil {
    total = total + (*bob_ptr).score;
  }
  
  delete(users);
  return total;
}
```

This example demonstrates:
- Creating a map with struct values
- Inserting multiple entries
- Accessing multiple keys
- Proper cleanup
