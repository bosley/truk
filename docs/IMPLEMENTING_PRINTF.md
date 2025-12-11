# Implementing Printf Builtin for Truk

This guide walks through implementing a full-featured `printf` builtin with variadic arguments.

## Overview

Adding `printf` requires changes to 3 core components:
1. **Builtin Registry** - Define the builtin signature
2. **Type Checker** - Validate printf calls
3. **C Emitter** - Generate C printf calls

## Challenge: Variadic Arguments

Updating the entire code base to support variadic functions in the type system
and EVERYWHERE `NOT JUST PRINTF` the VERY first task is to get variadics
working and THEN printf can start, and then elverage the new variadic definitions
to maintain consistency

This guide covers **Option A** - true variadic printf.

## Architecture Overview

```
printf("value: %d\n", x)
         ↓
    Parser (already handles this)
         ↓
    Type Checker (validate format string + args)
         ↓
    C Emitter (emit: printf("value: %d\n", x))
```

## Step 1: Add Printf to Builtin Registry

**File:** `libs/language/include/language/builtins.hpp`

Add to `builtin_kind_e` enum:

```cpp
enum class builtin_kind_e {
  ALLOC,
  FREE,
  ALLOC_ARRAY,
  FREE_ARRAY,
  LEN,
  SIZEOF,
  PANIC,
  PRINTF        // Add this
};
```

**File:** `libs/language/src/builtins.cpp`

Add the printf signature builder:

```cpp
static type_ptr build_printf_signature(const type_c *type_param) {
  std::vector<type_ptr> params;

  auto u8_type = std::make_unique<primitive_type_c>(keywords_e::U8, 0);
  auto format_param =
      std::make_unique<array_type_c>(0, std::move(u8_type), std::nullopt);
  params.push_back(std::move(format_param));

  auto return_type = std::make_unique<primitive_type_c>(keywords_e::VOID, 0);

  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}
```

Add to `builtin_registry`:

```cpp
static std::vector<builtin_signature_s> builtin_registry = {
    {"alloc", builtin_kind_e::ALLOC, true, {}, build_alloc_signature},
    {"free", builtin_kind_e::FREE, false, {"ptr"}, build_free_signature},
    {"alloc_array",
     builtin_kind_e::ALLOC_ARRAY,
     true,
     {"count"},
     build_alloc_array_signature},
    {"free_array",
     builtin_kind_e::FREE_ARRAY,
     false,
     {"arr"},
     build_free_array_signature},
    {"len", builtin_kind_e::LEN, false, {"arr"}, build_len_signature},
    {"sizeof", builtin_kind_e::SIZEOF, true, {}, build_sizeof_signature},
    {"panic",
     builtin_kind_e::PANIC,
     false,
     {"message"},
     build_panic_signature},
    {"printf",                           // Add this
     builtin_kind_e::PRINTF,
     false,
     {"format"},
     build_printf_signature}
};
```

## Step 2: Update Type Checker for Variadic Support

**File:** `libs/validation/src/typecheck.cpp`

Modify `validate_builtin_call` to handle variadic printf:

```cpp
void type_checker_c::validate_builtin_call(const call_c &node,
                                           const type_entry_s &func_type) {
  if (!func_type.builtin_kind.has_value()) {
    report_error("Internal error: builtin has no kind", node.source_index());
    return;
  }

  const auto *builtin = language::builtins::lookup_builtin(func_type.name);
  if (!builtin) {
    report_error("Internal error: builtin not found in registry",
                 node.source_index());
    return;
  }

  std::size_t expected_arg_start = 0;
  const type_c *type_param = nullptr;

  if (builtin->takes_type_param) {
    if (node.arguments().empty()) {
      report_error("Builtin '" + builtin->name + "' requires a type parameter",
                   node.source_index());
      return;
    }

    const auto *first_arg_type_param =
        dynamic_cast<const type_param_c *>(node.arguments()[0].get());
    if (!first_arg_type_param) {
      report_error("Builtin '" + builtin->name +
                       "' requires a type parameter (use @type syntax)",
                   node.source_index());
      return;
    }

    type_param = first_arg_type_param->type();
    expected_arg_start = 1;
  }

  // Special handling for printf (variadic)
  if (builtin->kind == language::builtins::builtin_kind_e::PRINTF) {
    if (node.arguments().empty()) {
      report_error("printf requires at least a format string",
                   node.source_index());
      return;
    }

    // Validate first argument is []u8 (format string)
    node.arguments()[0]->accept(*this);
    if (_current_expression_type) {
      if (_current_expression_type->kind != type_kind_e::ARRAY ||
          !_current_expression_type->element_type ||
          _current_expression_type->element_type->name != "u8") {
        report_error("printf format string must be []u8", node.source_index());
      }
    }

    // Type check remaining arguments (variadic args)
    // We validate they're printable types
    for (std::size_t i = 1; i < node.arguments().size(); ++i) {
      node.arguments()[i]->accept(*this);
      
      if (_current_expression_type) {
        // Ensure argument is a printable type
        bool is_printable = false;
        
        if (_current_expression_type->kind == type_kind_e::PRIMITIVE) {
          is_printable = true;
        } else if (_current_expression_type->kind == type_kind_e::POINTER) {
          is_printable = true;
        } else if (_current_expression_type->kind == type_kind_e::ARRAY &&
                   _current_expression_type->element_type &&
                   _current_expression_type->element_type->name == "u8") {
          is_printable = true;
        }
        
        if (!is_printable) {
          report_error("printf argument " + std::to_string(i) + 
                      " has unprintable type", node.source_index());
        }
      }
    }

    // Set return type to void
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::VOID_TYPE, "void");
    return;
  }

  // Rest of the existing validation for non-variadic builtins
  auto signature = builtin->build_signature(type_param);

  if (!signature) {
    report_error("Failed to build signature for builtin '" + builtin->name +
                     "'",
                 node.source_index());
    return;
  }

  auto *func_sig = dynamic_cast<const function_type_c *>(signature.get());
  if (!func_sig) {
    report_error("Internal error: builtin signature is not a function type",
                 node.source_index());
    return;
  }

  std::size_t expected_param_count = func_sig->param_types().size();
  std::size_t actual_arg_count = node.arguments().size() - expected_arg_start;

  if (actual_arg_count != expected_param_count) {
    report_error("Builtin '" + builtin->name + "' expects " +
                     std::to_string(expected_param_count) +
                     " argument(s) but got " + std::to_string(actual_arg_count),
                 node.source_index());
    return;
  }

  for (std::size_t i = 0; i < expected_param_count; ++i) {
    node.arguments()[expected_arg_start + i]->accept(*this);

    auto expected_type = resolve_type(func_sig->param_types()[i].get());
    if (!expected_type) {
      report_error("Failed to resolve parameter type for builtin",
                   node.source_index());
      continue;
    }

    bool type_matches = false;
    if (_current_expression_type) {
      if (types_equal(_current_expression_type.get(), expected_type.get())) {
        type_matches = true;
      } else if (expected_type->kind == type_kind_e::POINTER &&
                 expected_type->name == "void" &&
                 _current_expression_type->kind == type_kind_e::POINTER) {
        type_matches = true;
      } else if (expected_type->kind == type_kind_e::ARRAY &&
                 expected_type->element_type &&
                 expected_type->element_type->name == "void" &&
                 _current_expression_type->kind == type_kind_e::ARRAY &&
                 expected_type->array_size ==
                     _current_expression_type->array_size) {
        type_matches = true;
      }
    }

    if (_current_expression_type && !type_matches) {
      report_error("Argument type mismatch in builtin '" + builtin->name + "'",
                   node.source_index());
    }
  }

  auto return_type = resolve_type(func_sig->return_type());
  if (return_type) {
    _current_expression_type = std::move(return_type);
  } else {
    _current_expression_type.reset();
  }
}
```

## Step 3: Add C Emission for Printf

**File:** `libs/emitc/src/emitter.cpp`

Add printf case to the `visit(const call_c &node)` method:

```cpp
void emitter_c::visit(const call_c &node) {
  if (auto ident = dynamic_cast<const identifier_c *>(node.callee())) {
    const std::string &func_name = ident->id().name;
    auto builtin = builtins::lookup_builtin(func_name);

    if (builtin) {
      switch (builtin->kind) {
      case builtins::builtin_kind_e::ALLOC: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::FREE: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::ALLOC_ARRAY: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::FREE_ARRAY: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::LEN: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::SIZEOF: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::PANIC: {
        // ... existing code ...
        break;
      }
      case builtins::builtin_kind_e::PRINTF: {
        if (node.arguments().empty()) {
          break;
        }

        // Emit format string
        std::stringstream format_stream;
        std::swap(format_stream, _current_expr);
        node.arguments()[0]->accept(*this);
        std::string format_arg = _current_expr.str();
        std::swap(format_stream, _current_expr);

        // Start printf call
        _current_expr << "printf(\"%.*s\"";

        // Emit length for format string (for %.*s)
        _current_expr << ", (int)(" << format_arg << ").len";
        _current_expr << ", (const char*)(" << format_arg << ").data";

        // Emit variadic arguments
        for (size_t i = 1; i < node.arguments().size(); ++i) {
          _current_expr << ", ";
          
          std::stringstream arg_stream;
          std::swap(arg_stream, _current_expr);
          node.arguments()[i]->accept(*this);
          std::string arg = _current_expr.str();
          std::swap(arg_stream, _current_expr);
          
          _current_expr << arg;
        }

        _current_expr << ")";

        if (!_in_expression) {
          _functions << cdef::indent(_indent_level) << _current_expr.str()
                     << ";\n";
          _current_expr.str("");
          _current_expr.clear();
        }
        return;
      }
      }
    }
  }

  // ... rest of existing code for regular function calls ...
}
```

## Usage Examples

### Basic Usage

```truk
fn main() : i32 {
  var x: i32 = 42;
  printf("Hello, world!\n");
  printf("The answer is: %d\n", x);
  return 0;
}
```

### Multiple Arguments

```truk
fn main() : i32 {
  var a: i32 = 10;
  var b: i32 = 20;
  var c: f64 = 3.14;
  
  printf("a=%d, b=%d, sum=%d\n", a, b, a + b);
  printf("pi=%f\n", c);
  
  return 0;
}
```

### With Pointers

```truk
fn main() : i32 {
  var ptr: *i32 = alloc(@i32);
  *ptr = 99;
  
  printf("Value: %d\n", *ptr);
  printf("Address: %p\n", ptr);
  
  free(ptr);
  return 0;
}
```

### With Arrays

```truk
fn main() : i32 {
  var arr: []i32 = alloc_array(@i32, 5);
  arr[0] = 100;
  arr[1] = 200;
  
  printf("arr[0]=%d, arr[1]=%d\n", arr[0], arr[1]);
  printf("Array length: %llu\n", len(arr));
  
  free_array(arr);
  return 0;
}
```

### Format Specifiers Reference

Users will need to use C format specifiers:

| Truk Type | Format Specifier | Example |
|-----------|------------------|---------|
| i8, i16, i32 | `%d` | `printf("%d\n", x)` |
| i64 | `%lld` | `printf("%lld\n", x)` |
| u8, u16, u32 | `%u` | `printf("%u\n", x)` |
| u64 | `%llu` | `printf("%llu\n", x)` |
| f32, f64 | `%f` | `printf("%f\n", x)` |
| bool | `%d` | `printf("%d\n", x)` (0 or 1) |
| *T | `%p` | `printf("%p\n", ptr)` |
| []u8 | `%.*s` | `printf("%.*s\n", (int)s.len, s.data)` |

## Testing

Create test file: `tests/builtins/truk_builtins_printf_0.truk`

```truk
fn main() : i32 {
  var x: i32 = 42;
  var y: i64 = 9999;
  var z: f64 = 3.14159;
  
  printf("Integer: %d\n", x);
  printf("Long: %lld\n", y);
  printf("Float: %f\n", z);
  printf("Multiple: %d %lld %f\n", x, y, z);
  
  return 0;
}
```

Expected output:
```
Integer: 42
Long: 9999
Float: 3.141590
Multiple: 42 9999 3.141590
```
