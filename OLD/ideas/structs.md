# Structs and Implementation Blocks in Truk

Truk uses a clean separation between data definitions and behavior, avoiding OOP terminology while maintaining clear organization.

## Basic Structs

Structs in truk are pure data containers, just like C:

```truk
struct Point {
  x: i32,
  y: i32
}

struct Person {
  name: []u8,
  age: u32,
  email: []u8
}
```

### Struct Literals

Create struct instances using struct literal syntax:

```truk
var p: Point = Point{x: 10, y: 20};
var person: Person = Person{
  name: "Alice",
  age: 30,
  email: "alice@example.com"
};
```

### Field Access

Access struct fields using dot notation:

```truk
var x_coord: i32 = p.x;
var person_age: u32 = person.age;

p.x = 15;
person.age = 31;
```

---

## Implementation Blocks

Associate functions with types using `impl` blocks. This provides namespacing and organization without OOP baggage.

### Basic Impl Block

```truk
struct Point {
  x: i32,
  y: i32
}

impl Point {
  fn new(x: i32, y: i32) : Point {
    return Point{x: x, y: y};
  }
  
  fn distance_from_origin() : f64 {
    var x_sq: i32 = x * x;
    var y_sq: i32 = y * y;
    return sqrt((x_sq + y_sq) as f64);
  }
  
  fn translate(dx: i32, dy: i32) : void {
    x = x + dx;
    y = y + dy;
  }
}
```

### Calling Impl Functions

**Associated functions** (don't use `this`):
```truk
var p: Point = Point::new(10, 20);
```

**Methods** (use `this` to access instance):
```truk
var dist: f64 = p.distance_from_origin();
p.translate(5, -3);
```

---

## Implicit Member Access

Inside `impl` methods, member fields are automatically accessible without qualification. Just like C++, unqualified names refer to members of `this`.

### Direct Field Access
```truk
impl Point {
  fn scale(factor: i32) : void {
    x = x * factor;
    y = y * factor;
  }
}

var p: Point = Point{x: 10, y: 20};
p.scale(2);
```

The compiler automatically treats `x` as `this.x` and `y` as `this.y`.

### Explicit `this` When Needed
You can still use `this` explicitly for clarity or to resolve ambiguity:

```truk
impl Point {
  fn set_x(x: i32) : void {
    this.x = x;
  }
  
  fn distance_from(x: i32, y: i32) : f64 {
    var dx: i32 = this.x - x;
    var dy: i32 = this.y - y;
    return sqrt((dx * dx + dy * dy) as f64);
  }
}
```

When a parameter has the same name as a field, `x` refers to the parameter and `this.x` refers to the field.

### Reading Fields
```truk
impl Point {
  fn display() : void {
    printf("Point(%d, %d)\n", x, y);
  }
  
  fn get_x() : i32 {
    return x;
  }
  
  fn sum() : i32 {
    return x + y;
  }
}
```

### No `this` in Associated Functions
```truk
impl Point {
  fn new(x: i32, y: i32) : Point {
    return Point{x: x, y: y};
  }
  
  fn origin() : Point {
    return Point{x: 0, y: 0};
  }
}
```

Associated functions (like constructors) don't have access to members because they're not called on an instance.

---

## C Code Generation

Impl blocks compile to namespaced C functions:

**Truk Code:**
```truk
struct Point {
  x: i32,
  y: i32
}

impl Point {
  fn new(x: i32, y: i32) : Point {
    return Point{x: x, y: y};
  }
  
  fn scale(factor: i32) : void {
    x = x * factor;
    y = y * factor;
  }
}

fn main() : i32 {
  var p: Point = Point::new(10, 20);
  p.scale(2);
  return p.x;
}
```

**Generated C Code:**
```c
typedef struct Point {
  i32 x;
  i32 y;
} Point;

Point Point_new(i32 x, i32 y) {
  return (Point){.x = x, .y = y};
}

void Point_scale(Point* this, i32 factor) {
  this->x = this->x * factor;
  this->y = this->y * factor;
}

i32 main() {
  Point p = Point_new(10, 20);
  Point_scale(&p, 2);
  return p.x;
}
```

---

## Multiple Impl Blocks

Types can have multiple `impl` blocks, even in different files:

**point.truk:**
```truk
struct Point {
  x: i32,
  y: i32
}

impl Point {
  fn new(x: i32, y: i32) : Point {
    return Point{x: x, y: y};
  }
}
```

**point_math.truk:**
```truk
impl Point {
  fn distance(other: *Point) : f64 {
    var dx: i32 = x - other.x;
    var dy: i32 = y - other.y;
    return sqrt((dx * dx + dy * dy) as f64);
  }
}
```

This enables:
- Organizing related functionality
- Extension by users of your library
- Conditional compilation of features

---

## Advanced Patterns

### Builder Pattern

```truk
struct Config {
  host: []u8,
  port: u16,
  timeout: u32,
  debug: bool
}

impl Config {
  fn new() : Config {
    return Config{
      host: "localhost",
      port: 8080,
      timeout: 30,
      debug: false
    };
  }
  
  fn with_host(host: []u8) : *Config {
    this.host = host;
    return this;
  }
  
  fn with_port(port: u16) : *Config {
    this.port = port;
    return this;
  }
  
  fn with_debug(debug: bool) : *Config {
    this.debug = debug;
    return this;
  }
}

fn main() : i32 {
  var config: Config = Config::new();
  config.with_host("0.0.0.0")
        .with_port(3000)
        .with_debug(true);
  
  return 0;
}
```

### Type with Lifetime Management

```truk
struct Buffer {
  data: []u8,
  capacity: u64
}

impl Buffer {
  fn create(capacity: u64) : Buffer {
    var data: []u8 = make(@u8, capacity);
    return Buffer{data: data, capacity: capacity};
  }
  
  fn destroy() : void {
    delete(data);
    capacity = 0;
  }
  
  fn resize(new_capacity: u64) : void {
    var new_data: []u8 = make(@u8, new_capacity);
    
    var copy_len: u64 = capacity;
    if new_capacity < copy_len {
      copy_len = new_capacity;
    }
    
    memcpy(new_data.data, data.data, copy_len);
    delete(data);
    
    data = new_data;
    capacity = new_capacity;
  }
}

fn main() : i32 {
  var buf: Buffer = Buffer::create(100);
  defer buf.destroy();
  
  buf.resize(200);
  
  return 0;
}
```

### Generic-like Behavior (Manual Specialization)

```truk
struct Stack_i32 {
  data: []i32,
  top: u64,
  capacity: u64
}

impl Stack_i32 {
  fn create(capacity: u64) : Stack_i32 {
    return Stack_i32{
      data: make(@i32, capacity),
      top: 0,
      capacity: capacity
    };
  }
  
  fn push(value: i32) : void {
    if top >= capacity {
      panic("Stack overflow");
    }
    data[top] = value;
    top = top + 1;
  }
  
  fn pop() : i32 {
    if top == 0 {
      panic("Stack underflow");
    }
    top = top - 1;
    return data[top];
  }
}
```

---

## Design Principles

1. **Separation of Data and Behavior**
   - `struct` blocks define **only** data layout
   - `impl` blocks define **only** functions

2. **Simple and Familiar**
   - Implicit member access like C++ (no `this.` needed)
   - `this` keyword available when needed for disambiguation
   - Method calls desugar to regular function calls with pointer
   - Clean syntax without parameter clutter

3. **C-Compatible**
   - Structs compile to C structs directly
   - Methods compile to namespaced functions
   - No runtime overhead

4. **Flexible Organization**
   - Multiple `impl` blocks per type
   - Can be in different files/modules
   - Supports extension and composition

5. **No OOP Baggage**
   - No classes, objects, inheritance
   - No virtual dispatch or vtables (unless explicitly built)
   - No access modifiers (use naming conventions: `_private_func`)

---

## Common Patterns

### Constructor + Destructor Pair

```truk
impl Resource {
  fn open(path: []u8) : Resource {
    // Allocate and initialize
  }
  
  fn close() : void {
    // Cleanup and free
  }
}

fn use_resource() : void {
  var r: Resource = Resource::open("file.txt");
  defer r.close();
  
  // Use resource...
}
```

### Method Chaining

```truk
impl Builder {
  fn with_option(opt: i32) : *Builder {
    option = opt;
    return this;
  }
}

var b: Builder = Builder::new();
b.with_x(10).with_y(20).with_z(30);
```

### Associated Constants (via functions)

```truk
impl Color {
  fn red() : Color {
    return Color{r: 255, g: 0, b: 0};
  }
  
  fn green() : Color {
    return Color{r: 0, g: 255, b: 0};
  }
}

var c: Color = Color::red();
```

---

## Future Extensions

This design enables future language features:

### Traits/Interfaces
```truk
trait Display {
  fn display(self: *Self) : void;
}

impl Display for Point {
  fn display() : void {
    printf("Point(%d, %d)", x, y);
  }
}
```

### Generic Impl Blocks
```truk
impl Stack<T> {
  fn create(capacity: u64) : Stack<T> {
    // ...
  }
}
```

### Operator Overloading
```truk
impl Add for Point {
  fn add(other: Point) : Point {
    return Point{x: x + other.x, y: y + other.y};
  }
}

var p3: Point = p1 + p2;
```

But these are **optional future additions** - the core `struct` + `impl` pattern works perfectly well on its own.
