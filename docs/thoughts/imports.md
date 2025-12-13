# Imports

Truk provides two import mechanisms: importing C functions via `@cimport`, and importing other Truk files via `import`.

## C Function Imports

Use `@cimport("header.h")` to declare external C functions. The compiler will emit `#include` directives for the specified headers.

```truk
@cimport("stdio.h")
fn printf(fmt: *u8, ...) : i32;

@cimport("stdlib.h")
fn malloc(size: u64) : *void;

@cimport("stdlib.h")
fn free(ptr: *void) : void;

@cimport("math.h")
fn sin(x: f64) : f64;

fn main() : i32 {
    var ptr: *i32 = malloc(4) as *i32;
    defer free(ptr as *void);
    
    *ptr = 42;
    printf("Value: %d, sin(1.0) = %f\n", *ptr, sin(1.0));
    
    return 0;
}
```

The `@cimport` attribute must appear on function declarations (no body). The compiler collects all unique headers and emits them at the top of the generated C file.

### Opaque Types

C structs can be declared as opaque types:

```truk
@cimport("stdio.h")
struct FILE;

@cimport("stdio.h")
fn fopen(path: *u8, mode: *u8) : *FILE;

@cimport("stdio.h")
fn fclose(file: *FILE) : i32;

fn main() : i32 {
    var f: *FILE = fopen("test.txt", "r");
    defer fclose(f);
    return 0;
}
```

### Variadic Functions

Variadic functions use `...` syntax:

```truk
@cimport("stdio.h")
fn printf(fmt: *u8, ...) : i32;
```

## Truk File Imports

Use `import "file.truk";` to import declarations from other Truk files. All top-level declarations (functions, structs, globals, constants) are added to the current scope.

```truk
// math.truk
fn add(a: i32, b: i32) : i32 {
    return a + b;
}

fn multiply(a: i32, b: i32) : i32 {
    return a * b;
}

// main.truk
import "math.truk";

fn main() : i32 {
    return add(1, multiply(2, 3));
}
```

Imported declarations are placed in the global scope. There are no namespaces. If two imported files define the same symbol, a compile error occurs.

### Import Paths

Import paths are relative to the file containing the import statement:

```truk
import "utils.truk";
import "lib/math.truk";
import "../common/types.truk";
```

### Circular Imports

Circular imports are detected and result in a compile error:

```truk
// a.truk
import "b.truk";

// b.truk
import "a.truk";  // Error: circular import
```

### Code Generation

All imported Truk files are compiled into a single C file. The compiler parses all imports recursively, collects all declarations, and emits them in dependency order.

## Complete Example

```truk
// vec.truk
@cimport("math.h")
fn sqrtf(x: f32) : f32;

struct Vec2 {
    x: f32,
    y: f32
}

fn vec_add(a: Vec2, b: Vec2) : Vec2 {
    return Vec2{x: a.x + b.x, y: a.y + b.y};
}

fn vec_length(v: Vec2) : f32 {
    return sqrtf(v.x * v.x + v.y * v.y);
}

// main.truk
import "vec.truk";

@cimport("stdio.h")
fn printf(fmt: *u8, ...) : i32;

fn main() : i32 {
    var a: Vec2 = Vec2{x: 3.0, y: 4.0};
    var len: f32 = vec_length(a);
    printf("Length: %f\n", len);
    return 0;
}
```

Generated C includes both `math.h` and `stdio.h`, and all Truk code is compiled into a single translation unit.
