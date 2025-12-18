# truk

<p align="center">
  <img src="tools/syntax/icon.png" alt="truk" width="200"/>
</p>

truk is a statically-typed systems programming language that compiles to C. It provides manual memory management with runtime bounds checking, modern language features, and seamless C interoperability.

## Quick Start

```bash
truk run example.truk
truk compile example.truk -o program
truk test tests/
```

See [docs/getting-started/building.md](docs/getting-started/building.md) for build instructions and [docs/start-here.md](docs/start-here.md) for complete documentation.

## What Makes truk Different?

- **Systems Programming with Safety:** Manual memory management with runtime bounds checking
- **Modern Language Features:** Enums, pattern matching, lambdas, defer, tuples, and maps
- **C Interoperability:** Seamless integration with C libraries and headers
- **Fast Compilation:** Compiles to C using TCC backend with JIT execution mode
- **No Garbage Collection:** Explicit memory management with no runtime overhead
- **Built-in Testing:** Convention-based test framework with no external dependencies

## Feature Showcase

<details>
<summary>Enums and Pattern Matching</summary>

```truk
enum HttpStatus : i32 {
    OK = 200,
    NOT_FOUND = 404,
    SERVER_ERROR = 500
}

fn handle_response(status: HttpStatus) : i32 {
    match status {
        case HttpStatus.OK => return 0,
        case HttpStatus.NOT_FOUND => return 1,
        case HttpStatus.SERVER_ERROR => return 2,
        _ => return -1,
    }
}
```

</details>

<details>
<summary>Multiple Return Values and Destructuring</summary>

```truk
fn divide(a: i32, b: i32) : (bool, i32) {
    if b == 0 {
        return false, 0;
    }
    return true, a / b;
}

fn main() : i32 {
    let success, result = divide(10, 2);
    if success {
        return result;
    }
    return -1;
}
```

</details>

<details>
<summary>Defer for Automatic Cleanup</summary>

```truk
fn process_file(filename: *u8) : i32 {
    var file: *File = open_file(filename);
    defer close_file(file);
    
    var buffer: []u8 = make(@u8, 1024);
    defer delete(buffer);
    
    return 0;
}
```

</details>

<details>
<summary>Maps with Type-Safe Keys and Values</summary>

```truk
fn count_words(text: []u8) : i32 {
    var counts: map[*u8, i32] = make(@map[*u8, i32]);
    defer delete(counts);
    
    counts["hello"] = 5;
    counts["world"] = 3;
    
    var total: i32 = 0;
    each(counts, &total, fn(key: *u8, val: *i32, ctx: *i32) : bool {
        *ctx = *ctx + *val;
        return true;
    });
    
    return total;
}
```

</details>

<details>
<summary>Lambdas for Functional Programming</summary>

```truk
fn map_array(arr: []i32, f: fn(i32) : i32) : void {
    var i: u64 = 0;
    var len: u64 = len(arr);
    while i < len {
        arr[i] = f(arr[i]);
        i = i + 1;
    }
}

fn main() : i32 {
    var arr: []i32 = make(@i32, 3);
    defer delete(arr);
    
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    
    map_array(arr, fn(x: i32) : i32 {
        return x * 2;
    });
    
    return arr[0] + arr[1] + arr[2];
}
```

</details>

## Language Features

### Type System

**Primitive types:**
- Signed integers: `i8`, `i16`, `i32`, `i64`
- Unsigned integers: `u8`, `u16`, `u32`, `u64`
- Floating point: `f32`, `f64`
- Boolean: `bool`
- Character: `i8` (char literals: `'a'`, `'\n'`, `'\t'`, `'\x41'`, `'\0'`)
- Void: `void`

<details>
<summary>Character literal examples</summary>

```truk
fn classify_char(c: i8) : i32 {
    match c {
        case 'a' => return 1,
        case 'b' => return 2,
        case '\n' => return 10,
        case '\t' => return 9,
        case '\0' => return 0,
        case '\x41' => return 65,
        _ => return -1,
    }
}

fn main() : i32 {
    var letter: i8 = 'A';
    var newline: i8 = '\n';
    var hex: i8 = '\x48';
    
    return classify_char(letter);
}
```

Character literals are `i8` type and support escape sequences: `\n`, `\t`, `\r`, `\0`, `\\`, `\'`, `\"`, and hex codes `\xHH`.

</details>

**Arrays:**
- Sized arrays: `[10]i32` (stack allocated)
- Unsized arrays: `[]i32` (heap allocated slices)
- Multi-dimensional: `[5][10]i32`

**Maps:**
- Hash tables with typed keys and values: `map[K, V]`
- Key types: primitives (i8-i64, u8-u64, f32, f64, bool) or string pointers (*u8, *i8)
- Indexing returns pointer: `m["key"]` returns `*V` (nil if key doesn't exist)
- Nil checking: `if m["key"] != nil { ... }`

**Pointers:**
- Single: `*i32`
- Multiple indirection: `**i32`

**User-defined types:**
- Structs with named fields
- Enums with typed values

**Tuples:**
- Multiple return values: `(i32, i32)`
- Destructuring with `let`: `let x, y = get_coords();`

### Control Flow

- `if`/`else if`/`else` statements
- `while` loops
- `for` loops with C-style syntax
- `match` expressions for pattern matching
- `break` and `continue`
- `return` statements
- `defer` for cleanup operations

### Enums

Type-safe enumerations with explicit underlying types:

<details>
<summary>Click to see enum examples</summary>

```truk
enum Status : i32 {
    OK = 0,
    ERROR = 1,
    PENDING = 2
}

enum Color : u8 {
    RED = 0xFF0000,
    GREEN = 0x00FF00,
    BLUE = 0x0000FF
}

fn main() : i32 {
    var status: Status = Status.OK;
    var color: Color = Color.RED;
    
    if status == Status.OK {
        return 0;
    }
    return 1;
}
```

Enums can be cast to their underlying type and used in expressions, function parameters, struct fields, and more.

</details>

### Match Expressions

Pattern matching for clean control flow:

<details>
<summary>Click to see match examples</summary>

```truk
enum Status : i32 { OK = 0, ERROR = 1, PENDING = 2 }

fn handle_status(s: Status) : i32 {
    match s {
        case Status.OK => return 0,
        case Status.ERROR => {
            return 1;
        },
        case Status.PENDING => return 2,
        _ => return -1,
    }
}

fn classify_number(x: i32) : i32 {
    match x {
        case 0 => return 0,
        case 1 => return 1,
        case 42 => return 42,
        _ => return -1,
    }
}

fn main() : i32 {
    var c: i8 = 'a';
    match c {
        case 'a' => return 1,
        case 'b' => return 2,
        _ => return 0,
    }
}
```

Match expressions support integers, characters, booleans, enums, and pointers. The wildcard `_` case is required.

</details>

### Memory Management

Manual memory management with explicit allocation and deallocation:

<details>
<summary>Click to see memory management examples</summary>

```truk
var ptr: *i32 = make(@i32);
*ptr = 42;
delete(ptr);

var count: u64 = 100;
var arr: []i32 = make(@i32, count);
arr[0] = 10;
delete(arr);

var m: map[*u8, i32] = make(@map[*u8, i32]);
m["key"] = 42;
delete(m);
```

Runtime bounds checking on all array accesses. Out-of-bounds access causes a panic.

</details>

### Lambdas and Defer

First-class functions and scope-based cleanup:

<details>
<summary>Click to see lambda and defer examples</summary>

```truk
fn process(x: i32, op: fn(i32) : i32) : i32 {
    return op(x);
}

fn main() : i32 {
    var result: i32 = process(10, fn(x: i32) : i32 {
        return x * 2;
    });
    
    var ptr: *i32 = make(@i32);
    defer delete(ptr);
    
    *ptr = result;
    return *ptr;
}

fn with_map() : i32 {
    var m: map[*u8, i32] = make(@map[*u8, i32]);
    defer delete(m);
    
    m["count"] = 0;
    
    each(m, nil, fn(key: *u8, val: *i32, ctx: *void) : bool {
        *val = *val + 1;
        return true;
    });
    
    var ptr: *i32 = m["count"];
    if ptr != nil {
        return *ptr;
    }
    return 0;
}
```

Lambdas are non-capturing and compile to static functions. Defer executes cleanup code when exiting the current scope.

</details>

### Builtin Functions

<details>
<summary>Click to see builtin functions</summary>

**Memory Management:**
- `make(@type)` - allocate single value on heap
- `make(@type, count)` - allocate array on heap
- `make(@map[K, V])` - allocate and initialize map
- `delete(ptr)` - free allocated memory (single value, array, or map)
- `delete(m[key])` - remove key-value pair from map

**Array Operations:**
- `len(arr)` - get array length

**Type Information:**
- `sizeof(@type)` - get type size in bytes

**Iteration:**
- `each(collection, context, callback)` - iterate over maps or slices

**Error Handling:**
- `panic(message: []u8)` - abort with error message

Type parameters use `@` prefix syntax to pass types to builtins.

</details>

### Complete Example

<details>
<summary>Click to see a complete program demonstrating key features</summary>

```truk
enum Status : i32 {
    SUCCESS = 0,
    NOT_FOUND = 1,
    ERROR = 2
}

struct Point {
    x: i32,
    y: i32
}

fn distance_squared(p1: Point, p2: Point) : i32 {
    var dx: i32 = p2.x - p1.x;
    var dy: i32 = p2.y - p1.y;
    return dx * dx + dy * dy;
}

fn find_point(points: []Point, target_x: i32) : (Status, Point) {
    var i: u64 = 0;
    var len: u64 = len(points);
    
    while i < len {
        if points[i].x == target_x {
            return Status.SUCCESS, points[i];
        }
        i = i + 1;
    }
    
    return Status.NOT_FOUND, Point{x: 0, y: 0};
}

fn main() : i32 {
    var count: u64 = 3;
    var points: []Point = make(@Point, count);
    defer delete(points);
    
    points[0] = Point{x: 0, y: 0};
    points[1] = Point{x: 3, y: 4};
    points[2] = Point{x: 6, y: 8};
    
    var cache: map[*u8, i32] = make(@map[*u8, i32]);
    defer delete(cache);
    
    cache["first"] = distance_squared(points[0], points[1]);
    cache["second"] = distance_squared(points[1], points[2]);
    
    let status, point = find_point(points, 3);
    
    match status {
        case Status.SUCCESS => {
            var ptr: *i32 = cache["first"];
            if ptr != nil {
                return *ptr;
            }
            return 0;
        },
        case Status.NOT_FOUND => return 1,
        _ => return 2,
    }
}
```

This example demonstrates:
- Enum definitions and usage
- Struct definitions
- Array allocation with `make`
- Defer for automatic cleanup
- Map creation and indexing
- Multiple return values with tuples
- Let destructuring
- Match expressions
- Proper memory management

</details>

### C Interoperability

truk provides seamless interoperability with C through `cimport` and `extern` declarations.

<details>
<summary>Click to see C interop examples</summary>

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

</details>

### Privacy System

Convention-based privacy with file and shard scoping:

<details>
<summary>Click to see privacy examples</summary>

```truk
struct Connection {
    host: *u8,
    port: u16,
    _socket_fd: i32,
    _is_connected: bool
}

fn _internal_helper(x: i32) : i32 {
    return x * 2;
}

fn public_api(x: i32) : i32 {
    return _internal_helper(x);
}
```

Identifiers starting with `_` are private to their file. Files can declare shards to share private members:

```truk
shard "database_internal";

fn _shared_internal() : void {
}
```

See [docs/language/privacy.md](docs/language/privacy.md) for details.

</details>

### Testing

Built-in testing framework with convention-based test discovery:

<details>
<summary>Click to see testing examples</summary>

```truk
extern struct __truk_test_context_s;
extern fn __truk_test_assert_i32(t: *__truk_test_context_s, 
                                 expected: i32, actual: i32, 
                                 msg: *u8) : void;

fn add(a: i32, b: i32) : i32 {
    return a + b;
}

fn test_addition(t: *__truk_test_context_s) : void {
    __truk_test_assert_i32(t, 4, add(2, 2), "2+2 should equal 4");
    __truk_test_assert_i32(t, 10, add(7, 3), "7+3 should equal 10");
}

fn test_setup(t: *__truk_test_context_s) : void {
}

fn test_teardown(t: *__truk_test_context_s) : void {
}
```

Run tests with:
```bash
truk test math.truk
truk test tests/
```

See [docs/language/testing.md](docs/language/testing.md) for the complete testing API.

</details>

## Compilation

truk compiles to C and uses TCC (Tiny C Compiler) internally as the backend. The compiler performs type checking and validation before emitting C code.

**Compilation modes:**
- `truk run` - JIT compile and execute
- `truk compile` - Compile to executable
- `truk toc` - Transpile to C source files
- `truk test` - Run tests

## Documentation

**[📚 Start Here - Complete Documentation Guide](docs/start-here.md)**

The documentation is organized into three sections:

### Getting Started
- [Building truk Programs](docs/getting-started/building.md) - Compilation commands and workflows

### Language Reference
- [Grammar](docs/language/grammar.md) - Complete language syntax
- [Builtin Functions](docs/language/builtins.md) - Memory management, arrays, type operations
- [Maps](docs/language/maps.md) - Hash table types and operations
- [Defer Statements](docs/language/defer.md) - Scope-based cleanup
- [Imports](docs/language/imports.md) - Module system and C interoperability
- [Lambdas](docs/language/lambdas.md) - First-class functions and callbacks
- [Privacy](docs/language/privacy.md) - File and shard-based privacy system
- [Testing](docs/language/testing.md) - Built-in test framework
- [Runtime Architecture](docs/language/runtime.md) - How truk programs execute

### Compiler Internals
- [Error Handling](docs/compiler-internals/error-handling.md) - Unified error reporting system
- [Error Flow Diagrams](docs/compiler-internals/error-flow-diagram.md) - Error flow through compilation stages
- [C Emitter](docs/compiler-internals/emitter.md) - Code generation architecture
- [Type Checker](docs/compiler-internals/typechecker.md) - Type validation and semantic analysis

Visit [docs/start-here.md](docs/start-here.md) for the full documentation index with navigation and examples.

## Key Language Features at a Glance

| Feature | Description | Example |
|---------|-------------|---------|
| **Enums** | Type-safe enumerations with explicit types | `enum Status : i32 { OK = 0 }` |
| **Match** | Pattern matching for control flow | `match x { case 0 => ..., _ => ... }` |
| **Tuples** | Multiple return values | `fn f() : (i32, i32) { return 1, 2; }` |
| **Let** | Tuple destructuring | `let x, y = get_coords();` |
| **Defer** | Scope-based cleanup | `defer delete(ptr);` |
| **Lambdas** | First-class functions | `fn(x: i32) : i32 { return x * 2; }` |
| **Maps** | Built-in hash tables | `map[*u8, i32]` |
| **Each** | Iterator for maps and slices | `each(m, ctx, callback)` |
| **Char Literals** | Character constants with escapes | `'a'`, `'\n'`, `'\x41'` |
| **Privacy** | Convention-based with `_` prefix | `_private_field`, `_private_fn()` |
| **Shards** | Shared privacy boundaries | `shard "internal";` |
| **Testing** | Built-in test framework | `fn test_foo(t: *__truk_test_context_s)` |
| **C Interop** | Seamless C integration | `cimport <stdio.h>; extern fn printf(...)` |

## Memory Model

Stack allocation for local variables with sized arrays and structs. Heap allocation through explicit `make` calls. No garbage collection. Memory must be manually freed with `delete`. Double-free and use-after-free result in undefined behavior.

## Contributing

truk is actively developed. See the [compiler internals documentation](docs/compiler-internals/) to understand the implementation, and check [todo.md](todo.md) for planned features and improvements.

## License

See the repository for license information.
