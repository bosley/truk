# Maps in truk

Maps are hash tables with string keys and values of any type. They provide O(1) average-case lookup, insertion, and deletion.

## Syntax

```truk
map[V]
```

Where `V` is the value type. Keys are always string-like types.

## Basic Usage

### Creating a Map

```truk
var m: map[i32] = make(@map[i32]);
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

Maps accept **string-like keys**:

### String Literals

```truk
m["literal"] = 42;
```

String literals are `*u8` type.

### String Variables

```truk
var key1: *u8 = "hello";
var key2: *i8 = "world";
m[key1] = 1;
m[key2] = 2;
```

Both `*u8` and `*i8` work as keys.

### Slice Keys

```truk
var size: u64 = 10;
var key: []u8 = make(@u8, size);
m[key] = 42;
delete(key);
```

Slices (`[]u8`, `[]i8`) automatically use their `.data` field as the C string pointer.

## Value Types

Maps can store any type as values.

### Primitives

```truk
var m1: map[i32] = make(@map[i32]);
var m2: map[f64] = make(@map[f64]);
var m3: map[bool] = make(@map[bool]);

m1["count"] = 42;
m2["pi"] = 3.14;
m3["flag"] = true;
```

### Pointers

```truk
var m: map[*i32] = make(@map[*i32]);

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

var m: map[Point] = make(@map[Point]);

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
var m: map[Point] = make(@map[Point]);

var stack_point: Point = Point{x: 10, y: 20};
m["key"] = stack_point;

delete(m);
```

**Safe!** The map copied `stack_point`'s bytes. Deleting the map frees the copy, not the original.

#### Modifications Don't Affect Original

```truk
var m: map[Point] = make(@map[Point]);

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
var m: map[*Point] = make(@map[*Point]);

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
var m: map[*i32] = make(@map[*i32]);

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
var m: map[i32] = make(@map[i32]);

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
var m: map[i32] = make(@map[i32]);

var ptr: *i32 = m["key"];
if ptr == nil {
  m["key"] = 42;
}
```

### Accumulator Pattern

```truk
var m: map[i32] = make(@map[i32]);

m["total"] = 0;

var ptr: *i32 = m["total"];
if ptr != nil {
  *ptr = *ptr + 10;
}
```

### Multiple Maps

```truk
var cache: map[i32] = make(@map[i32]);
var index: map[*Data] = make(@map[*Data]);

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

var m: map[Container] = make(@map[Container]);

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

var m: map[Outer] = make(@map[Outer]);

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

### String Keys Only

Keys must be string-like types:
- `*i8` - C string pointer (signed char)
- `*u8` - C string pointer (unsigned char)
- `[]i8` - Byte slice (uses `.data` field)
- `[]u8` - Byte slice (uses `.data` field)

Integer keys, struct keys, and other types are not supported.

### No Iteration

Currently, there is no built-in way to iterate over map keys or values. This is a future enhancement.

### Key Removal

You can remove individual keys from a map using `delete` on the indexed value:

```truk
var m: map[i32] = make(@map[i32]);

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
| Syntax | `map[string]int` | `map[i32]` |
| Keys | Any comparable type | String-like only |
| Indexing | Returns value + ok | Returns `*V` (nil if missing) |
| Creation | `make(map[K]V)` | `make(@map[V])` |
| Deletion | `delete(m, key)` | No key removal |
| Iteration | `for k, v := range m` | Not supported |
| Memory | Garbage collected | Manual with `delete(m)` |

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
var m: map[*Data] = make(@map[*Data]);

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
var m: map[*Point] = make(@map[*Point]);
m["shared"] = shared;

delete(shared);
delete(m);
```

### 4. Pair make with delete

```truk
var m: map[i32] = make(@map[i32]);
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
  var users: map[User] = make(@map[User]);
  
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
