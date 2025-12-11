#include "../include/typecheck.hpp"
#include <cstring>
#include <truk/ingestion/parser.hpp>

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

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
