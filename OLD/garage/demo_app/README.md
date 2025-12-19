# Truk Demo App - Modular Build Example

This demo showcases Truk's modular build system with separate libraries and applications.

## Project Structure

```
demo_app/
├── pkg/                    # Reusable libraries
│   ├── lib_a/
│   │   └── library_a.truk  # Point math library
│   └── lib_b/
│       └── library_b.truk  # Math utilities library
├── cmds/                   # Applications
│   ├── app_one/
│   │   └── main.truk       # Uses both lib_a and lib_b
│   └── app_two/
│       └── main.truk       # Uses only lib_b
└── build/                  # Generated files (created by make)
```

## Build Process

The Makefile demonstrates a modular build with **include path support**:

1. **Libraries** (`lib_a`, `lib_b`):
   - Transpiled to C using `truk toc` (generates `.h` and `.c`)
   - No main function = automatic library mode

2. **Applications** (`app_one`, `app_two`):
   - Import libraries using `import "lib_a/library_a.truk"`
   - Compiled with `-I pkg` flag to resolve imports
   - Links against library code automatically

## Key Feature: Include Paths

The `-I` flag allows modular imports:

```truk
import "lib_a/library_a.truk";  // Resolved via -I pkg
import "lib_b/library_b.truk";  // Resolved via -I pkg
```

The compiler searches:
1. Relative to the current file
2. In each `-I` include directory

## Building

```bash
# Build everything
make all

# Build only libraries
make libs

# Build specific app
make app_one

# Clean build artifacts
make clean
```

## Running

```bash
# Run app_one
make run_app_one
./build/app_one

# Output:
# === App One: Point Math Demo ===
# Point 1: (3, 4)
# Point 2: (5, 12)
# Sum: (8, 16)
# Distance squared from origin: 320
# 10 + 20 = 30
```

## What This Demonstrates

✅ **Library Generation**: `toc` automatically generates `.h` + `.c` for libraries (no main)
✅ **Include Path Resolution**: `-I` flag enables modular imports
✅ **Code Reuse**: Libraries shared between applications
✅ **Clean Separation**: Libraries in `pkg/`, applications in `cmds/`
✅ **Incremental Builds**: Make tracks dependencies efficiently

## Implementation Details

- **Compiler Enhancement**: Added include path support to import resolver
- **Automatic Detection**: Presence of `main` determines application vs library
- **Header Generation**: Libraries get clean headers with only public API
- **Source Organization**: Implementation details stay in `.c` files
