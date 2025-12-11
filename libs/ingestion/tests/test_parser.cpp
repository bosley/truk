#include "truk/parser/parser.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(CoreTests){void setup() override{}

                      void teardown() override{}};

TEST(CoreTests, CanConstruct) { CHECK_TRUE(true); }

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
