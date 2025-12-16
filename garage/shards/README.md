# Shard System Demonstration

This example demonstrates Truk's shard-based privacy system with nested modules.

## Architecture

```
main.truk                    (no shard - imports ONLY top-level api.truk)
  │
  ├─ shard_set_a/
  │   ├─ api.truk            (shard "set_a_internal" - PUBLIC API FACADE)
  │   ├─ file1.truk          (shard "set_a_internal" - private implementation)
  │   └─ file2.truk          (shard "set_a_internal" - internal helpers)
  │
  └─ shard_set_b/
      ├─ api.truk            (shard "set_b_internal" - PUBLIC API FACADE)
      ├─ file1.truk          (shard "set_b_internal" - private impl, uses set_x API)
      ├─ file2.truk          (shard "set_b_internal" - internal helpers)
      └─ innser_set_x/
          ├─ api.truk        (shard "set_x_internal" - PUBLIC API FACADE)
          ├─ file1.truk      (shard "set_x_internal" - private implementation)
          ├─ file2.truk      (shard "set_x_internal" - internal helpers)
          └─ file3.truk      (shard "set_x_internal" - utilities)
```

**Import Chain:**
- `main.truk` imports `shard_set_a/api.truk` and `shard_set_b/api.truk`
- `shard_set_b/*` imports `innser_set_x/api.truk` (NOT internal files)
- `innser_set_x/*` imports each other (same shard)
- `shard_set_a/*` imports each other (same shard)
- `shard_set_b/*` imports each other (same shard)

## Privacy Flow

```
main.truk (NO SHARD)
    │
    ├─── imports ───> shard_set_a/api.truk (PUBLIC)
    │                      │
    │                      └─── uses (PRIVATE) ───> file1.truk, file2.truk
    │                                                (shard "set_a_internal")
    │
    └─── imports ───> shard_set_b/api.truk (PUBLIC)
                           │
                           └─── uses (PRIVATE) ───> file1.truk, file2.truk
                                                     (shard "set_b_internal")
                                                         │
                                                         └─── imports ───> innser_set_x/api.truk (PUBLIC)
                                                                                │
                                                                                └─── uses (PRIVATE) ───> file1.truk, file2.truk, file3.truk
                                                                                                         (shard "set_x_internal")
```

**Key Points:**
- Each shard has ONE `api.truk` file as its public interface
- Internal files within a shard can access each other's `_` members
- Shards use other shards' `api.truk` files (public API only)
- `main.truk` only imports top-level `api.truk` files
- Inner shards (set_x) are completely hidden from main

## Privacy Layers

### Layer 1: Shard Set X (Lowest Level - Logging)

**Shard:** `"set_x_internal"`

**Private Implementation:**
- `file1.truk`: Defines `LoggerX` struct with private fields `_enabled`, `_count`
- `file1.truk`: Private function `_format_message()` - handles actual printf calls
- `file1.truk`: Private global `_global_log_level`
- `file2.truk`: Private function `_adjust_log_level()` - modifies global state

**Public API:**
- `logger_x_create()` - Create logger
- `logger_x_log()` - Log message
- `logger_x_disable()` - Disable logger
- `logger_x_get_count()` - Get log count
- `logger_x_set_level()` - Set log level

**Access:**
- ✅ Files in `set_x_internal` can access each other's `_` members
- ❌ Files outside shard cannot access `_enabled`, `_count`, `_format_message()`, etc.

### Layer 2: Shard Set B (Middle Level - Service)

**Shard:** `"set_b_internal"`

**Private Implementation:**
- `file1.truk`: Defines `ServiceB` struct with private fields `_logger`, `_initialized`
- `file1.truk`: Private function `_init_service()` - uses LoggerX public API
- `file2.truk`: Private function `_log_operation()` - wraps logger calls

**Public API:**
- `service_b_create()` - Create service
- `service_b_execute()` - Execute operation
- `service_b_get_log_count()` - Get log count
- `service_b_shutdown()` - Shutdown service

**Access:**
- ✅ Files in `set_b_internal` can access each other's `_` members
- ✅ Files in `set_b_internal` can use LoggerX **public API**
- ❌ Files in `set_b_internal` CANNOT access LoggerX private members (different shard)
- ❌ Files outside shard cannot access `_logger`, `_initialized`, `_init_service()`, etc.

### Layer 3: Shard Set A (Middle Level - Data Processing)

**Shard:** `"set_a_internal"`

**Private Implementation:**
- `file1.truk`: Defines `DataA` struct with private field `_internal_state`
- `file1.truk`: Private functions `_compute_internal()`, `_validate_data()`
- `file2.truk`: Internal helper functions using private members

**Public API Facade (`api.truk`):**
- `api_create_data()` - Create data
- `api_process_data()` - Process data
- `api_get_value()` - Get value
- `api_reset_data()` - Reset state

**Access:**
- ✅ Files in `set_a_internal` can access each other's `_` members
- ✅ `api.truk` is the ONLY file main should import
- ❌ Files outside shard cannot access `_internal_state`, `_compute_internal()`, etc.

### Layer 4: Main (Top Level - Application)

**No Shard Declaration** (enforced by compiler)

**Imports ONLY api.truk files:**
- `import "shard_set_a/api.truk";`
- `import "shard_set_b/api.truk";`

**Access:**
- ✅ Can use public APIs through api.truk facades
- ❌ Cannot access ANY private members (no shard membership)
- ❌ Cannot declare shard (would be compile error)
- ❌ Should not import internal files (only api.truk)

## Key Demonstrations

### 1. Shard Isolation

`set_b_internal` uses `set_x_internal`'s **public API** but cannot access its private members:

```truk
// file1.truk (set_b_internal)
fn _init_service(svc: *ServiceB) : void {
  (*svc)._logger = logger_x_create(...);  // ✅ Public API
  logger_x_log(&(*svc)._logger, "...");   // ✅ Public API
  
  // (*svc)._logger._enabled = false;     // ❌ Would fail: different shard
  // _format_message(&(*svc)._logger, ""); // ❌ Would fail: different shard
}
```

### 2. Shard Encapsulation

`main.truk` cannot access private members from ANY shard:

```truk
// main.truk (no shard)
fn main() : i32 {
  var data: DataA = data_a_create(10);     // ✅ Public API
  var result: i32 = data_a_process(&data); // ✅ Public API
  
  // data._internal_state = 0;             // ❌ Would fail: not in shard
  // _compute_internal(5);                 // ❌ Would fail: not in shard
  
  var svc: ServiceB = service_b_create("MainService"); // ✅ Public API
  
  // svc._logger = ...;                    // ❌ Would fail: not in shard
  // _init_service(&svc);                  // ❌ Would fail: not in shard
}
```

### 3. Nested Module Pattern

`set_x_internal` is completely hidden from `main.truk`:
- `main.truk` never imports `innser_set_x` files
- `main.truk` only sees `ServiceB` API
- `ServiceB` internally uses `LoggerX`, but that's an implementation detail
- Perfect encapsulation!

## Building

```bash
cd garage/shards
../../build/apps/truk/truk main.truk -o shard_demo
./shard_demo
```

**Output:**
```
[MainService] Service initialized
[MainService] Executing operation
[MainService] Shutting down
```

**Exit code:** 90 (the computed result: 10 → 45 → 90)

## Privacy Violations

Try adding this to `main.truk`:

```truk
fn main() : i32 {
  var data: DataA = data_a_create(10);
  data._internal_state = 999;  // Try to access private field
  return 0;
}
```

**Compile Error:**
```
error: Cannot access private field '_internal_state' of struct 'DataA' 
       from outside its defining file or shard
```

Try adding shard to `main.truk`:

```truk
shard "set_a_internal";

fn main() : i32 {
  return 0;
}
```

**Compile Error:**
```
error: Shard declarations are not allowed in files containing a main function.
       Shards are for sharing implementation details between library files,
       not for application entry points
```

## Design Principles Demonstrated

1. **Explicit Trust** - Files must declare `shard "X"` to access internals
2. **Layered Encapsulation** - Each shard is isolated from others
3. **Public API Boundaries** - Shards use each other's public APIs
4. **No Shard Leaking** - Inner shards (set_x) hidden from outer consumers
5. **Application Isolation** - Main cannot join shards, must use public APIs
6. **API Facade Pattern** - Each shard has one `api.truk` file as the public interface
7. **Single Import Point** - Main only imports `api.truk` files, never internal files
