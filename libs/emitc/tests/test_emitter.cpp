#include <cstring>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/parser.hpp>

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(EmitterBasicTests) {
  truk::emitc::emitter_c *emitter;

  void setup() override { emitter = new truk::emitc::emitter_c(); }

  void teardown() override { delete emitter; }

  void parse_and_emit(const char *source) {
    truk::ingestion::parser_c parser(source, std::strlen(source));
    auto result = parser.parse();
    CHECK_TRUE(result.success);
    for (auto &decl : result.declarations) {
      emitter->emit(decl.get());
    }
    emitter->finalize();
  }
};

TEST(EmitterBasicTests, EmitterInstantiation) {
  const char *source = R"(
    var a: i32 = 42;
  )";
  parse_and_emit(source);
  auto result = emitter->result();
  CHECK_FALSE(result.has_errors());
}

TEST(EmitterBasicTests, EmitSimpleFunction) {
  const char *source = R"(
    fn add(a: i32, b: i32) : i32 {
      return a + b;
    }
  )";
  parse_and_emit(source);
  auto result = emitter->result();
  CHECK_FALSE(result.has_errors());
  CHECK_TRUE(result.chunks.size() > 0);
}

TEST(EmitterBasicTests, EmitStruct) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
  )";
  parse_and_emit(source);
  auto result = emitter->result();
  CHECK_FALSE(result.has_errors());
}

TEST(EmitterBasicTests, EmitIfStatement) {
  const char *source = R"(
    fn test(x: i32) : i32 {
      if x > 0 {
        return 1;
      } else {
        return 0;
      }
    }
  )";
  parse_and_emit(source);
  auto result = emitter->result();
  CHECK_FALSE(result.has_errors());
}

TEST(EmitterBasicTests, EmitWhileLoop) {
  const char *source = R"(
    fn test(n: i32) : void {
      while n > 0 {
        n = n - 1;
      }
    }
  )";
  parse_and_emit(source);
  auto result = emitter->result();
  CHECK_FALSE(result.has_errors());
}

TEST(EmitterBasicTests, EmitCompleteProgram) {
  const char *source = R"(
    struct Point {
      x: i32,
      y: i32
    }
    
    fn add(a: i32, b: i32) : i32 {
      return a + b;
    }
    
    fn main() : i32 {
      var p: Point = Point{x: 10, y: 20};
      var sum: i32 = add(p.x, p.y);
      return sum;
    }
  )";
  parse_and_emit(source);
  auto result = emitter->result();
  CHECK_FALSE(result.has_errors());
  CHECK_TRUE(result.chunks.size() >= 3);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
