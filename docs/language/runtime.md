[← Back to Documentation Index](../start-here.md)

# Truk Runtime Architecture

**Language Reference:** [Grammar](grammar.md) · [Builtins](builtins.md) · [Maps](maps.md) · [Defer](defer.md) · [Imports](imports.md) · [Lambdas](lambdas.md) · [Privacy](privacy.md)

---

## Runtime Integration Flow

This diagram shows how the sxs runtime is embedded into the truk compiler and how compiled user code interacts with it.

```mermaid
graph TB
    subgraph buildTime [Build Time - Compiler Construction]
        sxsSource[sxs Runtime Source<br/>C library in runtime/sxs/]
        sxsBuild[Build sxs Library]
        sxsTest[Run sxs Tests]
        embedScript[EmbedRuntime.cmake]
        embeddedHeader[embedded_runtime.hpp<br/>String literals in C++]
        emitcLib[truk_emitc Library]
        trukBinary[truk Compiler Binary<br/>Self-contained executable]
        
        sxsSource --> sxsBuild
        sxsBuild --> sxsTest
        sxsTest --> embedScript
        embedScript -->|Converts C to strings| embeddedHeader
        embeddedHeader --> emitcLib
        emitcLib --> trukBinary
    end
    
    subgraph compileTime [Compile Time - User Program]
        userCode[User .truk File<br/>with main function]
        trukCompiler[truk Compiler]
        emitter[C Code Emitter]
        generatedC[Generated C Code<br/>with inlined sxs runtime]
        tccCompiler[TCC Backend]
        executable[Native Executable]
        
        userCode --> trukCompiler
        trukCompiler --> emitter
        emitter -->|Inlines runtime code| generatedC
        generatedC --> tccCompiler
        tccCompiler --> executable
    end
    
    subgraph runtimeExec [Runtime Execution]
        execStart[Program Start]
        sxsStart[sxs_start entry point]
        userMain[User main function]
        sxsFuncs[sxs Runtime Functions<br/>alloc, free, bounds_check, panic]
        
        execStart --> sxsStart
        sxsStart -->|Dispatches to| userMain
        userMain -.->|Calls during execution| sxsFuncs
    end
    
    trukBinary -.->|Contains embedded runtime| trukCompiler
    
    style sxsSource fill:#e1f5ff
    style embeddedHeader fill:#fff4e1
    style generatedC fill:#e8f5e9
    style executable fill:#f3e5f5
    style sxsStart fill:#ffebee
```

## Compilation Pipeline Sequence

This diagram shows the complete compilation process through all library components in temporal order.

```mermaid
sequenceDiagram
    participant User
    participant CLI as truk CLI
    participant Resolver as Import Resolver<br/>(ingestion lib)
    participant Parser as Parser<br/>(ingestion lib)
    participant TypeChecker as Type Checker<br/>(validation lib)
    participant Emitter as C Emitter<br/>(emitc lib)
    participant TCC as TCC Backend<br/>(tcc lib)
    participant Output as Executable

    User->>CLI: truk compile program.truk
    
    Note over CLI,Resolver: Phase 1: Import Resolution
    CLI->>Resolver: resolve(program.truk)
    Resolver->>Parser: parse entry file
    Parser-->>Resolver: AST declarations
    
    loop For each import
        Resolver->>Parser: parse imported file
        Parser-->>Resolver: more declarations
    end
    
    Resolver->>Resolver: topological sort<br/>by dependencies
    Resolver-->>CLI: all_declarations + c_imports
    
    Note over CLI,TypeChecker: Phase 2: Type Validation
    CLI->>TypeChecker: check each declaration
    
    loop For each declaration
        TypeChecker->>TypeChecker: validate types<br/>check symbols<br/>verify semantics
    end
    
    TypeChecker-->>CLI: success or errors
    
    Note over CLI,Emitter: Phase 3: Code Generation
    CLI->>Emitter: add_declarations(all_declarations)
    CLI->>Emitter: set_c_imports(c_imports)
    CLI->>Emitter: finalize()
    
    Emitter->>Emitter: collect declarations
    Emitter->>Emitter: emit forward declarations
    Emitter->>Emitter: emit struct definitions
    Emitter->>Emitter: emit function definitions
    Emitter->>Emitter: inline sxs runtime code
    Emitter->>Emitter: generate main wrapper
    
    Emitter-->>CLI: C source code + metadata
    
    Note over CLI,TCC: Phase 4: Native Compilation
    CLI->>TCC: compile_string(c_source)
    TCC->>TCC: compile C to machine code
    TCC->>Output: write executable
    TCC-->>CLI: success
    
    CLI-->>User: Compilation complete
```

## Import and Extern Declaration Handling

This diagram shows how truk handles two different import mechanisms and how they flow through compilation.

```mermaid
graph TB
    subgraph userCode [User Source Code]
        trukImport[import lib.truk]
        cImport[cimport stdio.h]
        externDecl[extern fn printf]
    end
    
    subgraph trukPath [Truk Import Path]
        parseImported[Parse Imported File]
        mergeAST[Merge AST Declarations]
        topoSort[Topological Sort]
        typeCheckTruk[Type Check Declarations]
        emitTrukCode[Generate C Code]
        resultTruk[Full C Implementation]
    end
    
    subgraph cPath [C Import Path with Extern]
        collectCImport[Collect as Metadata]
        parseExtern[Parse Extern Declaration]
        externAST[Create AST Node<br/>is_extern=true]
        typeCheckExtern[Type Check Signature]
        skipEmit[Skip Code Generation]
        emitInclude[Emit #include Directive]
        resultC[C Header Provides Declaration<br/>Linker Resolves Symbols]
    end
    
    trukImport --> parseImported
    parseImported --> mergeAST
    mergeAST --> topoSort
    topoSort --> typeCheckTruk
    typeCheckTruk --> emitTrukCode
    emitTrukCode --> resultTruk
    
    cImport --> collectCImport
    externDecl --> parseExtern
    parseExtern --> externAST
    externAST --> typeCheckExtern
    typeCheckExtern --> skipEmit
    collectCImport --> emitInclude
    skipEmit --> resultC
    emitInclude --> resultC
    
    subgraph finalOutput [Generated C File]
        includes[#include stdio.h<br/>#include sxs runtime]
        trukFunctions[Generated Truk Functions]
        mainWrapper[main wrapper calling sxs_start]
    end
    
    resultTruk --> trukFunctions
    resultC --> includes
    trukFunctions --> mainWrapper
    includes --> mainWrapper
    
    style trukImport fill:#e3f2fd
    style parseImported fill:#e3f2fd
    style mergeAST fill:#e3f2fd
    style emitTrukCode fill:#e3f2fd
    style resultTruk fill:#c8e6c9
    
    style cImport fill:#fff3e0
    style externDecl fill:#fff3e0
    style collectCImport fill:#fff3e0
    style parseExtern fill:#fff3e0
    style skipEmit fill:#ffccbc
    style resultC fill:#ffe0b2
    
    style finalOutput fill:#f3e5f5
```

### Key Differences

**Truk Imports** (`import "file.truk"`):
- Files are fully parsed into AST
- Declarations merged into compilation unit
- Topologically sorted by dependencies
- Type-checked for correctness
- Full C code generated for all declarations
- Result: Complete implementation in generated C

**C Imports** (`cimport <header.h>`):
- Collected as metadata, not parsed
- Emitted as `#include` directives at top of C file
- Truk does not process header contents
- Result: Pass-through to C compiler

**Extern Declarations** (`extern fn`, `extern struct`):
- Parsed into AST with `is_extern=true` flag
- Type-checked for signature correctness
- Skipped during code generation
- C header provides actual declaration
- Linker resolves symbols at link time
- Result: Type safety without code duplication

**Why Extern is Necessary:**
Truk doesn't parse C headers, so you must explicitly declare C functions and structs you want to use. The type checker validates your extern declarations match how you use them, but trusts that the C header provides the actual implementation.

## Key Components

### Build Time
- **sxs runtime**: Core C library providing type definitions, memory operations, and safety checks
- **EmbedRuntime.cmake**: CMake script that converts sxs C files into C++ string literals
- **embedded_runtime.hpp**: Generated header containing runtime code as strings
- **truk_emitc**: Library that embeds the runtime and generates C code

### Compile Time Libraries
- **ingestion**: Tokenizes, parses .truk files, resolves imports across files
- **validation**: Type checking, symbol resolution, semantic validation
- **emitc**: Generates C code with inlined sxs runtime
- **tcc**: Tiny C Compiler backend for native code generation

### Runtime Execution
- **sxs_start**: Entry point that receives user's main function
- **sxs runtime functions**: Memory management, bounds checking, panic handling
- All runtime code is inlined directly into generated C - no external dependencies

## Flexible Build Workflows

This diagram shows the different compilation commands and how they enable custom build systems with Make or other build tools.

```mermaid
graph TB
    subgraph commands [Truk Commands]
        compile[truk compile<br/>Direct to executable]
        toc[truk toc<br/>Transpile to C]
        tcc[truk tcc<br/>Compile C to executable]
        run[truk run<br/>JIT execution]
    end
    
    subgraph simpleWorkflow [Simple Workflow - Single Command]
        trukFile1[program.truk]
        compileCmd[truk compile program.truk -o app]
        executable1[app executable]
        
        trukFile1 --> compileCmd
        compileCmd --> executable1
    end
    
    subgraph modularWorkflow [Modular Workflow - Make Integration]
        direction TB
        
        subgraph libs [Library Phase]
            libA[lib_a/library_a.truk]
            libB[lib_b/library_b.truk]
            tocA[truk toc library_a.truk]
            tocB[truk toc library_b.truk]
            cFileA[library_a.c + library_a.h]
            cFileB[library_b.c + library_b.h]
            
            libA --> tocA
            libB --> tocB
            tocA --> cFileA
            tocB --> cFileB
        end
        
        subgraph apps [Application Phase]
            appMain[app/main.truk<br/>imports libraries]
            compileApp[truk compile main.truk<br/>-I pkg flag resolves imports]
            executable2[app executable]
            
            appMain --> compileApp
            cFileA -.->|Available via -I| compileApp
            cFileB -.->|Available via -I| compileApp
            compileApp --> executable2
        end
    end
    
    subgraph customWorkflow [Custom Workflow - Full Control]
        direction TB
        
        subgraph transpile [Transpile Phase]
            trukSrc[Multiple .truk files]
            tocCmd[truk toc for each file]
            cSources[Generated .c and .h files]
            
            trukSrc --> tocCmd
            tocCmd --> cSources
        end
        
        subgraph customize [Customization Phase]
            cSources --> customC[Add custom C code<br/>Modify generated files<br/>Add build flags]
            customC --> buildSystem[Custom build system<br/>Make, CMake, Ninja, etc]
        end
        
        subgraph link [Linking Phase]
            buildSystem --> compiler[C Compiler<br/>gcc, clang, tcc]
            compiler --> linkLibs[Link external libraries<br/>-lm, -lpthread, etc]
            linkLibs --> finalExe[Final executable]
        end
    end
    
    compile -.->|Uses internally| simpleWorkflow
    toc -.->|Enables| modularWorkflow
    toc -.->|Enables| customWorkflow
    
    style compile fill:#e1f5ff
    style toc fill:#fff4e1
    style tcc fill:#f3e5f5
    style run fill:#e8f5e9
    
    style executable1 fill:#c8e6c9
    style executable2 fill:#c8e6c9
    style finalExe fill:#c8e6c9
    
    style customC fill:#ffecb3
    style buildSystem fill:#ffecb3
```

### Command Overview

**`truk compile`** - All-in-one compilation:
- Parses .truk files
- Resolves imports
- Type checks
- Generates C code
- Compiles to native executable
- Best for: Simple projects, quick iteration

**`truk toc`** - Transpile to C:
- Parses .truk files
- Resolves imports
- Type checks
- Generates C code files
- Automatically detects library vs application:
  - **No main function**: Generates .h and .c (library mode)
  - **Has main function**: Generates single .c file (application mode)
- Stops before native compilation
- Best for: Modular builds, custom build systems, library generation

**`truk tcc`** - Compile C to executable:
- Takes generated C code
- Compiles to native executable using TCC
- Supports linking flags
- Best for: Two-stage builds, custom C integration

**`truk run`** - JIT execution:
- Compiles in memory
- Executes immediately
- No output file
- Best for: Testing, scripting, rapid prototyping

### Use Cases

**Simple Projects:**

```bash
truk compile main.truk -o myapp
```

**Modular Projects with Libraries:**

```makefile
libs:
    truk toc pkg/lib_a/library_a.truk -o build/lib_a/library_a.c
    truk toc pkg/lib_b/library_b.truk -o build/lib_b/library_b.c

apps: libs
    truk compile app/main.truk -o build/app -I pkg
```

**Custom Build Integration:**

```bash
truk toc src/module1.truk -o build/module1.c
truk toc src/module2.truk -o build/module2.c

gcc -O3 build/module1.c build/module2.c custom.c -o myapp -lm
```

**Mixed C and Truk:**

```bash
truk toc truk_code.truk -o generated.c

gcc -c generated.c -o generated.o
gcc -c existing_c_code.c -o existing.o
gcc generated.o existing.o -o final_app -lpthread
```

### Benefits of `toc` Command

**Incremental Builds:**
- Only regenerate changed .truk files
- Make tracks dependencies automatically
- Faster iteration on large projects

**Build System Integration:**
- Works with Make, CMake, Ninja, etc.
- Standard C files fit existing workflows
- Easy to add custom build steps

**Custom Optimization:**
- Apply specific compiler flags per file
- Use different C compilers (gcc, clang, msvc)
- Control linking order and libraries

**Library Distribution:**
- Automatically generates .h headers for public API (when no main function detected)
- Distribute as standard C libraries
- No truk compiler needed for consumers
- Clean separation: declarations in .h, implementation in .c

**Debugging and Inspection:**
- Inspect generated C code
- Use C debuggers (gdb, lldb)
- Understand compilation output
