#include "truk/core/core.hpp"
#include <fmt/core.h>
#include <iostream>

int main() {
    truk::core::core_c core;
    core.initialize();
    
    fmt::print("Hello World from truk!\n");
    fmt::print("Build hash: {}\n", core.get_build_hash());
    
    core.shutdown();
    return 0;
}
