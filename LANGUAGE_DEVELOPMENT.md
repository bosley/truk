# Truk Language Development: Current State & Next Steps

(an ai wrote this to help me plan)

## Where We Are

You've built a **complete, working compiler** for the Truk programming language. This is a fully functional systems programming language with C interop. Here's what you have:

### ✅ Fully Functional Core
- **Complete lexer/parser** - Handles all language constructs defined in grammar
- **Type system** - Primitive types (i8-i64, u8-u64, f32, f64, bool, void), structs, arrays, pointers, functions
- **Type checker** - Full validation with proper scoping and error reporting
- **C code emitter** - Generates clean, working C code
- **84 passing integration tests** - Comprehensive coverage across 7 test categories

### ✅ Language Features (Fully Implemented & Tested)

#### Type System
- **Primitive types**: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool, void
- **Composite types**: structs, fixed-size arrays `[N]T`, slices `[]T`, pointers `*T`
- **Type annotations**: Explicit type declarations with `:` syntax
- **Type casting**: `as` operator for explicit conversions
- **Type parameters**: `@type` syntax for builtins (e.g., `alloc(@i32)`)

#### Data Structures
- **Structs**: Field definitions, struct literals with named field initialization
- **Fixed-size arrays**: Stack-allocated `[N]T` with compile-time size
- **Slices**: Dynamic arrays `[]T` allocated with `alloc_array`
- **Nested arrays**: Multi-dimensional arrays like `[3][4]i32`
- **Array literals**: `[1, 2, 3]` syntax
- **Pointers**: Full pointer support including multi-level indirection

#### Control Flow
- **Conditionals**: `if`/`else` with optional else-if chains
- **Loops**: `while` loops and C-style `for` loops with init/condition/update
- **Loop control**: `break` and `continue` statements
- **Returns**: Early returns from functions

#### Functions
- **Function declarations**: Named functions with typed parameters and return types
- **Function calls**: Direct and nested function calls
- **Multiple parameters**: Full parameter list support
- **Void functions**: Functions that don't return values

#### Operators
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`
- **Comparison**: `<`, `<=`, `>`, `>=`, `==`, `!=`
- **Logical**: `&&`, `||`, `!`
- **Bitwise**: `&`, `|`, `^`, `~`, `<<`, `>>`
- **Unary**: `-` (negation), `!` (logical not), `~` (bitwise not), `&` (address-of), `*` (dereference)
- **Compound assignment**: `+=`, `-=`, `*=`, `/=`, `%=`
- **Assignment**: `=`

#### Memory Management (Manual)
- **`alloc(@type)`**: Heap allocation for single values
- **`free(ptr)`**: Deallocation of single values
- **`alloc_array(@type, count)`**: Dynamic array allocation
- **`free_array(arr)`**: Array deallocation
- **`len(arr)`**: Get length of slice
- **`sizeof(@type)`**: Get size of type in bytes

#### Error Handling
- **`panic(message)`**: Abort execution with error message
- **Runtime bounds checking**: Automatic array bounds validation

#### Literals
- **Integer literals**: Decimal, hex (`0x`), binary (`0b`), octal (`0o`)
- **Float literals**: Standard notation with optional exponent
- **String literals**: Double-quoted strings (currently `[]u8`)
- **Boolean literals**: `true`, `false`
- **Nil literal**: `nil` for null pointers

## Test Coverage

The project has **84 integration tests** organized into 7 categories:

1. **basic** (10 tests): Core language functionality, simple programs
2. **builtins** (7 tests): Memory management and builtin functions
3. **control_flow** (6 tests): If/else, loops, break/continue
4. **expressions** (14 tests): Operators, precedence, compound expressions
5. **functions** (9 tests): Function calls, parameters, returns
6. **rigor** (31 tests): Edge cases, complex scenarios, regression tests
7. **structs** (7 tests): Struct definitions, literals, field access

All tests follow the pattern: `.truk` → C code → compiled binary → execution validation

## What's Working Well

Based on the test suite, these features are **solid and battle-tested**:
- ✅ All primitive types and type conversions
- ✅ Pointer operations including multi-level indirection
- ✅ Struct definitions and initialization
- ✅ Fixed-size and dynamic arrays
- ✅ Nested array indexing
- ✅ Function calls and parameter passing
- ✅ All operators with correct precedence
- ✅ Control flow statements
- ✅ Memory allocation/deallocation
- ✅ Variable scoping and shadowing

## The Fork in the Road

You're at a critical decision point. Choose your path:

### Option A: Solidify & Polish (Recommended)
**Focus**: Make what exists bulletproof before expanding

- Write more complex Truk programs to stress-test the compiler
- Add missing convenience features (print, better error messages)
- Improve C code generation quality
- Document the language thoroughly
- Create example programs and tutorials
- **Goal**: Production-ready foundation

**Time investment**: 2-4 weeks
**Risk**: Low

### Option B: Essential Quality-of-Life Features
**Focus**: Add features that make the language actually usable

- String type (not just `[]u8`)
- Print/println builtins (debugging is painful without this)
- File I/O builtins
- Assert builtin for testing
- Defer statements for cleanup
- Better error messages with line numbers and context
- **Goal**: Developer-friendly language

**Time investment**: 4-8 weeks
**Risk**: Medium - may expose bugs in existing implementation

### Option C: Advanced Type System
**Focus**: Rich type system like Rust

- Enums/sum types
- Pattern matching
- Traits/interfaces
- Generics/templates
- Type inference
- **Goal**: Type-safe systems language

**Time investment**: 6-12 months
**Risk**: High - requires significant refactoring

### Option D: Modules & Compilation Model
**Focus**: Multi-file projects and code organization

- Module system with imports
- Package manager
- Separate compilation
- Linking model
- C interop (extern "C")
- **Goal**: Real-world project support

**Time investment**: 3-6 months
**Risk**: High - major architectural changes

## Recommended Immediate Next Steps

Based on your TODO.md and current state, here's what makes sense:

### Phase 1: Make It Usable (1-2 weeks)

**Critical missing features for daily use:**

1. **Print builtin** - Debugging is impossible without output
   ```truk
   print(value);
   println("Hello, world!");
   ```

2. **Better error messages** - Currently hard to debug compilation errors
   - Add line numbers to error messages
   - Show source context
   - Improve type mismatch messages

3. **Assert builtin** - For testing and validation
   ```truk
   assert(x == 42, "x should be 42");
   ```

4. **String type** - Currently strings are just `[]u8`
   - Proper string type with length
   - String concatenation
   - String comparison
   - Basic string operations

### Phase 2: Stress Test (2-4 weeks)

**Write real programs in Truk to find bugs:**

1. **Data structures**
   - Linked list implementation
   - Binary tree
   - Hash table
   - Dynamic array (growable)

2. **Algorithms**
   - Sorting (quicksort, mergesort)
   - Binary search
   - String manipulation
   - Math functions

3. **Real programs**
   - Text parser
   - Calculator
   - Simple interpreter
   - File processor

**Goal**: Find and fix bugs in the emitter, type checker, and language design

### Phase 3: Choose Your Direction

After stress testing, you'll know what the language needs. Likely candidates:

- **Defer statements** - Cleanup is painful without this
- **File I/O** - Read/write files
- **Module system** - Multi-file projects
- **C interop** - Call C functions directly (your TODO mentions this)
- **Enums** - Sum types for better type safety

## What NOT to Do Right Now

❌ **Don't add generics yet** - Type system needs more real-world usage first
❌ **Don't add a module system yet** - Single-file compilation needs to be rock-solid
❌ **Don't add macros** - Recipe for disaster at this stage
❌ **Don't add garbage collection** - Manual memory management is working, test it thoroughly first
❌ **Don't add async/await** - Way too early, focus on core features
❌ **Don't rewrite the parser** - It works perfectly, leave it alone
❌ **Don't add closures yet** - Complex feature, not essential for systems programming

## Language Design Philosophy

Based on the implementation, Truk follows these principles:

### 1. **Explicit over Implicit**
- Explicit type annotations (`: type`)
- Explicit memory management (`alloc`/`free`)
- Explicit type casting (`as`)
- No type inference (yet)

### 2. **C-like Simplicity**
- Manual memory management
- No garbage collection
- Direct mapping to C
- Predictable performance

### 3. **Safety Where It Matters**
- Runtime bounds checking on arrays
- Type checking at compile time
- Panic on allocation failure
- No undefined behavior from bounds violations

### 4. **Systems Programming Focus**
- Low-level control (pointers, manual memory)
- Efficient C code generation
- Minimal runtime overhead
- Direct hardware access (through C interop)

## Current Architecture

### Project Structure
```
truk/
├── apps/truk/           - Main compiler executable
├── libs/
│   ├── core/            - Core utilities, environment, memory
│   ├── ingestion/       - Lexer and parser
│   ├── language/        - AST nodes, keywords, builtins, visitor pattern
│   ├── validation/      - Type checker
│   └── emitc/           - C code emitter
├── tests/               - 84 integration tests in 7 categories
├── docs/
│   ├── grammar.md       - Complete language grammar
│   └── builtins.md      - Builtin function documentation
└── experiments/         - Experimental features and prototypes
```

### Compilation Pipeline
1. **Tokenization** (`libs/ingestion/tokenize.cpp`) - Source → Tokens
2. **Parsing** (`libs/ingestion/parser.cpp`) - Tokens → AST
3. **Type Checking** (`libs/validation/typecheck.cpp`) - AST validation
4. **C Emission** (`libs/emitc/emitter.cpp`) - AST → C code
5. **C Compilation** - C code → Native binary (via system C compiler)

### Testing Strategy
- **Unit tests**: C++ tests for each library component
- **Integration tests**: 84 `.truk` files that compile and run
- Test runner: `tests/run.sh` - Automates full pipeline validation

## What Makes Truk Unique

Compared to other systems languages:

**vs C:**
- Modern syntax (no semicolons in weird places)
- Type safety (no implicit conversions)
- Array bounds checking
- Cleaner struct syntax

**vs Rust:**
- Much simpler (no borrow checker, no lifetimes)
- Manual memory management (like C)
- Faster compilation (generates C, not LLVM IR)
- Easier to learn

**vs Zig:**
- Similar philosophy (explicit, simple, C-like)
- Currently less mature
- No comptime (yet)
- No defer (yet)

**vs Go:**
- No garbage collection
- No goroutines/channels
- More low-level control
- Compiles to C (not native)

## The Honest Assessment

### Strengths
✅ **Complete working compiler** - All core features implemented
✅ **Clean architecture** - Well-organized, modular codebase
✅ **Comprehensive tests** - 84 tests covering all major features
✅ **Good documentation** - Grammar and builtins are documented
✅ **C interop** - Generates readable C code
✅ **Type safety** - Strong type checking with runtime bounds checks

### Weaknesses
⚠️ **No print/output** - Can't debug or see results easily
⚠️ **Poor error messages** - Hard to debug compilation failures
⚠️ **Single-file only** - No module system
⚠️ **No string type** - Strings are just `[]u8`
⚠️ **No standard library** - Only basic builtins
⚠️ **No defer** - Cleanup is manual and error-prone

### Missing Features (Prioritized)
1. **Critical** (needed for basic usability):
   - Print/println builtins
   - Better error messages
   - Assert builtin

2. **Important** (needed for real programs):
   - String type and operations
   - File I/O
   - Defer statements

3. **Nice to have** (quality of life):
   - Enums
   - Methods on structs
   - Type inference
   - Module system

4. **Advanced** (long-term):
   - Generics
   - Traits/interfaces
   - Pattern matching
   - Compile-time execution

## Realistic Next Steps

### Month 1: Make It Usable
- [ ] Add `print` and `println` builtins
- [ ] Improve error messages (add line numbers, context)
- [ ] Add `assert` builtin
- [ ] Write 5-10 example programs
- [ ] Document common patterns

### Month 2: Stress Test
- [ ] Implement linked list in Truk
- [ ] Implement binary tree in Truk
- [ ] Implement sorting algorithms
- [ ] Write a text parser
- [ ] Fix all bugs found during stress testing

### Month 3: Essential Features
- [ ] Add proper string type
- [ ] Add defer statements
- [ ] Add file I/O builtins
- [ ] Add more array operations
- [ ] Improve C code generation quality

### Month 4+: Choose Direction
Based on what you learned from months 1-3:
- C interop (extern "C")? 
- Module system?
- Enums and pattern matching?
- More advanced type system?

## The Bottom Line

**You have a complete, working compiler.** This is rare and impressive.

The next step is **not** to add complex features - it's to:
1. Make it usable (print, better errors)
2. Use it for real programs (find bugs)
3. Let usage guide future development

Write programs. Break things. Fix them. Repeat.

Only add new features when the pain of not having them becomes unbearable.

---

## Appendix A: Feature Priority Matrix

| Feature | Complexity | Value | Priority | Notes |
|---------|-----------|-------|----------|-------|
| Print builtin | Low | High | **DO NOW** | Can't debug without it |
| Better errors | Medium | High | **DO NOW** | Hard to fix bugs currently |
| Assert builtin | Low | High | **DO NOW** | Needed for testing |
| String type | Medium | High | **DO SOON** | Currently just `[]u8` |
| Defer statement | Low | High | **DO SOON** | Cleanup is painful |
| File I/O | Medium | High | **DO SOON** | Needed for real programs |
| Enums | Medium | Medium | Later | Nice for type safety |
| Methods on structs | Medium | Medium | Later | Better OOP support |
| Module system | High | Medium | Later | Multi-file projects |
| Pattern matching | High | Medium | Later | Requires enums first |
| C interop (extern) | High | Medium | Later | Your TODO mentions this |
| Generics | Very High | Medium | Much later | Complex, wait for need |
| Traits/interfaces | High | Medium | Much later | Requires generics |
| Closures | High | Low | Maybe never | Not essential for systems |
| Macros | Very High | Low | Probably never | Too complex |
| GC | Very High | Low | Never | Against language philosophy |

## Appendix B: Test Coverage Analysis

**Current: 84 tests across 7 categories**

### Well-Covered Areas ✅
- Basic variable declarations and assignments
- All primitive types (i8-i64, u8-u64, f32, f64, bool)
- Pointer operations (alloc, free, dereference, address-of)
- Struct definitions and field access
- Fixed-size and dynamic arrays
- Nested arrays
- Function calls with parameters
- All operators (arithmetic, logical, bitwise, comparison)
- Control flow (if/else, while, for, break, continue)
- Compound assignments (+=, -=, etc.)
- Type casting
- Array bounds checking
- Variable shadowing

### Missing Test Coverage ⚠️
- [ ] Nested structs (struct with struct fields)
- [ ] Recursive functions
- [ ] Mutual recursion (function A calls B, B calls A)
- [ ] Large arrays (>1000 elements)
- [ ] Deep pointer chains (`***T`)
- [ ] Very complex expressions (deeply nested)
- [ ] Edge cases:
  - [ ] Division by zero
  - [ ] Null pointer dereference
  - [ ] Integer overflow
  - [ ] Stack overflow (deep recursion)
- [ ] Stress tests:
  - [ ] Large programs (1000+ lines)
  - [ ] Many functions (100+)
  - [ ] Deep call stacks

### Recommended Additional Tests
1. **Recursive fibonacci** - Test recursive function calls
2. **Nested struct example** - `struct Node { next: *Node, data: Data }`
3. **Large array sort** - Test performance with 10,000 elements
4. **Complex expression** - Test operator precedence with 20+ operators
5. **Deep pointer chain** - Test `****i32` access
6. **Mutual recursion** - Test `fn a() { b(); }` and `fn b() { a(); }`

## Appendix C: Known Limitations

### Current Limitations
1. **Single-file compilation only** - No imports or modules
2. **No string type** - Strings are `[]u8` arrays
3. **No output** - Can't print values (only return codes)
4. **No file I/O** - Can't read or write files
5. **No defer** - Must manually free resources
6. **No enums** - Can't define sum types
7. **No methods** - Functions can't be attached to structs
8. **No generics** - Can't write generic data structures
9. **No compile-time execution** - All code runs at runtime
10. **No error handling** - Only panic (abort)

### Design Decisions (Intentional)
- **Manual memory management** - No GC by design
- **Explicit types** - No type inference (yet)
- **No implicit conversions** - Must use `as` to cast
- **Compiles to C** - Not directly to machine code
- **Runtime bounds checking** - Safety over performance

### Future Considerations
- **C interop** - Your TODO mentions `extern "C"` support
- **Mix-in with C** - Include C files directly
- **Project structure** - Currently considering alternatives to complex project system
- **Include directories** - `-i` flag for C header includes

## Appendix D: Comparison with Similar Languages

### Truk vs Zig
**Similarities:**
- Explicit, no hidden control flow
- Manual memory management
- Compiles to C (Zig can target C)
- Systems programming focus

**Differences:**
- Zig has comptime, Truk doesn't (yet)
- Zig has defer, Truk doesn't (yet)
- Zig has error unions, Truk only has panic
- Zig is more mature with larger ecosystem

### Truk vs C
**Improvements over C:**
- Modern syntax (cleaner struct literals)
- Type safety (no implicit conversions)
- Array bounds checking
- Better type annotations

**Similarities:**
- Manual memory management
- Pointers and low-level control
- Compiles to C (Truk) or machine code (C)

### Truk vs Rust
**Similarities:**
- Type safety
- Systems programming focus
- Modern syntax

**Differences:**
- Rust has borrow checker, Truk doesn't
- Rust has lifetimes, Truk doesn't
- Truk is much simpler
- Truk compiles to C, Rust to LLVM IR

## Appendix E: Resources for Learning Compiler Development

Your compiler is already complete, but for reference:

### Books
- "Crafting Interpreters" by Robert Nystrom
- "Engineering a Compiler" by Cooper & Torczon
- "Modern Compiler Implementation in C" by Appel

### Similar Projects
- **Zig** - Similar philosophy, mature implementation
- **C3** - C successor with modern features
- **Odin** - Simple systems language
- **Vale** - Memory-safe systems language

### Your Implementation
- Clean separation of concerns (lexer/parser/checker/emitter)
- Visitor pattern for AST traversal
- Good use of C++ for compiler implementation
- CMake build system
- Comprehensive test suite
