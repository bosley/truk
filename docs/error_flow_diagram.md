# TRUK Error Flow

## Command: `run`

```mermaid
flowchart TD
    A[run command] --> B[Create error_reporter_c]
    B --> C[Read file]
    C --> D[parser_c::parse]
    D -->|Error| E[report_parse_error]
    D -->|Success| F[type_checker_c::check]
    E --> Z[print_summary & exit 1]
    F -->|Error| G[report_typecheck_error]
    F -->|Success| H[emitter_c::finalize]
    G --> Z
    H -->|Error| I[report_emission_error]
    H -->|Success| J[tcc_compiler_c::compile_and_run]
    I --> Z
    J -->|Error| K[report_compilation_error]
    J -->|Success| L[Return exit_code]
    K --> Z
```

## Command: `compile`

```mermaid
flowchart TD
    A[compile command] --> B[Create error_reporter_c]
    B --> C[import_resolver_c::resolve]
    C -->|Error| D[report_parse_error OR report_import_error_with_type]
    C -->|Success| E[type_checker_c::check]
    D --> Z[print_summary & exit 1]
    E -->|Error| F[report_generic_error PHASE: TYPE_CHECKING]
    E -->|Success| G[emitter_c::finalize]
    F --> Z
    G -->|Error| H[report_generic_error PHASE: CODE_EMISSION]
    G -->|Success| I[Check has_main_function]
    H --> Z
    I -->|No main| J[report_generic_error]
    I -->|Has main| K[tcc_compiler_c::compile_string]
    J --> Z
    K -->|Error| L[report_compilation_error]
    K -->|Success| M[Success]
    L --> Z
```

## Command: `toc`

```mermaid
flowchart TD
    A[toc command] --> B[Create error_reporter_c]
    B --> C[import_resolver_c::resolve]
    C -->|Error| D[report_parse_error OR report_import_error_with_type]
    C -->|Success| E[type_checker_c::check]
    D --> Z[print_summary & exit 1]
    E -->|Error| F[report_generic_error PHASE: TYPE_CHECKING]
    E -->|Success| G[emitter_c::finalize]
    F --> Z
    G -->|Error| H[report_generic_error PHASE: CODE_EMISSION]
    G -->|Success| I[Determine assembly_type]
    H --> Z
    I --> J[emit_result.assemble]
    J --> K{LIBRARY or APPLICATION?}
    K -->|LIBRARY| L[Write .h and .c files]
    K -->|APPLICATION| M[Write .c file]
    L -->|Error| N[report_file_error]
    M -->|Error| N
    L -->|Success| O[Success]
    M -->|Success| O
    N --> Z
```

## Command: `tcc`

```mermaid
flowchart TD
    A[tcc command] --> B[Create error_reporter_c]
    B --> C[tcc_compiler_c::compile_file]
    C -->|Error| D[report_compilation_error]
    C -->|Success| E[Success]
    D --> Z[print_summary & exit 1]
```

## Error Reporter Methods

| Method | Parameters | Used By |
|--------|-----------|---------|
| `report_parse_error` | file, source, line, col, msg | run, compile, toc |
| `report_import_error_with_type` | file, msg, line, col, is_parse | compile, toc |
| `report_typecheck_error` | file, source, source_index, msg | run |
| `report_emission_error` | file, source, source_index, msg, ctx | run |
| `report_compilation_error` | msg | run, compile, tcc |
| `report_file_error` | file, msg | toc |
| `report_generic_error` | phase, msg | compile, toc |

## Error Phases

- `PARSING`
- `IMPORT_RESOLUTION`
- `TYPE_CHECKING`
- `CODE_EMISSION`
- `C_COMPILATION`
- `FILE_IO`
- `UNKNOWN`
