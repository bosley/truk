#include "truk/core/core.hpp"
#include <fmt/core.h>

int main() {
    truk::core::core_c core;
    core.initialize();
    
    fmt::print("Example experiment running...\n");
    fmt::print("Testing some concept here\n");
    
    core.shutdown();
    return 0;
}
