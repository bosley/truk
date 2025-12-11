#include "../include/emitter.hpp"
#include <cstring>
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

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
