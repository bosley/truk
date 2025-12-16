#include "tcc.hpp"
#include <fmt/core.h>
#include <truk/core/error_reporter.hpp>
#include <truk/tcc/tcc.hpp>

namespace truk::commands {

int tcc(const tcc_options_s &opts) {
  core::error_reporter_c reporter;

  for (const auto &path : opts.include_paths) {
    fmt::print("Include path: {}\n", path);
  }
  for (const auto &path : opts.library_paths) {
    fmt::print("Library path: {}\n", path);
  }
  for (const auto &lib : opts.libraries) {
    fmt::print("Library: {}\n", lib);
  }
  for (const auto &path : opts.rpaths) {
    fmt::print("Rpath: {}\n", path);
  }

  truk::tcc::tcc_compiler_c compiler;
  compiler.set_output_type(truk::tcc::OUTPUT_EXE);

  for (const auto &path : opts.include_paths) {
    compiler.add_include_path(path);
  }
  for (const auto &path : opts.library_paths) {
    compiler.add_library_path(path);
  }
  for (const auto &lib : opts.libraries) {
    compiler.add_library(lib);
  }
  for (const auto &path : opts.rpaths) {
    compiler.set_rpath(path);
  }

  auto result = compiler.compile_file(opts.input_file, opts.output_file);

  if (!result.success) {
    reporter.report_compilation_error(result.error_message);
    reporter.print_summary();
    return 1;
  }

  fmt::print("Successfully compiled '{}' to '{}'\n", opts.input_file,
             opts.output_file);
  return 0;
}

} // namespace truk::commands
