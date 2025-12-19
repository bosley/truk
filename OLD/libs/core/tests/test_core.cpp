#include "truk/core/core.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(CoreTests){void setup() override{}

                      void teardown() override{}};

TEST(CoreTests, CanConstruct) {
  truk::core::core_c core;
  CHECK_FALSE(core.is_initialized());
}

TEST(CoreTests, CanInitialize) {
  truk::core::core_c core;
  core.initialize();
  CHECK_TRUE(core.is_initialized());
}

TEST(CoreTests, CanShutdown) {
  truk::core::core_c core;
  core.initialize();
  CHECK_TRUE(core.is_initialized());
  core.shutdown();
  CHECK_FALSE(core.is_initialized());
}

TEST(CoreTests, HasBuildHash) {
  truk::core::core_c core;
  std::string hash = core.get_build_hash();
  CHECK_FALSE(hash.empty());
}

TEST(CoreTests, CanMove) {
  truk::core::core_c core1;
  core1.initialize();
  CHECK_TRUE(core1.is_initialized());

  truk::core::core_c core2(std::move(core1));
  CHECK_TRUE(core2.is_initialized());
}

TEST(CoreTests, DoubleInitializeIsIdempotent) {
  truk::core::core_c core;
  core.initialize();
  core.initialize();
  CHECK_TRUE(core.is_initialized());
}

TEST(CoreTests, DoubleShutdownIsIdempotent) {
  truk::core::core_c core;
  core.initialize();
  core.shutdown();
  core.shutdown();
  CHECK_FALSE(core.is_initialized());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
