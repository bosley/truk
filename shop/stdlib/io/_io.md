# stdlib/io - Functional I/O Library

A functional, idiomatic truk library for I/O operations with proper error handling.

## Core Types

### `Result`
```truk
struct Result {
  value: i32,
  ok: bool
}
```

Represents the result of an I/O operation. Contains a value (bytes written/read or error code) and a success flag.

## Result Constructors

### `io_ok(value: i32) : Result`
Creates a successful Result with the given value.

### `io_err(value: i32) : Result`
Creates an error Result with the given error code.

## Result Predicates

### `io_is_ok(r: *Result) : bool`
Returns true if the Result represents success.

### `io_is_err(r: *Result) : bool`
Returns true if the Result represents an error.

## Result Extractors

### `io_unwrap(r: *Result) : i32`
Extracts the value from a Result. Use only when you're certain the operation succeeded.

### `io_unwrap_or(r: *Result, default_value: i32) : i32`
Extracts the value from a Result, or returns `default_value` if it's an error.

## Functional Combinators

### `io_and_then(r: *Result, f: fn(i32) : Result) : Result`
Chains operations. If `r` is successful, calls `f` with the value. Otherwise returns the error.

**Example with inline lambda:**
```truk
var r: Result = io_println("First");
r = io_and_then(&r, fn(n: i32) : Result {
  return io_println("Second (only if first succeeded)");
});
```

**Example with named function:**
```truk
fn print_second(n: i32) : Result {
  return io_println("Second");
}

var r: Result = io_println("First");
r = io_and_then(&r, print_second);
```

### `io_or_else(r: *Result, f: fn(i32) : Result) : Result`
Error recovery. If `r` is an error, calls `f` with the error code. Otherwise returns the success.

**Example with inline lambda:**
```truk
var r: Result = io_print("Might fail");
r = io_or_else(&r, fn(err: i32) : Result {
  io_eprintln("Recovering from error!");
  return io_ok(0);
});
```

### `io_map(r: *Result, f: fn(i32) : i32) : Result`
Transforms the value inside a successful Result using function `f`. Errors pass through unchanged.

**Example with inline lambda:**
```truk
var r: Result = io_print("Hello");
r = io_map(&r, fn(bytes: i32) : i32 {
  return bytes * 2;
});
```

## Standard I/O Functions

### `io_print(msg: *u8) : Result`
Prints a message to stdout without a newline.

### `io_println(msg: *u8) : Result`
Prints a message to stdout with a newline.

### `io_eprint(msg: *u8) : Result`
Prints a message to stderr without a newline.

### `io_eprintln(msg: *u8) : Result`
Prints a message to stderr with a newline.

### `io_write_stdout(buf: *u8, count: i32) : Result`
Writes `count` bytes from `buf` to stdout.

### `io_write_stderr(buf: *u8, count: i32) : Result`
Writes `count` bytes from `buf` to stderr.

### `io_read_stdin(buf: *u8, count: i32) : Result`
Reads up to `count` bytes from stdin into `buf`.

## File Operations

### `io_open(filename: *u8, mode: *u8) : (*u8, Result)`
Opens a file. Returns a tuple of (file handle, result).

**Modes:** `"r"` (read), `"w"` (write), `"a"` (append), `"r+"`, `"w+"`, `"a+"`

**Example:**
```truk
let f, open_result = io_open("file.txt", "w");
if io_is_err(&open_result) {
  return -1;
}
```

### `io_close(f: *u8) : Result`
Closes a file handle.

### `io_flush(f: *u8) : Result`
Flushes buffered data to disk.

### `io_write_file(f: *u8, buf: *u8, count: i32) : Result`
Writes `count` bytes from `buf` to file `f`.

### `io_read_file(f: *u8, buf: *u8, count: i32) : Result`
Reads up to `count` bytes from file `f` into `buf`.

## Higher-Order Functions

### `io_with_file(filename: *u8, mode: *u8, handler: fn(*u8) : Result) : Result`
Opens a file, calls `handler` with the file handle, and automatically closes the file when done.

**Example with inline lambda:**
```truk
var r: Result = io_with_file("test.txt", "w", fn(f: *u8) : Result {
  var data: *u8 = "Hello!\n";
  return io_write_file(f, data, 7);
});
```

**Example with named function:**
```truk
fn write_data(f: *u8) : Result {
  var data: *u8 = "Hello!\n";
  return io_write_file(f, data, 7);
}

var r: Result = io_with_file("test.txt", "w", write_data);
```

## Usage Patterns

### Basic Error Handling
```truk
var r: Result = io_println("Hello!");
if io_is_err(&r) {
  return -1;
}
```

### Chaining Operations
```truk
var r: Result = io_println("First");

r = io_and_then(&r, fn(n: i32) : Result {
  return io_println("Second (only if first succeeded)");
});

r = io_and_then(&r, fn(n: i32) : Result {
  return io_println("Third (only if second succeeded)");
});

return io_unwrap_or(&r, -1);
```

### Manual File Handling with Defer
```truk
let f, open_r = io_open("file.txt", "w");
if io_is_err(&open_r) {
  return -1;
}

defer {
  io_close(f);
}

var data: *u8 = "Content\n";
var write_r: Result = io_write_file(f, data, 8);
return io_unwrap_or(&write_r, -1);
```

### Automatic File Handling with Lambda
```truk
var r: Result = io_with_file("file.txt", "w", fn(f: *u8) : Result {
  var data: *u8 = "Content\n";
  return io_write_file(f, data, 8);
});
```

## Import

```truk
import "stdlib/io";
```
