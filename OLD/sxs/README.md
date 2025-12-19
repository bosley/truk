# sxs - Systems Extension Support

The sxs runtime library provides core functionality for truk programs, including type definitions and runtime safety checks.

## Architecture

sxs is developed as a standard C library in this directory but is **embedded directly into the truk compiler binary** at build time. When truk compiles a program, it inlines the sxs runtime code directly into the generated C code.

## What sxs Provides

**Type Definitions:**
- Type aliases: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`

**Memory Operations (inlined):**
- `sxs_alloc(u64 size)` - Single value allocation (internal)
- `sxs_free(void *ptr)` - Single value deallocation (internal)
- `sxs_alloc_array(u64 elem_size, u64 count)` - Array allocation (internal)
- `sxs_free_array(void *ptr)` - Array deallocation (internal)

**Type Operations (inlined):**
- `sxs_sizeof_type(u64 size)` - Type size query

**Safety Checks (inlined):**
- `sxs_bounds_check(u64 idx, u64 len)` - Array bounds validation

**Error Handling:**
- `sxs_panic(const char *msg, u64 len)` - Fatal error

**Program Entry:**
- `sxs_start(sxs_target_app_s *app)` - Runtime entry point

## Program Entry Structure

All truk programs enter through the runtime via `sxs_start`:

```c
typedef struct {
  void *entry_fn;      // User's main function
  bool has_args;       // Whether main takes argc/argv
  i32 argc;           // Command line arguments
  i8 **argv;
} sxs_target_app_s;
```

Generated programs create this structure and pass it to `sxs_start`, which dispatches to the user's code.

## Build Process

1. sxs C files are written in `runtime/sxs/`
2. sxs library is built FIRST in the build process
3. sxs tests are run to verify correctness
4. CMake script (`cmake/EmbedRuntime.cmake`) converts sxs files to C++ string literals
5. Generated header (`build/generated/embedded_runtime.hpp`) is compiled into truk
6. When truk compiles a program, it inlines the runtime code into generated C

## Build Order

The CMake build ensures this order:
1. **sxs library** - Built first (C library)
2. **test_sxs_runtime** - Tests run immediately after sxs builds
3. **truk_emitc** - Depends on sxs, embeds runtime
4. **truk binary** - Final executable

## Testing

Run sxs tests:
```bash
cd build
ctest -R "^sxs_"
```

Or run directly:
```bash
./runtime/sxs/tests/test_sxs_runtime -v
```

## File Organization

```
runtime/sxs/
├── include/sxs/
│   ├── types.h      - Type aliases
│   ├── runtime.h    - Runtime functions (inlined hot-path functions)
│   └── sxs.h        - Master include
├── src/
│   └── runtime.c    - Non-inlined runtime functions
└── tests/
    ├── test_runtime.cpp  - CppUTest unit tests
    └── CMakeLists.txt
```

## Performance

Hot-path functions are `static inline` to eliminate function call overhead:
- Memory allocation/deallocation
- Bounds checking (called on every array access)
- Type size queries

Cold-path functions remain as regular functions:
- Panic (error path, rarely called)
- Program startup (called once)

## Memory Management

All memory operations route through sxs functions, which currently call standard `malloc`/`free`. This provides a single point of control for future garbage collection or custom allocator implementations.

## Independence

The truk binary is completely self-contained. Once built, it can compile truk programs anywhere on the system without access to the compiler source code or runtime files. All runtime code is embedded as string literals in the binary.
