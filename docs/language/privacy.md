# Privacy in Truk

Truk provides file-scoped privacy using a simple naming convention: identifiers starting with an underscore (`_`) are private to the file where they're defined.

## Overview

Privacy in Truk is:
- **Convention-based**: Use `_` prefix for private identifiers
- **File-scoped**: Private items are accessible only within their defining file
- **Zero runtime cost**: Privacy is enforced at compile-time
- **C-compatible**: Private functions/globals become `static` in generated C

## Privacy Rules

### Private Struct Fields

Fields starting with `_` can only be accessed within the file where the struct is defined.

```truk
struct Connection {
  host: *u8,
  port: u16,
  _socket_fd: i32,
  _is_connected: bool
}

fn connection_new(host: *u8, port: u16) : Connection {
  var conn: Connection = Connection{
    host: host,
    port: port,
    _socket_fd: -1,
    _is_connected: false
  };
  
  conn._socket_fd = 42;
  
  return conn;
}

fn connection_is_alive(conn: *Connection) : bool {
  return conn._is_connected;
}
```

### Private Functions

Functions starting with `_` can only be called within the file where they're defined.

**In library mode**, private functions are:
- Emitted as `static` in the `.c` file
- **Not included** in the `.h` header file

```truk
fn _internal_helper(x: i32) : i32 {
  return x * 2;
}

fn public_api(x: i32) : i32 {
  return _internal_helper(x) + 1;
}
```

### Private Global Variables

Global variables starting with `_` can only be accessed within the file where they're defined.

**In library mode**, private globals are:
- Emitted as `static` in the `.c` file
- **Not included** in the `.h` header file

```truk
var _max_retries: u32 = 3;

fn get_max_retries() : u32 {
  return _max_retries;
}

fn set_max_retries(value: u32) : void {
  _max_retries = value;
}
```

## Access Control Table

| Item Type | Visibility | Same File | Different File | Library .h | Library .c |
|-----------|-----------|-----------|----------------|-----------|-----------|
| `public_field` | All files | ✅ Read/Write | ✅ Read/Write | ✅ In struct | ✅ In struct |
| `_private_field` | Defining file only | ✅ Read/Write | ❌ Compile Error | ✅ In struct | ✅ In struct |
| `public_function` | All files | ✅ Call | ✅ Call | ✅ Declared | ✅ Defined |
| `_private_function` | Defining file only | ✅ Call | ❌ Compile Error | ❌ Not included | ✅ Defined (static) |
| `public_global` | All files | ✅ Access | ✅ Access | ✅ Declared (extern) | ✅ Defined |
| `_private_global` | Defining file only | ✅ Access | ❌ Compile Error | ❌ Not included | ✅ Defined (static) |

## Complete Example

### Library File: `connection.truk`

```truk
struct Connection {
  host: *u8,
  port: u16,
  _socket_fd: i32,
  _is_connected: bool,
  _retry_count: u32
}

var _max_retries: u32 = 3;

fn _attempt_connect(conn: *Connection) : bool {
  conn._retry_count = conn._retry_count + 1;
  if conn._retry_count > _max_retries {
    return false;
  }
  conn._socket_fd = 42;
  conn._is_connected = true;
  return true;
}

fn _close_socket(conn: *Connection) : void {
  if conn._socket_fd > 0 {
    conn._socket_fd = -1;
  }
  conn._is_connected = false;
}

fn connection_new(host: *u8, port: u16) : Connection {
  var conn: Connection = Connection{
    host: host,
    port: port,
    _socket_fd: -1,
    _is_connected: false,
    _retry_count: 0
  };
  
  _attempt_connect(&conn);
  
  return conn;
}

fn connection_close(conn: *Connection) : void {
  _close_socket(conn);
}

fn connection_is_alive(conn: *Connection) : bool {
  return conn._is_connected;
}

fn connection_reconnect(conn: *Connection) : bool {
  _close_socket(conn);
  conn._retry_count = 0;
  return _attempt_connect(conn);
}
```

### Generated `connection.h` (Library Mode)

```c
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct Connection Connection;
struct Connection {
  u8* host;
  u16 port;
  i32 _socket_fd;
  bool _is_connected;
  u32 _retry_count;
};

Connection connection_new(u8* host, u16 port);
void connection_close(Connection* conn);
bool connection_is_alive(Connection* conn);
bool connection_reconnect(Connection* conn);
```

### Generated `connection.c` (Library Mode)

```c
#include "connection.h"

static u32 _max_retries = 3;

static bool _attempt_connect(Connection* conn) {
  conn->_retry_count = conn->_retry_count + 1;
  if (conn->_retry_count > _max_retries) {
    return false;
  }
  conn->_socket_fd = 42;
  conn->_is_connected = true;
  return true;
}

static void _close_socket(Connection* conn) {
  if (conn->_socket_fd > 0) {
    conn->_socket_fd = -1;
  }
  conn->_is_connected = false;
}

Connection connection_new(u8* host, u16 port) {
  Connection conn = (Connection){
    .host = host,
    .port = port,
    ._socket_fd = -1,
    ._is_connected = false,
    ._retry_count = 0
  };
  
  _attempt_connect(&conn);
  
  return conn;
}

void connection_close(Connection* conn) {
  _close_socket(conn);
}

bool connection_is_alive(Connection* conn) {
  return conn->_is_connected;
}

bool connection_reconnect(Connection* conn) {
  _close_socket(conn);
  conn->_retry_count = 0;
  return _attempt_connect(conn);
}
```

### Application File: `main.truk`

```truk
import "connection.truk";

fn main() : i32 {
  var conn: Connection = connection_new("localhost", 5432);
  
  var alive: bool = connection_is_alive(&conn);
  
  connection_close(&conn);
  
  return 0;
}
```

## Privacy Violations

Attempting to access private members from a different file results in compile errors:

```truk
import "connection.truk";

fn main() : i32 {
  var conn: Connection = connection_new("localhost", 5432);
  
  var fd: i32 = conn._socket_fd;
  
  _attempt_connect(&conn);
  
  _max_retries = 5;
  
  return 0;
}
```

**Compile Errors:**
```
error: Cannot access private field '_socket_fd' of struct 'Connection' from outside its defining file
error: Cannot call private function '_attempt_connect' from outside its defining file
error: Cannot access private global variable '_max_retries' from outside its defining file
```

## Best Practices

### 1. Use Privacy for Implementation Details

Make internal state and helper functions private:

```truk
struct Buffer {
  data: []u8,
  capacity: u64,
  _len: u64,
  _initialized: bool
}

fn _grow(buf: *Buffer) : void {
}

fn buffer_append(buf: *Buffer, value: u8) : void {
  if buf._len >= buf.capacity {
    _grow(buf);
  }
  buf.data[buf._len] = value;
  buf._len = buf._len + 1;
}
```

### 2. Provide Public Accessors

Don't expose private fields directly:

```truk
struct Pool {
  connections: []Connection,
  capacity: u32,
  _active_count: u32
}

fn pool_active_count(pool: *Pool) : u32 {
  return pool._active_count;
}
```

### 3. Keep Public API Minimal

Only expose what users need:

```truk
fn _validate_input(x: i32) : bool {
  return x > 0;
}

fn _process_internal(x: i32) : i32 {
  return x * 2;
}

fn process(x: i32) : i32 {
  if !_validate_input(x) {
    return -1;
  }
  return _process_internal(x);
}
```

## Design Philosophy

Truk's privacy system follows these principles:

1. **Simple Convention**: The `_` prefix is familiar to C programmers
2. **File-Scoped**: Natural boundaries that match compilation units
3. **Zero Cost**: Privacy is compile-time only for fields
4. **C-Level Hiding**: Private functions are truly `static` in C
5. **No OOP Baggage**: No classes, access modifiers, or inheritance
6. **Library-Friendly**: Clean headers with only public API

## Comparison to C

In C, you would use `static` for file-scoped functions and globals:

**C Code:**
```c
static int _internal_helper(int x) {
  return x * 2;
}

int public_api(int x) {
  return _internal_helper(x) + 1;
}
```

**Truk Code:**
```truk
fn _internal_helper(x: i32) : i32 {
  return x * 2;
}

fn public_api(x: i32) : i32 {
  return _internal_helper(x) + 1;
}
```

Truk generates the exact same C code, but provides additional compile-time checking for struct fields.
