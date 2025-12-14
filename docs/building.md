# Building Truk Programs

Truk provides multiple compilation modes to suit different use cases.

## Compilation Commands

### `truk compile` - Build Executables

Compiles Truk source directly to an executable using TCC:

```bash
truk input.truk -o output_binary
```

- Requires a `main` function
- Produces a native executable
- Uses TCC for fast compilation

### `truk run` - JIT Execution

Runs Truk source immediately without creating an output file:

```bash
truk run input.truk [-- program args...]
```

- Requires a `main` function
- Compiles to memory and executes
- Perfect for testing and scripting

### `truk toc` - Transpile to C

Converts Truk source to C code:

```bash
truk toc input.truk -o output.c
```

**Behavior depends on presence of `main` function:**

#### With `main` function (Application)
- Generates a single `.c` file
- Contains all runtime code and implementations
- Ready to compile with any C compiler

#### Without `main` function (Library)
- Generates both `.h` and `.c` files
- **Header (`.h`)**: Clean interface with type definitions, struct declarations, and function prototypes
- **Source (`.c`)**: Implementation details, runtime macros, and function bodies
- Can be used as a reusable library

**Example:**

```bash
# Library (no main) - produces mylib.h and mylib.c
truk toc mylib.truk -o mylib.c

# Application (has main) - produces app.c only
truk toc app.truk -o app.c
```

## Example: Using argc/argv

```truk
cimport <stdio.h>;

extern fn printf(fmt: *i8, ...args): i32;

fn main(argc: i32, argv: **i8) : i32 {
  printf("Program: %s\n", argv[0]);
  printf("Args: %d\n", argc);
  return 0;
}
```

Compile and run:
```bash
truk input.truk -o program
./program arg1 arg2
```

Or run directly:
```bash
truk run input.truk -- arg1 arg2
```
