# Truk Language Evolution - Critical Design Decisions

## Architecture Vision
- Transpile to C (cfront model)
- C++ runtime as host process (inverted control flow)
- User code compiles to C ABI library loaded by runtime
- Runtime manages memory, modules, lifecycle
- Project structure: solution-like with cmds/, kits/, tests/

## Critical Landmines to Avoid (Lessons from C++)

### 1. Implicit Conversions
- **NO implicit numeric conversions** - `var x: i64 = 42` should fail, require `42 as i64`
- **NO implicit pointer conversions** except `nil` to any pointer
- **NO array decay** - `[5]i32` and `[]i32` are forever different types
- Explicit `as` casts only

### 2. The `const` Problem
- Consider killing `const` entirely for now
- If kept: make it deep and enforced (immutability all the way down)
- No escape hatches like `const_cast`
- Alternative: add `immut` later with proper semantics

### 3. Struct Purity
- **Structs are data forever** - no methods, constructors, inheritance
- Guarantee C ABI compatibility
- Functions are separate from data
- Keep structs as POD (plain old data)

### 4. No References
- **Never add references** (`T&` style)
- Pointers are pointers, can be `nil`
- Pass-by-reference is explicit: `fn modify(p: *i32)`
- No hidden dereferencing

### 5. Module System (Future)
- **No textual inclusion** - semantic imports only
- **No preprocessor** - ever, not even `#ifdef`
- **Order independence** - forward declarations handled automatically
- Imports are compiled modules with stable ABI

### 6. Type Aliases (Future)
- Add `newtype` for opaque types (different types, explicit conversion)
- Skip transparent aliases
- Enables type-driven domain modeling

### 7. Operators
- **Operators for primitives only**
- **NO operator overloading** - ever
- Functions for everything else
- Maybe allow `==` and `!=` for structs (memberwise comparison)

### 8. Generics (Future)
- **Monomorphization only** - no template metaprogramming
- **Bounded by interfaces/traits** - explicit constraints
- Alternative: compile-time code generation with `@generate_for`
- Keep it simple, not Turing-complete

### 9. Error Handling
- **Errors are values** - return codes or Result types
- **Panic is for bugs** - unrecoverable programmer errors only
- **NO exceptions** - no invisible control flow
- Future: algebraic data types (enums with data)

### 10. Ownership Model (CRITICAL)
- Add ownership annotations to grammar NOW
- `owned *T` vs `borrowed *T` vs `shared *T`
- Track ownership in type system
- Runtime enforces violations (advantage over compile-time checking)
- This is the foundation - must be right from day one

### 11. Integer Literals
- Literals are untyped until assigned
- Require explicit type annotation or cast for ambiguity
- `var x: i32 = 42;` or `var y = 42 as i64;`

### 12. Null Safety (Future)
- Make pointers non-nullable by default: `*T` cannot be `nil`
- Nullable pointers explicit: `?*T` can be `nil`
- Option types: `Option<*T>` is `Some(ptr)` or `None`
- Runtime checks at unwrap, compiler enforces checking

## 2040 Vision

### AI-First Design
- Code written by LLMs, read by humans
- Explicit over implicit (greppable, analyzable)
- No invisible behavior

### Runtime Advantages
- Sandboxing and isolation
- Hot reload of modules
- Multi-tenancy (multiple programs in one runtime)
- Memory management without GC overhead

### Heterogeneous Targets
- Same language, different C backends
- CPU, GPU, FPGA via runtime configuration
- Module loading enables hardware abstraction

### Verifiability
- Simple grammar enables formal verification
- Ownership tracking enables safety proofs
- Runtime enforcement catches what compile-time misses

## Immediate Next Steps
1. Add ownership qualifiers to grammar
2. Lock down implicit conversion rules
3. Design kit/module system
4. Define runtime C ABI
5. Implement ownership tracking in runtime
