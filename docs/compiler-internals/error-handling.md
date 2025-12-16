[← Back to Documentation Index](../start-here.md)

# Unified Error Handling System

**Compiler Internals:** [Error Flow Diagrams](error-flow-diagram.md) · [C Emitter](emitter.md) · [Type Checker](typechecker.md)

---

## Overview

The Truk compiler uses a unified error reporting system (`error_reporter_c`) that provides consistent, high-quality error messages across all compilation phases. This document describes where errors can occur and how they are handled.

## Error Sources in the Compilation Pipeline

### 1. Parser Errors (Ingestion Phase)

**Location**: `libs/ingestion/parser.cpp`

**Error Type**: `parse_error` exception → `parse_result_s`

**Information Available**:
- Error message
- Line number
- Column number
- Source text

**Example Errors**:
- Syntax errors (missing semicolons, braces, etc.)
- Unexpected tokens
- Malformed expressions
- Invalid declarations

**Reporting**:
```cpp
reporter.report_parse_error(filename, source, line, column, message);
```

### 2. Import Resolution Errors

**Location**: `libs/ingestion/import_resolver.cpp`

**Error Type**: `import_error_s` → `resolved_imports_s`

**Information Available**:
- File path
- Error message
- Line number (optional)
- Column number (optional)

**Example Errors**:
- File not found
- Circular import dependencies
- Import path resolution failures
- Parse errors in imported files

**Reporting**:
```cpp
reporter.report_import_error(file_path, message, line, column);
```

### 3. Type Checking Errors

**Location**: `libs/validation/typecheck.cpp`

**Error Type**: `type_error_s` with source index

**Information Available**:
- Error message
- Source index (byte offset in source)
- Automatically converted to line/column

**Example Errors**:
- Type mismatches
- Undefined identifiers
- Invalid operations on types
- Function signature mismatches
- Struct field access errors
- Array/pointer type errors

**Reporting**:
```cpp
reporter.report_typecheck_error(filename, source, source_index, message);
```

### 4. Code Emission Errors

**Location**: `libs/emitc/emitter.cpp`

**Error Type**: `error_s` with emission phase context

**Information Available**:
- Error message
- Source index
- Emission phase (collection, forward declaration, etc.)
- Node context (which function/struct)

**Example Errors**:
- Code generation failures
- Invalid C constructs
- Missing type information
- Unsupported language features

**Reporting**:
```cpp
reporter.report_emission_error(filename, source, source_index, message, phase_context);
```

### 5. C Compilation Errors (TCC)

**Location**: `libs/tcc/tcc.cpp`

**Error Type**: String error messages from TCC

**Information Available**:
- Error message from TCC compiler

**Example Errors**:
- C syntax errors in generated code
- Linker errors
- Missing libraries
- Symbol resolution failures

**Reporting**:
```cpp
reporter.report_compilation_error(message);
```

### 6. File I/O Errors

**Location**: Various (file operations)

**Error Type**: File operation failures

**Information Available**:
- File path
- Error message

**Example Errors**:
- Cannot read input file
- Cannot write output file
- Permission denied
- Disk full

**Reporting**:
```cpp
reporter.report_file_error(file_path, message);
```

## Error Display Features

### Source Context Display

For errors with source location information (parser, typecheck, emission), the error reporter uses `error_display_c` to show:

1. **Error header** with severity and location
2. **Source context** showing lines around the error
3. **Visual indicator** (caret) pointing to the exact error location
4. **Color coding** (when terminal supports it)

Example output:
```
error: Type mismatch in variable initialization
  --> example.truk:5:10
   |
 5 |   var x: i32 = "hello";
   |              ^
```

### Error Summary

After reporting all errors, the reporter can print a summary:
```cpp
reporter.print_summary();
```

Output:
```
Compilation failed with 3 error(s)
```

## Usage in Command Implementations

All command entry points (`run`, `compile`, `toc`, `tcc`) follow this pattern:

```cpp
int command(const options_s &opts) {
  core::error_reporter_c reporter;
  
  // ... perform compilation steps ...
  
  if (error_occurred) {
    reporter.report_*_error(...);
    reporter.print_summary();
    return 1;
  }
  
  return 0;
}
```

## Error Phase Tracking

The error reporter tracks which phase of compilation each error occurred in:

- `PARSING` - Tokenization and syntax analysis
- `IMPORT_RESOLUTION` - Import dependency resolution
- `TYPE_CHECKING` - Semantic analysis and type validation
- `CODE_EMISSION` - C code generation
- `C_COMPILATION` - TCC compilation of generated C
- `FILE_IO` - File read/write operations
- `UNKNOWN` - Unclassified errors

This allows for:
- Better error categorization
- Debugging compiler issues
- Understanding where compilation failed

## Design Principles

1. **Consistency**: All errors use the same reporting interface
2. **Context**: Errors show relevant source code when possible
3. **Clarity**: Error messages are descriptive and actionable
4. **Completeness**: All errors are collected and reported
5. **User-friendly**: Visual indicators and color coding improve readability
