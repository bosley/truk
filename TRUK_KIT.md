# Truk Kit: Project-Based Build System

This document describes the design and implementation plan for Truk's project-based build system, inspired by Go's workflow.

## Overview

Truk Kit introduces a project-based workflow using `truk.kit` files (similar to `go.mod`) that define libraries, applications, and tests in a declarative configuration format. This eliminates command-line flag management and provides a scalable build system for real-world projects.

## Design Goals

1. **Familiar workflow** - Mirror Go's proven project structure
2. **No directory enforcement** - Use relative paths from kit file
3. **Explicit build order** - Declare dependencies, auto-resolve build order
4. **Multi-target support** - Multiple libraries and applications per project
5. **Test integration** - First-class test support
6. **Backward compatible** - Preserve `./truk file.truk` direct compilation

## Commands

### New Commands

```bash
./truk new <name>             # Create new project with truk.kit
./truk build [target]         # Build project (or specific target directory)
./truk test [target]          # Run all tests (or specific test)
./truk clean                  # Remove build artifacts
```

### Existing Commands (Preserved)

```bash
./truk file.truk              # Direct compilation (unchanged)
./truk compile file.truk      # Explicit compile (unchanged)
./truk run file.truk          # Direct run (unchanged)
./truk toc file.truk          # Transpile to C (unchanged)
./truk tcc file.truk          # Compile with TCC (unchanged)
```

## Project Structure

No enforced directory structure. Example layout:

```
myproject/
  truk.kit                    # Project configuration
  apps/
    server/
      main.truk
    cli/
      main.truk
  libs/
    math/
      lib.truk
      test.truk
    geometry/
      lib.truk
      test.truk
  build/                      # Output directory (auto-created)
    libmath.a
    libgeometry.a
    server
    cli
```

## The `truk.kit` File Format

Simple key-value configuration format (not Truk syntax):

### Basic Structure

```
project <name>                # Optional: project name for documentation

library <name> {
    source = <path>           # Required: path to lib.truk
    output = <path>           # Required: output library path
    depends = <lib1> <lib2>   # Optional: library dependencies
    test = <path>             # Optional: test file path
}

application <name> {
    source = <path>           # Required: path to main.truk
    output = <path>           # Required: output executable path
    libraries = <lib1> <lib2> # Optional: libraries to link
    library_paths = <path1>   # Optional: additional -L paths
    include_paths = <path1>   # Optional: additional -I paths
}
```

### Example: Simple Project

```
project myapp

library math {
    source = libs/math/lib.truk
    output = build/libmath.a
    test = libs/math/test.truk
}

application main {
    source = apps/main/main.truk
    output = build/myapp
    libraries = math
}
```

### Example: Complex Multi-Target Project

```
project webserver

library json {
    source = libs/json/lib.truk
    output = build/libjson.a
    test = libs/json/test.truk
}

library database {
    source = libs/database/lib.truk
    output = build/libdatabase.a
    depends = json
    test = libs/database/test.truk
}

library http {
    source = libs/http/lib.truk
    output = build/libhttp.a
    depends = json
    test = libs/http/test.truk
}

application server {
    source = apps/server/main.truk
    output = build/server
    libraries = http json database
}

application admin {
    source = apps/admin/main.truk
    output = build/admin
    libraries = http json database
}
```

## Build Order Resolution

The build system automatically resolves dependencies and determines build order:

1. Parse `truk.kit` file
2. Build dependency graph from `depends` clauses
3. Perform topological sort
4. Build libraries in dependency order
5. Build applications (which depend on libraries)

Example build order for complex project above:
1. `json` library (no dependencies)
2. `database` library (depends on json)
3. `http` library (depends on json)
4. `server` application (uses http, json, database)
5. `admin` application (uses http, json, database)

## Test Integration

Tests are defined per-library in the `truk.kit` file:

```
library math {
    source = libs/math/lib.truk
    output = build/libmath.a
    test = libs/math/test.truk
}
```

Test file structure (`libs/math/test.truk`):

```truk
import "lib.truk";

fn test_add(): i32 {
    if (add(2, 3) != 5) {
        return 1;
    }
    return 0;
}

fn test_multiply(): i32 {
    if (multiply(3, 4) != 12) {
        return 1;
    }
    return 0;
}

fn main(): i32 {
    if (test_add() != 0) return 1;
    if (test_multiply() != 0) return 1;
    return 0;
}
```

Running tests:

```bash
./truk test                   # Run all tests in project
./truk test libs/math         # Run specific test
```

Test execution:
1. Compile test file as executable
2. Link against library being tested
3. Run executable
4. Report success (exit 0) or failure (exit non-zero)

## Workflow Examples

### Creating a New Project

```bash
./truk new myproject
cd myproject
```

Creates:
```
myproject/
  truk.kit              # Empty project config
  apps/
    main/
      main.truk         # Stub main function
  libs/
```

### Building a Project

```bash
cd myproject
./truk build
```

Output:
```
Building library: math
Building library: geometry
Building application: server
Building application: cli
Successfully built myproject
```

### Running Tests

```bash
./truk test
```

Output:
```
Testing library: math ... PASS
Testing library: geometry ... PASS
All tests passed
```

### Running an Application

```bash
./truk run server
```

Builds (if needed) and runs the `server` application.

## Implementation Requirements

### 1. Kit File Parser

Create `libs/kit/` library with:

```cpp
struct library_config_s {
    std::string name;
    std::string source;
    std::string output;
    std::vector<std::string> depends;
    std::string test;
};

struct application_config_s {
    std::string name;
    std::string source;
    std::string output;
    std::vector<std::string> libraries;
    std::vector<std::string> library_paths;
    std::vector<std::string> include_paths;
};

struct kit_config_s {
    std::string project_name;
    std::vector<library_config_s> libraries;
    std::vector<application_config_s> applications;
};

kit_config_s parse_kit_file(const std::string &path);
```

Parser implementation:
- Line-by-line parsing
- State machine for blocks (library/application)
- Key-value pair extraction
- Path resolution relative to kit file location

### 2. Dependency Resolution

Create dependency resolver:

```cpp
struct build_order_s {
    std::vector<library_config_s> libraries;  // In build order
    std::vector<application_config_s> applications;
};

build_order_s resolve_dependencies(const kit_config_s &config);
```

Algorithm:
- Build directed graph from `depends` clauses
- Topological sort (Kahn's algorithm or DFS)
- Detect circular dependencies
- Return ordered build list

### 3. New Commands

#### `./truk new <name>`

Implementation in `apps/truk/commands/new.cpp`:

```cpp
int new_project(const std::string &name) {
    fs::create_directory(name);
    fs::create_directory(name + "/apps");
    fs::create_directory(name + "/apps/main");
    fs::create_directory(name + "/libs");
    
    std::ofstream kit(name + "/truk.kit");
    kit << "project " << name << "\n\n";
    kit << "# Define your libraries and applications here\n\n";
    kit << "application main {\n";
    kit << "    source = apps/main/main.truk\n";
    kit << "    output = build/main\n";
    kit << "}\n";
    kit.close();
    
    std::ofstream main(name + "/apps/main/main.truk");
    main << "fn main(): i32 {\n";
    main << "    return 0;\n";
    main << "}\n";
    main.close();
    
    fmt::print("Created project: {}\n", name);
    return 0;
}
```

#### `./truk build [target]`

Implementation in `apps/truk/commands/build.cpp`:

```cpp
int build(const std::string &target_dir) {
    std::string kit_path = find_kit_file(target_dir);
    if (kit_path.empty()) {
        fmt::print(stderr, "Error: No truk.kit found\n");
        return 1;
    }
    
    auto config = parse_kit_file(kit_path);
    auto build_order = resolve_dependencies(config);
    
    for (const auto &lib : build_order.libraries) {
        fmt::print("Building library: {}\n", lib.name);
        int result = lib_compile({lib.source, lib.output});
        if (result != 0) {
            fmt::print(stderr, "Failed to build library: {}\n", lib.name);
            return 1;
        }
    }
    
    for (const auto &app : config.applications) {
        fmt::print("Building application: {}\n", app.name);
        int result = compile({
            app.source,
            app.output,
            app.include_paths,
            app.library_paths,
            app.libraries,
            {}
        });
        if (result != 0) {
            fmt::print(stderr, "Failed to build application: {}\n", app.name);
            return 1;
        }
    }
    
    fmt::print("Successfully built {}\n", config.project_name);
    return 0;
}
```

#### `./truk test [target]`

Implementation in `apps/truk/commands/test.cpp`:

```cpp
int test(const std::string &target_dir) {
    std::string kit_path = find_kit_file(target_dir);
    auto config = parse_kit_file(kit_path);
    auto build_order = resolve_dependencies(config);
    
    int failed = 0;
    for (const auto &lib : build_order.libraries) {
        if (lib.test.empty()) continue;
        
        fmt::print("Testing library: {} ... ", lib.name);
        
        std::string test_exe = "build/test_" + lib.name;
        int result = compile({
            lib.test,
            test_exe,
            {},
            {"build"},
            {lib.name},
            {}
        });
        
        if (result != 0) {
            fmt::print("COMPILE FAILED\n");
            failed++;
            continue;
        }
        
        int test_result = system(test_exe.c_str());
        if (test_result == 0) {
            fmt::print("PASS\n");
        } else {
            fmt::print("FAIL\n");
            failed++;
        }
    }
    
    if (failed == 0) {
        fmt::print("All tests passed\n");
        return 0;
    } else {
        fmt::print("{} test(s) failed\n", failed);
        return 1;
    }
}
```

#### `./truk clean`

Implementation in `apps/truk/commands/clean.cpp`:

```cpp
int clean(const std::string &target_dir) {
    std::string kit_path = find_kit_file(target_dir);
    auto config = parse_kit_file(kit_path);
    
    for (const auto &lib : config.libraries) {
        fs::remove(lib.output);
    }
    
    for (const auto &app : config.applications) {
        fs::remove(app.output);
    }
    
    fs::remove_all("build");
    
    fmt::print("Cleaned build artifacts\n");
    return 0;
}
```

### 4. Argument Parsing Updates

Update `apps/truk/common/args.cpp`:

```cpp
struct args_s {
    std::string command;      // "new", "build", "test", "clean", etc.
    std::string project_name; // For "new" command
    std::string target_dir;   // For "build", "test" commands
    
    // Existing fields for direct compilation
    std::string input_file;
    std::string output_file;
    std::vector<std::string> include_paths;
    std::vector<std::string> library_paths;
    std::vector<std::string> libraries;
    std::vector<std::string> rpaths;
};

args_s parse_args(int argc, char **argv) {
    if (argc < 2) {
        return {.command = "compile"};
    }
    
    std::string first_arg = argv[1];
    
    if (first_arg == "new") {
        return {.command = "new", .project_name = argv[2]};
    }
    else if (first_arg == "build") {
        return {.command = "build", .target_dir = argc > 2 ? argv[2] : "."};
    }
    else if (first_arg == "test") {
        return {.command = "test", .target_dir = argc > 2 ? argv[2] : "."};
    }
    else if (first_arg == "clean") {
        return {.command = "clean", .target_dir = argc > 2 ? argv[2] : "."};
    }
    
    // Existing command parsing (toc, tcc, run, compile)
}
```

### 5. Main Entry Point Updates

Update `apps/truk/main.cpp`:

```cpp
int main(int argc, char **argv) {
    auto args = truk::common::parse_args(argc, argv);
    
    if (args.command == "new") {
        return truk::commands::new_project(args.project_name);
    }
    else if (args.command == "build") {
        return truk::commands::build(args.target_dir);
    }
    else if (args.command == "test") {
        return truk::commands::test(args.target_dir);
    }
    else if (args.command == "clean") {
        return truk::commands::clean(args.target_dir);
    }
    else if (args.command == "toc") {
        return truk::commands::toc({args.input_file, args.output_file, args.include_paths});
    }
    else if (args.command == "tcc") {
        return truk::commands::tcc({args.input_file, args.output_file, args.include_paths, 
                                    args.library_paths, args.libraries, args.rpaths});
    }
    else if (args.command == "run") {
        return truk::commands::run({args.input_file, args.include_paths, args.library_paths, 
                                    args.libraries, args.rpaths, argc, argv});
    }
    else {
        return truk::commands::compile({args.input_file, args.output_file, args.include_paths, 
                                        args.library_paths, args.libraries, args.rpaths});
    }
}
```

## File Organization

New files to create:

```
libs/
  kit/
    CMakeLists.txt
    include/
      truk/
        kit/
          parser.hpp
          resolver.hpp
    src/
      parser.cpp
      resolver.cpp
    tests/
      CMakeLists.txt
      test_parser.cpp
      test_resolver.cpp

apps/
  truk/
    commands/
      new.cpp
      new.hpp
      build.cpp
      build.hpp
      test.cpp
      test.hpp
      clean.cpp
      clean.hpp
```

## Benefits

1. ✅ **No CLI flag management** - Configuration in `truk.kit`
2. ✅ **Familiar workflow** - Go-inspired commands
3. ✅ **Multi-target projects** - Multiple apps/libs per project
4. ✅ **Automatic dependency resolution** - Build order computed automatically
5. ✅ **Test integration** - First-class test support
6. ✅ **Scalable** - Works for small and large projects
7. ✅ **Backward compatible** - Direct compilation still works
8. ✅ **No directory enforcement** - Flexible project structure

## Comparison to Go

| Go | Truk |
|----|------|
| `go.mod` | `truk.kit` |
| `go mod init` | `truk new` |
| `go build` | `truk build` |
| `go test` | `truk test` |
| `go clean` | `truk clean` |
| `go run` | `truk run` |

## Implementation Phases

### Phase 1: Kit File Parser
- Implement `libs/kit/parser.cpp`
- Parse library and application blocks
- Extract key-value pairs
- Resolve paths relative to kit file

### Phase 2: Dependency Resolution
- Implement `libs/kit/resolver.cpp`
- Build dependency graph
- Topological sort
- Circular dependency detection

### Phase 3: New Command
- Implement `commands/new.cpp`
- Create project directory structure
- Generate stub `truk.kit` and `main.truk`

### Phase 4: Build Command
- Implement `commands/build.cpp`
- Find and parse `truk.kit`
- Build libraries in dependency order
- Build applications

### Phase 5: Test Command
- Implement `commands/test.cpp`
- Compile and run test files
- Report results

### Phase 6: Clean Command
- Implement `commands/clean.cpp`
- Remove build artifacts

## Future Enhancements

- Library versioning in kit file
- External dependency management
- Incremental builds (only rebuild changed files)
- Parallel compilation
- Build profiles (debug/release)
- Custom build steps
- Package registry integration
