# Truk Projects

A Truk project is a structured collection of libraries and applications defined in a `truk.kit` file. Projects enable code organization, dependency management, and incremental builds.

## Project Structure

Here's the structure of the `manual_cimport` example project from the test suite:

```
manual_cimport/
├── truk.kit
├── libs/
│   └── fileutil/
│       ├── lib.truk
│       ├── lib_impl.truk
│       └── test.truk
├── apps/
│   └── main/
│       └── main.truk
├── include/
│   └── filehelper.h
└── test_data/
    └── input.txt
```

## The truk.kit File

The `truk.kit` file defines your project. Here's the actual configuration from `manual_cimport`:

```
project manual_cimport

library fileutil {
    source = libs/fileutil/lib_impl.truk
    test = libs/fileutil/test.truk
    include_paths = include libs
}

application main {
    source = apps/main/main.truk
    output = build/main
    libraries = fileutil
    include_paths = include libs
}
```

**Note:** Libraries do not have an `output` field. They are automatically compiled to `.cache/libraries/<name>/` with `.c`, `.o`, and `.a` files. Only applications need an `output` field for the final executable.

### Library Configuration

Libraries are automatically compiled to `.cache/libraries/<name>/` with `.c`, `.o`, and `.a` files. You don't specify an output path.

**Fields:**
- `source`: Entry point file for the library (required)
- `test`: Optional test file for the library
- `include_paths`: Directories to search for imports and C headers
- `depends`: Other libraries this library depends on

### Application Configuration

Applications are compiled to the specified output path.

**Fields:**
- `source`: Entry point file (must have a `main` function) (required)
- `output`: Path for the compiled executable (required)
- `libraries`: Truk libraries to link against
- `include_paths`: Directories for imports and C headers
- `library_paths`: Directories to search for external C libraries

## How Imports Work in Projects

### Library Interfaces

The `manual_cimport` example demonstrates the recommended pattern:

**libs/fileutil/lib.truk** (interface):
```truk
extern fn get_file_size(path: *i8): i64;
extern fn get_first_byte(path: *i8): i32;
extern fn get_number_from_file(path: *i8): i32;
```

**libs/fileutil/lib_impl.truk** (implementation):
```truk
cimport <stdio.h>;
cimport "filehelper.h";

extern fn read_file_size(filename: *i8): i64;
extern fn read_first_byte(filename: *i8): i32;
extern fn read_file_as_number(filename: *i8): i32;

import "lib.truk";

fn get_file_size(path: *i8): i64 {
    return read_file_size(path);
}

fn get_first_byte(path: *i8): i32 {
    return read_first_byte(path);
}

fn get_number_from_file(path: *i8): i32 {
    return read_file_as_number(path);
}
```

**apps/main/main.truk** (application):
```truk
cimport <stdio.h>;

extern fn printf(fmt: *i8, ...args): i32;

import "fileutil/lib.truk";

fn main(): i32 {
    var path: *i8 = "test_data/input.txt";
    
    var size: i64 = get_file_size(path);
    printf("File size: %ld bytes\n", size);
    
    var first: i32 = get_first_byte(path);
    printf("First byte: %d\n", first);
    
    var num: i32 = get_number_from_file(path);
    printf("Number from file: %d\n", num);
    
    return num;
}
```

**Key points:**
- The interface file (`lib.truk`) declares functions with `extern fn`
- The implementation (`lib_impl.truk`) imports the interface and provides definitions
- Applications import the interface, not the implementation
- The `extern fn` declarations tell Truk's type checker about function signatures

### Import Resolution

With `include_paths = include libs` in `truk.kit`:

```truk
import "fileutil/lib.truk";
```

Searches:
1. `./fileutil/lib.truk` (relative to current file)
2. `include/fileutil/lib.truk`
3. `libs/fileutil/lib.truk` ✓ (found)

### Importing C Headers

```truk
cimport <stdio.h>;
cimport "filehelper.h";

extern fn printf(fmt: *i8, ...args): i32;
extern fn read_file_size(filename: *i8): i64;
```

- `cimport` includes C headers in generated code
- `extern fn` declarations tell Truk about C functions
- System headers use angle brackets `<>`
- Local headers use quotes `""`
- See [imports.md](imports.md) for detailed C interop

## Building Projects

### Build Command

```bash
truk build
```

Output from building `manual_cimport`:

```
Building library: fileutil
Building application: main
Successfully built manual_cimport
```

On subsequent builds (no changes):

```
Library 'fileutil' is up to date
Building application: main
Successfully built manual_cimport
```

### What Happens During Build

1. Reads `truk.kit` to understand project structure
2. Builds libraries in dependency order
3. For each library:
   - Checks if source files changed (incremental build)
   - If changed, compiles to `.cache/libraries/<name>/`
   - Creates `.c`, `.o`, and `.a` files
4. Compiles applications, linking against library `.o` files
5. Outputs executable to specified path

### Clean Command

```bash
truk clean
```

Output:

```
Removed: /path/to/build/main
Removed empty build directory
Removed .cache directory (8 file(s))
Cleaned 9 build artifact(s)
```

Removes:
- The `build/` directory
- The `.cache/` directory

## Incremental Builds

Truk caches compiled libraries in `.cache/`:

```
.cache/
└── libraries/
    └── fileutil/
        ├── fileutil.c          # Generated C code
        ├── fileutil.o          # Compiled object file
        ├── fileutil.a          # Static library archive
        └── .build_info.json    # Build metadata (source mtimes)
```

**How it works:**
- Tracks modification times of all source files (including imports)
- Compares against last build time stored in `.build_info.json`
- Only rebuilds if sources changed
- Applications always rebuild (fast since libraries are cached)

**Cache invalidation triggers:**
- Source file modifications
- Changes to imported files
- Dependency changes

## Library Dependencies

Libraries can depend on other libraries:

```
library core {
    source = libs/core/core.truk
}

library utils {
    source = libs/utils/utils.truk
    depends = core
}

application main {
    source = apps/main/main.truk
    output = build/main
    libraries = utils core
}
```

**Rules:**
- Libraries build in dependency order (dependencies first)
- Circular dependencies are detected and reported as errors
- Applications must list all libraries they use

## Testing Libraries

```
library fileutil {
    source = libs/fileutil/lib_impl.truk
    test = libs/fileutil/test.truk
}
```

Run tests:

```bash
truk test
```

Test files are standalone programs that exercise library functions.

## Best Practices

### Project Organization

1. **Separate interface from implementation**: `lib.truk` for declarations, `lib_impl.truk` for definitions
2. **One library per directory**: Keep related code together
3. **Use meaningful names**: `fileutil`, `networking`, not `lib1`, `lib2`
4. **Group by feature**: Organize by functionality

### Import Strategy

1. **Import interfaces, not implementations**: Import `lib.truk`, not `lib_impl.truk`
2. **Use relative imports within libraries**: Keep library code self-contained
3. **Use include_paths for cross-library imports**: Makes refactoring easier
4. **Minimize dependencies**: Only import what you need

### Library Design

1. **Keep interfaces small**: Only expose what consumers need
2. **Use `extern fn` for public API**: Clear contract between library and users
3. **Document behavior**: Add comments in interface files
4. **Test thoroughly**: Write comprehensive test files

### Build Configuration

1. **Set include_paths consistently**: Use same paths for libraries and applications
2. **List all dependencies**: Don't rely on transitive includes
3. **Keep truk.kit simple**: Don't over-configure

## Common Patterns

### Wrapper Libraries

Wrap C libraries with Truk-friendly interfaces (like `fileutil` wraps `filehelper.h`):

```truk
cimport "mylib.h";

extern fn c_function(arg: *i8): i32;

fn my_function(arg: *i8): i32 {
    return c_function(arg);
}
```

### Utility Libraries

Collect common functions:

```truk
fn max(a: i32, b: i32): i32 {
    if (a > b) { return a; }
    return b;
}

fn min(a: i32, b: i32): i32 {
    if (a < b) { return a; }
    return b;
}
```

## Troubleshooting

### "No truk.kit found"

Make sure you're in the project directory or a subdirectory.

### "Library 'X' depends on unknown library 'Y'"

Check that all dependencies are defined in `truk.kit`.

### "Circular dependency detected"

Libraries cannot depend on each other in a cycle. Refactor to break the cycle.

### Import not found

Check:
1. File path is correct (relative or in `include_paths`)
2. `include_paths` is set in `truk.kit`
3. File extension is `.truk`

### Build is slow

First build compiles everything. Subsequent builds use caching and are much faster.

## Example: Complete Project

See `tests/projects/manual_cimport/` in the Truk repository for a complete working example demonstrating:
- Library with C interop
- Interface/implementation separation
- Application using the library
- Include paths configuration
- Test file
