# Imports in Truk

Truk supports two types of imports: Truk module imports and C header imports.

## Truk Module Imports

Import other Truk source files using `import`:

```truk
import "math.truk";
import "utils.truk";

fn main(): i32 {
    var result: i32 = add(5, 10);
    return result;
}
```

Truk imports are:
- Recursive: imported files are fully parsed and type-checked
- Relative: paths are resolved relative to the importing file
- Deduplicated: each file is processed once, even if imported multiple times

## C Header Imports

Import C headers using `cimport` with angle brackets or quotes:

```truk
cimport <stdio.h>;
cimport <math.h>;
cimport "myheader.h";
```

**Angle brackets `<>`**: System headers (standard library)
**Quotes `""`**: Local headers (use `-I` flag to specify include paths)

C imports are:
- Pass-through: headers are included in generated C code, not parsed by Truk
- Non-recursive: Truk doesn't process the C header contents
- Emitted as `#include` directives at the top of generated C

## Extern Function Declarations

Declare C functions you want to call using `extern fn`:

```truk
cimport <stdio.h>;
cimport <math.h>;

extern fn printf(fmt: *i8, ...args): i32;
extern fn sqrt(x: f64): f64;
extern fn fopen(filename: *i8, mode: *i8): *FILE;

fn main(): i32 {
    printf("sqrt(16) = %f\n", sqrt(16.0));
    return 0;
}
```

Extern functions:
- Have no body (end with semicolon, not a block)
- Are type-checked for signature correctness
- Support variadic parameters using `...name` syntax
- Are NOT emitted in generated C (the C header provides the declaration)

## Extern Struct Declarations

Declare C structs in two ways:

### Opaque Structs (Forward Declaration)

Use when you only need pointers to the struct:

```truk
cimport <stdio.h>;

extern struct FILE;

extern fn fopen(filename: *i8, mode: *i8): *FILE;
extern fn fclose(file: *FILE): i32;

fn main(): i32 {
    var f: *FILE = fopen("test.txt", "w");
    if (f != nil) {
        fclose(f);
    }
    return 0;
}
```

### Defined Structs (With Layout)

Use when you need to access struct fields or know the layout:

```truk
cimport <time.h>;

extern struct tm {
    tm_sec: i32,
    tm_min: i32,
    tm_hour: i32,
    tm_mday: i32,
    tm_mon: i32,
    tm_year: i32,
    tm_wday: i32,
    tm_yday: i32,
    tm_isdst: i32
}

extern fn time(timer: *i64): i64;
extern fn localtime(timer: *i64): *tm;

fn main(): i32 {
    var t: i64 = time(nil);
    var info: *tm = localtime(&t);
    return 0;
}
```

Extern structs:
- Are type-checked but NOT emitted in generated C
- Generated C uses `struct Name` notation (works with all C headers)
- Fields are used for Truk-side type checking only
- The C header provides the actual definition

## Compilation with C Libraries

When using C libraries, pass appropriate flags:

```bash
truk myfile.truk -o myprogram -l m
truk myfile.truk -o myprogram -I /path/to/headers -L /path/to/libs -l mylib
```

Flags:
- `-I <path>`: Include directory for headers
- `-L <path>`: Library search path
- `-l <name>`: Link library (e.g., `-l m` for libm.so/libm.dylib)

## Type Compatibility

String literals in Truk are `*u8`, but C functions often expect `*i8`. The type checker allows this:

```truk
extern fn printf(fmt: *i8, ...args): i32;

fn main(): i32 {
    printf("Hello!\n");  // "Hello!\n" is *u8, but *i8 is accepted
    return 0;
}
```

## Best Practices

1. **Use system headers with angle brackets**: `cimport <stdio.h>;`
2. **Use local headers with quotes**: `cimport "myheader.h";`
3. **Declare only what you use**: Don't declare every function in a header
4. **Use opaque structs when possible**: Simpler and more maintainable
5. **Match C signatures exactly**: Especially for variadic functions
6. **Test with actual C compilation**: Ensure your extern declarations match the C header

## Limitations

- Truk doesn't parse C headers (you must declare what you use)
- Variadic parameters must not be the only parameter
- Extern struct fields are for type checking only (not validated against C header)
- C preprocessor macros are not accessible from Truk
