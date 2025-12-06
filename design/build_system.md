# Build System

## Quick Reference

| Command | Action |
|---------|--------|
| `./bar.sh build asan` | Build with AddressSanitizer in `build-asan/` |
| `./bar.sh build dev` | Build debug in `build-dev/` |
| `./bar.sh build rel` | Build release in `build-rel/` |
| `./bar.sh build` | Build all three configurations |
| `./bar.sh test` | Build asan + run CTest + execute `tests/*.sh` |
| `./bar.sh clean` | Remove all build directories |

## Architecture

### Package System

Each subdirectory in `pkg/` is an independent static library.

**Structure:**
```
pkg/
  CMakeLists.txt          # Includes all package subdirectories
  <package>/
    CMakeLists.txt        # Defines library target
    *.hpp                 # Headers (exported)
    *.cpp                 # Implementation
    tests/
      CMakeLists.txt      # Test executable(s)
      test_*.cpp          # CTest tests
```

**Package CMakeLists.txt:**
- Collects source files
- Creates library target (static by default, dynamic with `-DBUILD_SHARED_LIBS=ON`)
- Sets include directories (current dir exported PUBLIC)
- Adds tests subdirectory

### Command System

Each subdirectory in `cmds/` is an executable.

**Structure:**
```
cmds/
  CMakeLists.txt          # Includes all command subdirectories
  <command>/
    CMakeLists.txt        # Defines executable target
    main.cpp              # Entry point
```

**Command CMakeLists.txt:**
- Creates executable target
- Links against required package libraries
- Example: `target_link_libraries(truk PRIVATE matter)`

## Build Configurations

| Config | Directory | Type | Flags |
|--------|-----------|------|-------|
| asan | `build-asan/` | Debug | `-fsanitize=address -fno-omit-frame-pointer -g` |
| dev | `build-dev/` | Debug | `-g -O0` |
| rel | `build-rel/` | Release | `-O3 -DNDEBUG` |

All configurations: C++20, Clang, `-Wall -Wextra -Wpedantic`

## Testing

### CTest
- Automatically runs after each build
- Test executables in `pkg/<package>/tests/`
- Parallel execution

### Shell Scripts
- Place executable `.sh` files in `tests/` directory
- Executed by `./bar.sh test` after CTest passes
- Exit code 0 = pass, non-zero = fail
- Run in definition order

## Adding Components

### New Package
1. Create `pkg/<name>/` directory
2. Add `CMakeLists.txt` defining library target
3. Add to `pkg/CMakeLists.txt`: `add_subdirectory(<name>)`
4. Create `tests/` subdirectory with test executable

### New Command
1. Create `cmds/<name>/` directory
2. Add `CMakeLists.txt` defining executable target
3. Add to `cmds/CMakeLists.txt`: `add_subdirectory(<name>)`
4. Link required libraries: `target_link_libraries(<name> PRIVATE <pkg>...)`

## LSP Support

`compile_commands.json` generated in each build directory.

VSCode uses `build-dev/compile_commands.json`.
Neovim/clangd configured via `.clangd`.

