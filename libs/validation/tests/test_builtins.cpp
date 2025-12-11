#include "../include/typecheck.hpp"
#include <cstring>
#include <truk/ingestion/parser.hpp>

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

  return checker.errors();
}

TEST_GROUP(BuiltinTests){};

TEST(BuiltinTests, AllocReturnsPointerType) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = alloc(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, AllocWithStructType) {
  std::string code = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test() : void {
      var ptr: *Point = alloc(@Point);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, AllocArrayReturnsSlice) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: []i32 = alloc_array(@i32, count);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, FreeAcceptsPointer) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = alloc(@i32);
      free(@i32, ptr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, FreeArrayAcceptsSlice) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: []i32 = alloc_array(@i32, count);
      free_array(@i32, arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, LenReturnsU64) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: []i32 = alloc_array(@i32, count);
      var size: u64 = len(@i32, arr);
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
      var msg: []u8 = alloc_array(@u8, count);
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
      var ptr: *i32 = alloc(x);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("type parameter") != std::string::npos);
}

TEST(BuiltinTests, AllocRequiresTypeParameter) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = alloc();
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("requires a type parameter") != std::string::npos);
}

TEST(BuiltinTests, AllocArrayRequiresCountArgument) {
  std::string code = R"(
    fn test() : void {
      var arr: []i32 = alloc_array(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("expects 1 argument") != std::string::npos);
}

TEST(BuiltinTests, FreeRequiresPointerArgument) {
  std::string code = R"(
    fn test() : void {
      var x: i32 = 5;
      free(@i32, x);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("type mismatch") != std::string::npos);
}

TEST(BuiltinTests, LenRequiresSliceArgument) {
  std::string code = R"(
    fn test() : void {
      var arr: [5]i32 = [1, 2, 3, 4, 5];
      var size: u64 = len(@i32, arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_FALSE(errors.empty());
  CHECK_TRUE(errors[0].find("type mismatch") != std::string::npos);
}

TEST(BuiltinTests, AllocArrayWithStructType) {
  std::string code = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn test() : void {
      var count: u64 = 5;
      var arr: []Point = alloc_array(@Point, count);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, AllocWithPointerType) {
  std::string code = R"(
    fn test() : void {
      var ptr: **i32 = alloc(@*i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, AllocArrayWithArrayType) {
  std::string code = R"(
    fn test() : void {
      var count: u64 = 10;
      var arr: [][5]i32 = alloc_array(@[5]i32, count);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}



TEST(BuiltinTests, MultipleBuiltinCalls) {
  std::string code = R"(
    fn test() : void {
      var ptr: *i32 = alloc(@i32);
      var count: u64 = 10;
      var arr: []i32 = alloc_array(@i32, count);
      var size: u64 = len(@i32, arr);
      var type_size: u64 = sizeof(@i32);
      free_array(@i32, arr);
      free(@i32, ptr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, AllocInExpression) {
  std::string code = R"(
    fn get_ptr() : *i32 {
      return alloc(@i32);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, LenInExpression) {
  std::string code = R"(
    fn get_size(arr: []i32) : u64 {
      return len(@i32, arr);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

TEST(BuiltinTests, AllocWithAllPrimitiveTypes) {
  std::string code = R"(
    fn test() : void {
      var p1: *i8 = alloc(@i8);
      var p2: *i16 = alloc(@i16);
      var p3: *i32 = alloc(@i32);
      var p4: *i64 = alloc(@i64);
      var p5: *u8 = alloc(@u8);
      var p6: *u16 = alloc(@u16);
      var p7: *u32 = alloc(@u32);
      var p8: *u64 = alloc(@u64);
      var p9: *f32 = alloc(@f32);
      var p10: *f64 = alloc(@f64);
      var p11: *bool = alloc(@bool);
    }
  )";

  auto errors = typecheck_code(code);
  CHECK_TRUE(errors.empty());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
