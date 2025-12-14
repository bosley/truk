# Pattern Matching & String Parsing Primitives

This document outlines the minimal primitives needed for efficient string parsing and pattern matching in truk.

## String/Slice Builtins

These three builtins provide the foundation for efficient byte stream and string parsing.

### `starts_with(haystack: []u8, needle: []u8) -> bool`

Check if a slice begins with a specific prefix.

**Signature:**
```truk
builtin fn starts_with(haystack: []u8, needle: []u8) -> bool
```

**C Implementation:**
```c
bool starts_with(truk_slice_u8 haystack, truk_slice_u8 needle) {
  if (needle.len > haystack.len) return false;
  return memcmp(haystack.data, needle.data, needle.len) == 0;
}
```

**Example:**
```truk
var line: []u8 = "GET /index.html";
var get_prefix: []u8 = "GET ";

if starts_with(line, get_prefix) {
  // Handle GET request
}
```

---

### `ends_with(haystack: []u8, needle: []u8) -> bool`

Check if a slice ends with a specific suffix.

**Signature:**
```truk
builtin fn ends_with(haystack: []u8, needle: []u8) -> bool
```

**C Implementation:**
```c
bool ends_with(truk_slice_u8 haystack, truk_slice_u8 needle) {
  if (needle.len > haystack.len) return false;
  return memcmp(haystack.data + (haystack.len - needle.len), 
                needle.data, needle.len) == 0;
}
```

**Example:**
```truk
var filename: []u8 = "document.txt";
var txt_suffix: []u8 = ".txt";

if ends_with(filename, txt_suffix) {
  // Handle text file
}
```

---

### `substr(input: []u8, start: u64, length: u64) -> []u8`

Extract a substring/sub-slice from a slice. Returns a new slice struct pointing into the original data (zero-copy).

**Signature:**
```truk
builtin fn substr(input: []u8, start: u64, length: u64) -> []u8
```

**C Implementation:**
```c
truk_slice_u8 substr(truk_slice_u8 input, u64 start, u64 length) {
  truk_bounds_check(start, input.len);
  truk_bounds_check(start + length - 1, input.len);
  return (truk_slice_u8){input.data + start, length};
}
```

**Example:**
```truk
var line: []u8 = "GET /index.html";
var path: []u8 = substr(line, 4, line.len - 4);
```

---

## Match Statement

A minimal `match` statement for value equality checking. Compiles to efficient if-else chain.

**Syntax:**
```truk
match expression {
  value1 => result1,
  value2 => result2,
  _ => default_result
}
```

**Properties:**
- Expression-based (returns a value)
- Value equality only (no pattern destructuring)
- Exhaustiveness checking with `_` (wildcard/default case)
- Compiles to if-else chain

### Examples

**Matching Integers:**
```truk
fn classify_number(x: i32) : []u8 {
  match x {
    0 => return "zero",
    1 => return "one",
    42 => return "answer",
    _ => return "other"
  }
}
```

**Matching Characters:**
```truk
fn char_type(c: u8) : i32 {
  match c {
    'a' => return 1,
    'b' => return 2,
    'c' => return 3,
    '0' => return 10,
    '1' => return 11,
    _ => return 0
  }
}
```

**Match with Expressions:**
```truk
fn http_status_name(code: i32) : []u8 {
  match code {
    200 => return "OK",
    201 => return "Created",
    400 => return "Bad Request",
    404 => return "Not Found",
    500 => return "Internal Server Error",
    _ => return "Unknown"
  }
}
```

**Generated C Code:**
```c
// match x { 0 => ..., 1 => ..., _ => ... }
if (x == 0) {
  // case 0
} else if (x == 1) {
  // case 1
} else {
  // default case
}
```

---

## Slice Syntax

Proposed slice syntax for efficient substring operations without copying data. These operations return new slice structs that point into the original data.

### Syntax Proposals

#### **1. Range Slicing**
```truk
var original: []u8 = "hello world";

var from_start: []u8 = original[..5];    // First 5 bytes: "hello"
var from_index: []u8 = original[6..];    // From index 6 to end: "world"
var middle: []u8 = original[2..5];       // Index 2 to 5: "llo"
var full: []u8 = original[..];           // Full slice (copy struct)
```

#### **2. Generated C Code**
```c
// original[6..]
truk_slice_u8 from_index = {
  .data = original.data + 6,
  .len = original.len - 6
};

// original[2..5]
truk_slice_u8 middle = {
  .data = original.data + 2,
  .len = 5 - 2
};

// original[..5]
truk_slice_u8 from_start = {
  .data = original.data,
  .len = 5
};
```

#### **3. Bounds Checking**
All slice operations include runtime bounds checking:
```c
// For original[start..end]
if (start > original.len) panic("start out of bounds");
if (end > original.len) panic("end out of bounds");
if (start > end) panic("invalid range");
```

---

## Complete Parsing Example

Combining all features to parse HTTP request lines:

```truk
fn parse_http_request(line: []u8) : HttpMethod {
  var get: []u8 = "GET ";
  var post: []u8 = "POST ";
  var put: []u8 = "PUT ";
  var delete: []u8 = "DELETE ";
  
  if starts_with(line, get) {
    var path: []u8 = substr(line, 4, line.len - 4);
    return HttpMethod { method: GET, path: path };
  } else if starts_with(line, post) {
    var path: []u8 = substr(line, 5, line.len - 5);
    return HttpMethod { method: POST, path: path };
  } else if starts_with(line, put) {
    var path: []u8 = substr(line, 4, line.len - 4);
    return HttpMethod { method: PUT, path: path };
  } else if starts_with(line, delete) {
    var path: []u8 = substr(line, 7, line.len - 7);
    return HttpMethod { method: DELETE, path: path };
  } else {
    panic("Unknown HTTP method");
  }
}
```

**With Slice Syntax (future):**
```truk
fn parse_http_request(line: []u8) : HttpMethod {
  var get: []u8 = "GET ";
  var post: []u8 = "POST ";
  
  if starts_with(line, get) {
    return HttpMethod { method: GET, path: line[4..] };
  } else if starts_with(line, post) {
    return HttpMethod { method: POST, path: line[5..] };
  } else {
    panic("Unknown HTTP method");
  }
}
```

---

## Token Parsing Example

```truk
fn parse_token(input: []u8) : Token {
  if input.len == 0 {
    return Token { kind: EOF };
  }
  
  var first: u8 = input[0];
  
  match first {
    '(' => return Token { kind: LPAREN },
    ')' => return Token { kind: RPAREN },
    '{' => return Token { kind: LBRACE },
    '}' => return Token { kind: RBRACE },
    '"' => return parse_string(input),
    _ => {
      if is_digit(first) {
        return parse_number(input);
      } else if is_alpha(first) {
        return parse_identifier(input);
      } else {
        return Token { kind: UNKNOWN };
      }
    }
  }
}

fn is_digit(c: u8) : bool {
  return c >= '0' && c <= '9';
}

fn is_alpha(c: u8) : bool {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
```

---

## Implementation Priority

1. **Match statement** - Syntactic sugar for if-else chain (simplest)
2. **starts_with/ends_with builtins** - Essential for parsing, simple memcmp wrapper
3. **substr builtin** - Enables substring extraction before slice syntax exists
4. **Slice syntax** - More complex, requires parser/grammar changes but provides better ergonomics

The builtins and match statement can be implemented independently and provide immediate value. Slice syntax is a nice-to-have enhancement that makes the code cleaner but isn't strictly necessary.
