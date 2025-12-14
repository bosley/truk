# sxs - Systems Extension Support

The sxs runtime library provides core functionality for truk programs, including type definitions and runtime safety checks.

## Architecture

sxs is developed as a standard C library in this directory but is **embedded directly into the truk compiler binary** at build time. When truk compiles a program, it inlines the sxs runtime code directly into the generated C code.

This approach provides:
- **Zero installation dependencies** - users only need the truk binary
- **Version compatibility** - runtime always matches compiler version  
- **Fast compilation** - no external library linking required
- **Proper development** - sxs is written as real C code with full IDE support
- **Tested before use** - sxs tests run before truk is built

## What sxs Provides

sxs provides **minimal runtime support**:
- **Type aliases**: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64
- **Bounds checking**: `sxs_bounds_check()` for array access validation
- **Panic handling**: `sxs_panic()` for fatal errors

sxs does **NOT** provide:
- Memory allocation (uses standard `malloc`/`free` directly)
- Data structures (slices are generated per-type by the compiler)
- String handling
- I/O operations

Memory management is handled by the generated C code calling standard library functions directly.

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

This guarantees the runtime is tested before being embedded into the compiler.

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
│   ├── types.h      - Type aliases (i8, u8, f32, etc.)
│   ├── runtime.h    - Runtime safety functions (panic, bounds_check)
│   └── sxs.h        - Master include
├── src/
│   └── runtime.c    - Runtime safety implementations
└── tests/
    ├── test_runtime.cpp  - CppUTest unit tests
    └── CMakeLists.txt
```

## Application vs Library Compilation

The embedded runtime system supports two compilation modes:

- **Application mode**: Includes full runtime with implementations
- **Library mode**: Includes only type definitions and declarations

This is controlled via the `for_application` and `for_library` flags in the embedded runtime metadata.

## Adding New Runtime Features

To add new functionality to sxs:

1. Add C header to `include/sxs/`
2. Add C implementation to `src/`
3. Add tests to `tests/test_runtime.cpp`
4. Update `cmake/EmbedRuntime.cmake` to include new files
5. Rebuild - runtime is automatically tested and embedded

## Independence from Compiler Source

The truk binary is completely self-contained. Once built, it can compile truk programs anywhere on the system without access to the compiler source code or runtime files. All runtime code is embedded as string literals in the binary.

## C/C++ Interop

All sxs functions use `extern "C"` linkage to ensure they can be called from both C (in generated code) and C++ (in tests and the compiler itself).
