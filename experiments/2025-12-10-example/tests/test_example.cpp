#include "truk/core/core.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(ExampleExperimentTests) {
    void setup() override {
    }

    void teardown() override {
    }
};

TEST(ExampleExperimentTests, BasicTest) {
    truk::core::core_c core;
    CHECK_FALSE(core.is_initialized());
    core.initialize();
    CHECK_TRUE(core.is_initialized());
}

TEST(ExampleExperimentTests, CanGetBuildHash) {
    truk::core::core_c core;
    std::string hash = core.get_build_hash();
    CHECK_FALSE(hash.empty());
}

int main(int argc, char** argv) {
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
