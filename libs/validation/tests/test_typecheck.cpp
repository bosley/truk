#include <cstring>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>

// clang-format off
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
// clang-format on

TEST_GROUP(TypeCheckPrimitiveTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckPrimitiveTests, SignedIntegerTypes) {
  const char *source = R"(
    var a: i8 = 1;
    var b: i16 = 2;
    var c: i32 = 3;
    var d: i64 = 4;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckPrimitiveTests, UnsignedIntegerTypes) {
  const char *source = R"(
    var a: u8 = 1;
    var b: u16 = 2;
    var c: u32 = 3;
    var d: u64 = 4;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckPrimitiveTests, FloatingPointTypes) {
  const char *source = R"(
    var a: f32 = 1.5;
    var b: f64 = 2.5;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckPrimitiveTests, BooleanType) {
  const char *source = R"(
    var a: bool = true;
    var b: bool = false;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckPrimitiveTests, PointerTypes) {
  const char *source = R"(
    var a: *i32 = nil;
    var b: **u8 = nil;
    var c: *f64 = nil;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckPrimitiveTests, ArrayTypes) {
  const char *source = R"(
    var a: [10]i32 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    var b: [5]f64 = [1.0, 2.0, 3.0, 4.0, 5.0];
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST_GROUP(TypeCheckDeclarationTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckDeclarationTests, VariableDeclarations) {
  const char *source = R"(
    var x: i32 = 42;
    var y: f64 = 3.14;
    var z: bool = true;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, ConstantDeclarations) {
  const char *source = R"(
    const PI: f64 = 3.14159;
    const MAX: i32 = 100;
    const FLAG: bool = false;
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, FunctionDeclarationNoParams) {
  const char *source = R"(
    fn get_value(): i32 {
      return 42;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, FunctionDeclarationWithParams) {
  const char *source = R"(
    fn add(a: i32, b: i32): i32 {
      return a + b;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, FunctionDeclarationVoidReturn) {
  const char *source = R"(
    fn print_value(x: i32): void {
      return;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, StructDeclaration) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, StructWithMultipleFields) {
  const char *source = R"(
    struct Person {
      age: i32,
      height: f64,
      is_active: bool
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckDeclarationTests, StructWithPointerFields) {
  const char *source = R"(
    struct Node {
      value: i32,
      next: *Node
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST_GROUP(TypeCheckExpressionTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckExpressionTests, ArithmeticOperations) {
  const char *source = R"(
    fn test(): i32 {
      var a: i32 = 10;
      var b: i32 = 5;
      var sum: i32 = a + b;
      var diff: i32 = a - b;
      var prod: i32 = a * b;
      var quot: i32 = a / b;
      var mod: i32 = a % b;
      return sum;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckExpressionTests, ComparisonOperations) {
  const char *source = R"(
    fn test(): bool {
      var a: i32 = 10;
      var b: i32 = 5;
      var eq: bool = a == b;
      var ne: bool = a != b;
      var lt: bool = a < b;
      var le: bool = a <= b;
      var gt: bool = a > b;
      var ge: bool = a >= b;
      return eq;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckExpressionTests, LogicalOperations) {
  const char *source = R"(
    fn test(): bool {
      var a: bool = true;
      var b: bool = false;
      var and_result: bool = a && b;
      var or_result: bool = a || b;
      var not_result: bool = !a;
      return and_result;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckExpressionTests, BitwiseOperations) {
  const char *source = R"(
    fn test(): i32 {
      var a: i32 = 10;
      var b: i32 = 5;
      var and_result: i32 = a & b;
      var or_result: i32 = a | b;
      var xor_result: i32 = a ^ b;
      var left_shift: i32 = a << b;
      var right_shift: i32 = a >> b;
      return and_result;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckExpressionTests, UnaryNegation) {
  const char *source = R"(
    fn test(): i32 {
      var a: i32 = 10;
      var neg: i32 = -a;
      return neg;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckExpressionTests, AddressOfAndDereference) {
  const char *source = R"(
    fn test(): i32 {
      var a: i32 = 42;
      var ptr: *i32 = &a;
      var val: i32 = *ptr;
      return val;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckExpressionTests, Assignment) {
  const char *source = R"(
    fn test(): void {
      var x: i32 = 10;
      x = 20;
      x = 30;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST_GROUP(TypeCheckControlFlowTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckControlFlowTests, IfStatement) {
  const char *source = R"(
    fn test(x: i32): i32 {
      if x > 0 {
        return 1;
      }
      return 0;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, IfElseStatement) {
  const char *source = R"(
    fn test(x: i32): i32 {
      if x > 0 {
        return 1;
      } else {
        return -1;
      }
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, IfElseIfChain) {
  const char *source = R"(
    fn test(x: i32): i32 {
      if x > 0 {
        return 1;
      } else if x < 0 {
        return -1;
      } else {
        return 0;
      }
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, WhileLoop) {
  const char *source = R"(
    fn test(): i32 {
      var i: i32 = 0;
      while i < 10 {
        i = i + 1;
      }
      return i;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, ForLoop) {
  const char *source = R"(
    fn test(): i32 {
      var sum: i32 = 0;
      for var i: i32 = 0; i < 10; i = i + 1 {
        sum = sum + i;
      }
      return sum;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, BreakStatement) {
  const char *source = R"(
    fn test(): i32 {
      var i: i32 = 0;
      while true {
        if i >= 10 {
          break;
        }
        i = i + 1;
      }
      return i;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, ContinueStatement) {
  const char *source = R"(
    fn test(): i32 {
      var sum: i32 = 0;
      var i: i32 = 0;
      while i < 10 {
        i = i + 1;
        if i % 2 == 0 {
          continue;
        }
        sum = sum + i;
      }
      return sum;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, ReturnWithValue) {
  const char *source = R"(
    fn get_value(): i32 {
      return 42;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckControlFlowTests, ReturnVoid) {
  const char *source = R"(
    fn do_nothing(): void {
      return;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST_GROUP(TypeCheckComplexTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckComplexTests, NestedScopes) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 10;
      {
        var y: i32 = 20;
        x = y;
      }
      return x;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, FunctionCall) {
  const char *source = R"(
    fn add(a: i32, b: i32): i32 {
      return a + b;
    }
    
    fn test(): i32 {
      var result: i32 = add(10, 20);
      return result;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, ArrayIndexing) {
  const char *source = R"(
    fn test(): i32 {
      var arr: [5]i32 = [1, 2, 3, 4, 5];
      var idx: i32 = 2;
      var val: i32 = arr[idx];
      return val;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructMemberAccess) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): i32 {
      var p: Point = Point{x: 10, y: 20};
      var x_val: i32 = p.x;
      return x_val;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructLiteral) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): Point {
      var p: Point = Point{x: 10, y: 20};
      return p;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, ArrayLiteral) {
  const char *source = R"(
    fn test(): i32 {
      var arr: [3]i32 = [1, 2, 3];
      return arr[0];
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, NestedStructs) {
  const char *source = R"(
    struct Inner {
      value: i32
    }
    
    struct Outer {
      inner: Inner,
      count: i32
    }
    
    fn test(): i32 {
      var inner: Inner = Inner{value: 42};
      var outer: Outer = Outer{inner: inner, count: 1};
      return outer.inner.value;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, PointerArithmetic) {
  const char *source = R"(
    fn test(): i32 {
      var arr: [5]i32 = [1, 2, 3, 4, 5];
      var ptr: *i32 = &arr[0];
      var val: i32 = *ptr;
      return val;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, ComplexExpression) {
  const char *source = R"(
    fn test(): i32 {
      var a: i32 = 10;
      var b: i32 = 20;
      var c: i32 = 30;
      var result: i32 = (a + b) * c - (a / b);
      return result;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructArrays) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): i32 {
      var points: [3]Point = [Point{x: 1, y: 2}, Point{x: 3, y: 4}, Point{x: 5, y: 6}];
      var p: Point = points[1];
      return p.x;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, PointerToStruct) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): i32 {
      var p: Point = Point{x: 10, y: 20};
      var ptr: *Point = &p;
      var deref: Point = *ptr;
      return deref.x;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructAssignment) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): i32 {
      var p1: Point = Point{x: 10, y: 20};
      var p2: Point = p1;
      p2 = p1;
      return p2.x;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructAsParameter) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn get_x(p: Point): i32 {
      return p.x;
    }
    
    fn test(): i32 {
      var p: Point = Point{x: 42, y: 100};
      return get_x(p);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, EmptyStruct) {
  const char *source = R"(
    struct Empty {
    }
    
    fn test(): Empty {
      var e: Empty = Empty{};
      return e;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructWithArrayFields) {
  const char *source = R"(
    struct Container {
      items: [10]i32,
      count: i32
    }
    
    fn test(): i32 {
      var c: Container = Container{items: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], count: 10};
      var arr: [10]i32 = c.items;
      return arr[5];
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, StructFieldOrdering) {
  const char *source = R"(
    struct Data {
      first: i32,
      second: bool,
      third: f64
    }
    
    fn test(): f64 {
      var d: Data = Data{first: 10, second: true, third: 3.14};
      return d.third;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, MultipleStructTypes) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    struct Rectangle {
      top_left: Point,
      bottom_right: Point
    }
    
    struct Circle {
      center: Point,
      radius: f64
    }
    
    fn test(): i32 {
      var p1: Point = Point{x: 0, y: 0};
      var p2: Point = Point{x: 10, y: 10};
      var rect: Rectangle = Rectangle{top_left: p1, bottom_right: p2};
      var circ: Circle = Circle{center: p1, radius: 5.0};
      return rect.bottom_right.x + circ.center.y;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, ArrayOfPointers) {
  const char *source = R"(
    fn test(): i32 {
      var a: i32 = 10;
      var b: i32 = 20;
      var c: i32 = 30;
      var ptrs: [3]*i32 = [&a, &b, &c];
      var ptr: *i32 = ptrs[1];
      return *ptr;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, PointerToArrayElement) {
  const char *source = R"(
    fn test(): i32 {
      var arr: [5]i32 = [1, 2, 3, 4, 5];
      var ptr: *i32 = &arr[0];
      var val: i32 = *ptr;
      return val;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, ComplexPointerDereferencing) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 42;
      var ptr1: *i32 = &x;
      var ptr2: **i32 = &ptr1;
      var ptr3: ***i32 = &ptr2;
      var deref1: **i32 = *ptr3;
      var deref2: *i32 = *deref1;
      var val: i32 = *deref2;
      return val;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, NestedMemberAccessChains) {
  const char *source = R"(
    struct A {
      value: i32
    }
    
    struct B {
      a: A
    }
    
    struct C {
      b: B
    }
    
    struct D {
      c: C
    }
    
    fn test(): i32 {
      var d: D = D{c: C{b: B{a: A{value: 42}}}};
      return d.c.b.a.value;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, CompoundAssignmentOperators) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 10;
      x += 5;
      x -= 3;
      x *= 2;
      x /= 4;
      x %= 3;
      return x;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, BitwiseNOT) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 42;
      var y: i32 = ~x;
      return y;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, NumericTypeCompatibility) {
  const char *source = R"(
    fn test(): void {
      var i: i32 = 10;
      var f: f64 = 3.14;
      i = 20;
      f = 2.71;
      i = f;
      f = i;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckComplexTests, VoidPointerCompatibility) {
  const char *source = R"(
    fn test(): void {
      var x: i32 = 42;
      var ptr: *i32 = &x;
      var void_ptr: *void = ptr;
      var back_ptr: *i32 = void_ptr;
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST_GROUP(TypeCheckErrorTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckErrorTests, TypeMismatchInAssignment) {
  const char *source = R"(
    fn test(): void {
      var x: i32 = 10;
      x = true;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, TypeMismatchInVariableInit) {
  const char *source = R"(
    var x: i32 = true;
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, UndefinedVariable) {
  const char *source = R"(
    fn test(): i32 {
      return undefined_var;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, UndefinedType) {
  const char *source = R"(
    var x: UnknownType = nil;
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, WrongReturnType) {
  const char *source = R"(
    fn test(): i32 {
      return true;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, MissingReturnValue) {
  const char *source = R"(
    fn test(): i32 {
      return;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, NonBooleanIfCondition) {
  const char *source = R"(
    fn test(): void {
      if 42 {
        return;
      }
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, NonBooleanWhileCondition) {
  const char *source = R"(
    fn test(): void {
      while 42 {
        break;
      }
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, ArithmeticOnBooleans) {
  const char *source = R"(
    fn test(): bool {
      var a: bool = true;
      var b: bool = false;
      return a + b;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, LogicalOpOnIntegers) {
  const char *source = R"(
    fn test(): bool {
      var a: i32 = 10;
      var b: i32 = 20;
      return a && b;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, BitwiseOpOnFloats) {
  const char *source = R"(
    fn test(): f64 {
      var a: f64 = 1.5;
      var b: f64 = 2.5;
      return a & b;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, FunctionCallArgumentCountMismatch) {
  const char *source = R"(
    fn add(a: i32, b: i32): i32 {
      return a + b;
    }
    
    fn test(): i32 {
      return add(10);
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, FunctionCallArgumentTypeMismatch) {
  const char *source = R"(
    fn add(a: i32, b: i32): i32 {
      return a + b;
    }
    
    fn test(): i32 {
      return add(10, true);
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, CallNonFunction) {
  const char *source = R"(
    fn test(): void {
      var x: i32 = 10;
      x();
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, IndexWithNonInteger) {
  const char *source = R"(
    fn test(): i32 {
      var arr: [5]i32 = [1, 2, 3, 4, 5];
      return arr[true];
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, IndexNonArray) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 10;
      return x[0];
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, MemberAccessOnNonStruct) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 10;
      return x.field;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, UndefinedStructField) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): i32 {
      var p: Point = Point{x: 10, y: 20};
      return p.z;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, BreakOutsideLoop) {
  const char *source = R"(
    fn test(): void {
      break;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, ContinueOutsideLoop) {
  const char *source = R"(
    fn test(): void {
      continue;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, DereferenceNonPointer) {
  const char *source = R"(
    fn test(): i32 {
      var x: i32 = 10;
      return *x;
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, StructLiteralTypeMismatch) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): Point {
      return Point{x: true, y: 20};
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, StructLiteralUndefinedField) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): Point {
      return Point{x: 10, y: 20, z: 30};
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckErrorTests, ArrayLiteralInconsistentTypes) {
  const char *source = R"(
    fn test(): void {
      var arr: [3]i32 = [1, 2, true];
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST_GROUP(TypeCheckMapTests) {
  truk::validation::type_checker_c *checker;

  void setup() override { checker = new truk::validation::type_checker_c(); }

  void teardown() override { delete checker; }

  void parse_and_check(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      checker->check(decl.get());
    }
  }
};

TEST(TypeCheckMapTests, MapCreationWithPrimitiveValue) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapCreationWithStructValue) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): void {
      var m: map[*u8, Point] = make(@map[*u8, Point]);
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapCreationWithPointerValue) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, *i32] = make(@map[*u8, *i32]);
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingWithStringLiteral) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      m["key"] = 42;
      var ptr: *i32 = m["key"];
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingWithU8Pointer) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      var key: *u8 = "hello";
      m[key] = 42;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingWithI8Pointer) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      var key: *i8 = "world";
      m[key] = 42;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingWithU8Slice) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      var size: u64 = 10;
      var key: []u8 = make(@u8, size);
      m[key] = 42;
      delete(key);
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingReturnsPointerToValue) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      m["key"] = 42;
      var ptr: *i32 = m["key"];
      if ptr != nil {
        var value: i32 = *ptr;
      }
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapAssignmentWithCorrectType) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      m["key"] = 42;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapAssignmentWithStructValue) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test(): void {
      var m: map[*u8, Point] = make(@map[*u8, Point]);
      var p: Point = Point{x: 10, y: 20};
      m["origin"] = p;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapDeletion) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingWithInvalidKeyType) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      m[42] = 100;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapAssignmentWithWrongValueType) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      m["key"] = true;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapIndexingWrongReturnType) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, i32] = make(@map[*u8, i32]);
      m["key"] = 42;
      var value: i32 = m["key"];
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_TRUE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapWithPointerValuesCorrectUsage) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, *i32] = make(@map[*u8, *i32]);
      var value: i32 = 10;
      m["key"] = &value;
      var ptr_ptr: **i32 = m["key"];
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapTypeEqualityDifferentValueTypes) {
  const char *source = R"(
    fn test(): void {
      var m1: map[*u8, i32] = make(@map[*u8, i32]);
      var m2: map[*u8, f64] = make(@map[*u8, f64]);
      delete(m1);
      delete(m2);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

TEST(TypeCheckMapTests, MapNestedValueTypes) {
  const char *source = R"(
    fn test(): void {
      var m: map[*u8, *i32] = make(@map[*u8, *i32]);
      var value: i32 = 42;
      var ptr: *i32 = &value;
      m["key"] = ptr;
      delete(m);
    }
  )";
  parse_and_check(source);
  CHECK_FALSE(checker->has_errors());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
