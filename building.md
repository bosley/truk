# Building with Truk Kit

Truk Kit provides a project-based build system using `truk.kit` configuration files.

## Quick Start

Create a new project:

```bash
./truk new myproject
cd myproject
./truk build
./build/main
```

## Commands

```bash
./truk new <name>       # Create new project
./truk build [target]   # Build project
./truk test [target]    # Run tests
./truk clean            # Remove build artifacts
```

## Project Structure

```
myproject/
  truk.kit              # Project configuration
  apps/
    main/
      main.truk
  libs/
    mylib/
      lib.truk
      test.truk
  build/                # Output directory (auto-created)
```

## truk.kit Format

### Basic Application

```
project myproject

application main {
    source = apps/main/main.truk
    output = build/main
}
```

### Library with Test

```
library math {
    source = libs/math/lib.truk
    output = build/libmath.c
    test = libs/math/test.truk
}
```

### Library with Dependencies

```
library database {
    source = libs/database/lib.truk
    output = build/libdatabase.c
    depends = json logger
}
```

### Application with Libraries

```
application server {
    source = apps/server/main.truk
    output = build/server
    libraries = http json database
    library_paths = /usr/local/lib
    include_paths = /usr/local/include
}
```

## Build Order

Libraries are built in dependency order automatically. Applications are built after all libraries.

## Testing

Test files should have a `main()` function that returns 0 for success, non-zero for failure:

```truk
import "lib.truk";

fn test_add(): i32 {
    if (add(2, 3) != 5) {
        return 1;
    }
    return 0;
}

fn main(): i32 {
    if (test_add() != 0) return 1;
    return 0;
}
```

Run tests:

```bash
./truk test              # Run all tests
./truk test math         # Run specific library test
```

## Backward Compatibility

Direct compilation still works:

```bash
./truk file.truk         # Compile to executable
./truk run file.truk     # JIT execution
./truk toc file.truk     # Transpile to C
./truk tcc file.c        # Compile C with TCC
```

## Notes

- All paths in `truk.kit` are relative to the kit file location
- Libraries are compiled to `.c` files and linked via TCC
- The import system handles including library code in applications
- Comments start with `#`
- Use quotes for paths with spaces: `source = "path with spaces/file.truk"`
