# Lambdas in truk

Lambdas are anonymous functions that can be assigned to variables, passed as arguments, and returned from functions. They provide first-class function support for functional programming patterns.

## Syntax

```truk
fn(param1: Type1, param2: Type2) : ReturnType {
  // function body
}
```

Lambdas use the same `fn` keyword as regular functions but without a name.

## Basic Usage

### Assigning to Variables

```truk
var add: fn(i32, i32) : i32 = fn(a: i32, b: i32) : i32 {
  return a + b;
};

var result: i32 = add(5, 10);
```

### Inline Lambdas

Lambdas can be created inline without assigning to a variable:

```truk
var sum: i32 = 0;
each(slice, &sum, fn(elem: *i32, ctx: *i32) : bool {
  *ctx = *ctx + *elem;
  return true;
});
```

### Multiple Lambdas

```truk
var add: fn(i32, i32) : i32 = fn(x: i32, y: i32) : i32 {
  return x + y;
};

var mul: fn(i32, i32) : i32 = fn(x: i32, y: i32) : i32 {
  return x * y;
};

var sum: i32 = add(5, 10);
var product: i32 = mul(5, 10);
```

## Function Types

Lambda types are declared using function type syntax:

```truk
fn(ParamType1, ParamType2) : ReturnType
```

### Examples

```truk
var unary: fn(i32) : i32;
var binary: fn(i32, i32) : i32;
var void_func: fn() : void;
var no_params: fn() : i32;
```

### Complex Types

Function types can involve pointers, structs, and other types:

```truk
struct Point {
  x: i32,
  y: i32
}

var transform: fn(*Point) : Point;
var compare: fn(Point, Point) : bool;
var factory: fn(i32) : *Point;
```

## Passing Lambdas as Arguments

Lambdas can be passed to functions that accept function parameters:

```truk
fn apply(x: i32, y: i32, op: fn(i32, i32) : i32) : i32 {
  return op(x, y);
}

fn main() : i32 {
  var add: fn(i32, i32) : i32 = fn(a: i32, b: i32) : i32 {
    return a + b;
  };
  
  return apply(25, 75, add);
}
```

You can also pass lambdas inline:

```truk
fn apply(x: i32, y: i32, op: fn(i32, i32) : i32) : i32 {
  return op(x, y);
}

fn main() : i32 {
  return apply(10, 20, fn(a: i32, b: i32) : i32 {
    return a * b;
  });
}
```

## Returning Lambdas from Functions

Functions can return lambdas:

```truk
fn make_adder(n: i32) : fn(i32) : i32 {
  if n == 10 {
    return fn(x: i32) : i32 {
      return x + 10;
    };
  }
  return fn(x: i32) : i32 {
    return x + 5;
  };
}

fn main() : i32 {
  var add10: fn(i32) : i32 = make_adder(10);
  return add10(45);
}
```

## Lambda Semantics

### Non-Capturing

**Important:** Lambdas in truk are **non-capturing**. They cannot access variables from their enclosing scope.

```truk
fn main() : i32 {
  var x: i32 = 10;
  
  var add_x: fn(i32) : i32 = fn(y: i32) : i32 {
    return y + x;
  };
  
  return add_x(5);
}
```

This will NOT compile because the lambda tries to access `x` from the outer scope.

### Workaround: Context Parameters

To pass data to lambdas, use explicit parameters:

```truk
fn apply_with_context(y: i32, x: i32, op: fn(i32, i32) : i32) : i32 {
  return op(y, x);
}

fn main() : i32 {
  var x: i32 = 10;
  
  return apply_with_context(5, x, fn(y: i32, x: i32) : i32 {
    return y + x;
  });
}
```

The `each` builtin uses this pattern with its context parameter (see [Iteration with each](#iteration-with-each) below).

### Static Compilation

Lambdas are compiled to **static C functions** at compile time. They are not heap-allocated objects:

```truk
var f: fn(i32) : i32 = fn(x: i32) : i32 {
  return x * 2;
};
```

This generates a static C function and assigns its function pointer to `f`. No dynamic memory allocation occurs.

### No Memory Management

Since lambdas compile to static functions, they require no memory management:

```truk
fn main() : i32 {
  var multiply: fn(i32, i32) : i32 = fn(a: i32, b: i32) : i32 {
    return a * b;
  };
  
  return multiply(6, 7);
}
```

No `delete` is needed. The lambda is just a function pointer to a static function.

## Iteration with each

The `each` builtin uses lambdas to iterate over maps and slices. See the [each builtin documentation](builtins.md#each) for details.

### Map Iteration

```truk
var m: map[i32] = make(@map[i32]);
m["one"] = 1;
m["two"] = 2;

var count: i32 = 0;
each(m, &count, fn(key: *u8, val: *i32, ctx: *i32) : bool {
  *ctx = *ctx + 1;
  return true;
});

delete(m);
```

The lambda receives:
- `key`: Pointer to the key string (`*u8`)
- `val`: Pointer to the value (`*V` where V is the map value type)
- `ctx`: Context pointer (user-provided)

### Slice Iteration

```truk
var count: u64 = 5;
var slice: []i32 = make(@i32, count);
slice[0] = 1;
slice[1] = 2;
slice[2] = 3;

var sum: i32 = 0;
each(slice, &sum, fn(elem: *i32, ctx: *i32) : bool {
  *ctx = *ctx + *elem;
  return true;
});

delete(slice);
```

The lambda receives:
- `elem`: Pointer to the element (`*T` where T is the slice element type)
- `ctx`: Context pointer (user-provided)

## Common Patterns

### Callback Pattern

```truk
fn process_data(data: []i32, callback: fn(i32) : void) : void {
  var i: u64 = 0;
  var size: u64 = len(data);
  while i < size {
    callback(data[i]);
    i = i + 1;
  }
}

fn main() : i32 {
  var data: []i32 = make(@i32, 3);
  data[0] = 10;
  data[1] = 20;
  data[2] = 30;
  
  process_data(data, fn(x: i32) : void {
  });
  
  delete(data);
  return 0;
}
```

### Strategy Pattern

```truk
fn calculate(x: i32, y: i32, strategy: fn(i32, i32) : i32) : i32 {
  return strategy(x, y);
}

fn main() : i32 {
  var add_strategy: fn(i32, i32) : i32 = fn(a: i32, b: i32) : i32 {
    return a + b;
  };
  
  var mul_strategy: fn(i32, i32) : i32 = fn(a: i32, b: i32) : i32 {
    return a * b;
  };
  
  var sum: i32 = calculate(5, 10, add_strategy);
  var product: i32 = calculate(5, 10, mul_strategy);
  
  return sum + product;
}
```

### Filter Pattern with each

```truk
fn main() : i32 {
  var count: u64 = 5;
  var slice: []i32 = make(@i32, count);
  slice[0] = 1;
  slice[1] = 2;
  slice[2] = 3;
  slice[3] = 4;
  slice[4] = 5;
  
  var sum_even: i32 = 0;
  each(slice, &sum_even, fn(elem: *i32, ctx: *i32) : bool {
    if *elem % 2 == 0 {
      *ctx = *ctx + *elem;
    }
    return true;
  });
  
  delete(slice);
  return sum_even;
}
```

## Limitations

### No Closures

Lambdas cannot capture variables from their enclosing scope:

```truk
fn main() : i32 {
  var multiplier: i32 = 10;
  
  var scale: fn(i32) : i32 = fn(x: i32) : i32 {
    return x * multiplier;
  };
  
  return scale(5);
}
```

This does NOT work. Use explicit parameters instead.

### No Recursive Lambdas

Lambdas cannot call themselves recursively:

```truk
var factorial: fn(i32) : i32 = fn(n: i32) : i32 {
  if n <= 1 {
    return 1;
  }
  return n * factorial(n - 1);
};
```

This does NOT work. Use named functions for recursion.

### No Dynamic Creation

Lambdas are statically compiled. You cannot dynamically generate lambdas at runtime based on data.

## Implementation Details

### Compilation to C

Lambdas are compiled to static C functions with generated names:

```truk
var add: fn(i32, i32) : i32 = fn(a: i32, b: i32) : i32 {
  return a + b;
};
```

Generates C code similar to:

```c
static __truk_i32 __truk_lambda_1(__truk_i32 a, __truk_i32 b) {
  return a + b;
}

__truk_i32 (*add)(__truk_i32, __truk_i32) = __truk_lambda_1;
```

### Function Pointers

Lambda variables are function pointers in the generated C code. This makes them efficient with no runtime overhead.

### No Heap Allocation

Unlike some languages where lambdas are heap-allocated objects, truk lambdas are just function pointers to static functions. This means:
- No allocation overhead
- No garbage collection needed
- No memory leaks possible
- Simple and predictable performance

## Best Practices

### Use Descriptive Types

```truk
var comparator: fn(i32, i32) : bool = fn(a: i32, b: i32) : bool {
  return a < b;
};
```

### Keep Lambdas Simple

Complex logic should be in named functions:

```truk
fn complex_logic(x: i32, y: i32) : i32 {
  return x * x + y * y;
}

var processor: fn(i32, i32) : i32 = complex_logic;
```

### Use Context Parameters

When you need external data in a lambda, pass it as a context parameter:

```truk
each(collection, &context, fn(item: *T, ctx: *Context) : bool {
  return true;
});
```

### Prefer Named Functions for Recursion

```truk
fn factorial(n: i32) : i32 {
  if n <= 1 {
    return 1;
  }
  return n * factorial(n - 1);
}
```

## Complete Example

```truk
struct Point {
  x: i32,
  y: i32
}

fn transform_points(points: []Point, transformer: fn(*Point) : void) : void {
  var i: u64 = 0;
  var size: u64 = len(points);
  while i < size {
    transformer(&points[i]);
    i = i + 1;
  }
}

fn main() : i32 {
  var count: u64 = 3;
  var points: []Point = make(@Point, count);
  
  points[0] = Point{x: 1, y: 2};
  points[1] = Point{x: 3, y: 4};
  points[2] = Point{x: 5, y: 6};
  
  transform_points(points, fn(p: *Point) : void {
    (*p).x = (*p).x * 2;
    (*p).y = (*p).y * 2;
  });
  
  var sum: i32 = points[0].x + points[1].x + points[2].x;
  
  delete(points);
  return sum;
}
```

This example demonstrates:
- Defining a function that accepts a lambda
- Creating a lambda inline
- Modifying data through pointers in a lambda
- Proper memory management

## Comparison with Other Languages

| Feature | Go | JavaScript | Rust | truk |
|---------|-----|-----------|------|------|
| Syntax | `func(x int) int` | `(x) => x` | `\|x\| x` | `fn(x: i32) : i32` |
| Closures | Yes | Yes | Yes | No |
| Heap allocation | Yes | Yes | Sometimes | No |
| First-class | Yes | Yes | Yes | Yes |
| Type annotations | Optional | Optional | Required | Required |
| Memory management | GC | GC | Ownership | None needed |

truk lambdas are simpler than most languages: they're just function pointers with no closure support, making them predictable and efficient.
