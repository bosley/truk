#pragma once

#include "compile.hpp"

namespace truk::commands {

using run_options_s = compile_options_s;

inline int run(const run_options_s &opts) { return compile(opts); }

} // namespace truk::commands
