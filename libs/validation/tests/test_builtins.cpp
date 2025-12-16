#include <cstring>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

using namespace truk::validation;
using namespace truk::ingestion;

static parse_result_s parse_code(const std::string &code) {
  parser_c parser(code.c_str(), code.size());
  return parser.parse();
}

static std::vector<std::string> typecheck_code(const std::string &code) {
  auto result = parse_code(code);
  if (!result.success) {
    return {"Parse error: " + result.error_message};
  }

  type_checker_c checker;
  for (const auto &decl : result.declarations) {
    checker.check(decl.get());
  }

  std::vector<std::string> error_messages;
  for (const auto &err : checker.errors()) {
    error_messages.push_back(err.message);
  }
  return error_messages;
}

TEST_GROUP(BuiltinTests){};

TEST(BuiltinTests, MakeReturnsPointerType) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = make(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MakeWithStructType) {
  std::string code = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test() : void {
      var ptr: *Point = make(@Point);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MakeArrayReturnsSlice) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: []i32 = make(@i32, count);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, DeleteAcceptsPointer) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = make(@i32);
      delete(ptr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, DeleteAcceptsSlice) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: []i32 = make(@i32, count);
      delete(arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, LenReturnsU64) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: []i32 = make(@i32, count);
      var size: u64 = len(arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, SizeofReturnsU64) {
  std::string code = R"(
    fn test() : void {
      var size: u64 = sizeof(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, PanicAcceptsU8Array) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var msg: []u8 = make(@u8, count);
      panic(msg);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, TypeParameterMustBeType) {
  std::string code = R"(
    fn test() : void {
      var x: i32 = 5;
      var ptr: *i32 = make(x);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("type parameter") != std::string::npos);
}

TEST(BuiltinTests, MakeRequiresTypeParameter) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = make();
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("requires a type parameter") != std::string::npos);
}

TEST(BuiltinTests, MakeSingleValueWithOneArg) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = make(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, DeleteRequiresPointerOrArrayArgument) {
  std::string code = R"(
    fn test() : void {
      var x: i32 = 5;
      delete(x);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("pointer, array, or map") != std::string::npos);
}

TEST(BuiltinTests, LenRequiresSliceArgument) {
  std::string code = R"(
    fn test() : void {
      var arr: [5]i32 = [1, 2, 3, 4, 5];
      var size: u64 = len(arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("type mismatch") != std::string::npos);
}

TEST(BuiltinTests, MakeArrayWithStructType) {
  std::string code = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test() : void {
      var count: u64 = 5;
      var arr: []Point = make(@Point, count);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MakeWithPointerType) {
  std::string code = R"(
    fn test() : void {
      var ptr: **i32 = make(@*i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MakeArrayWithArrayType) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: [][5]i32 = make(@[5]i32, count);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MultipleBuiltinCalls) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = make(@i32);
      var count: u64 = 10;
      var arr: []i32 = make(@i32, count);
      var size: u64 = len(arr);
      var type_size: u64 = sizeof(@i32);
      delete(arr);
      delete(ptr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MakeInExpression) {
  std::string code = R"(
    fn get_ptr() : *i32 {
      return make(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, LenInExpression) {
  std::string code = R"(
    fn get_size(arr: []i32) : u64 {
      return len(arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, MakeWithAllPrimitiveTypes) {
  std::string code = R"(
    fn test() : void {
      var p1: *i8 = make(@i8);
      var p2: *i16 = make(@i16);
      var p3: *i32 = make(@i32);
      var p4: *i64 = make(@i64);
      var p5: *u8 = make(@u8);
      var p6: *u16 = make(@u16);
      var p7: *u32 = make(@u32);
      var p8: *u64 = make(@u64);
      var p9: *f32 = make(@f32);
      var p10: *f64 = make(@f64);
      var p11: *bool = make(@bool);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
