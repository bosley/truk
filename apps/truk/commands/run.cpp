#include "run.hpp"
#include "../common/file_utils.hpp"
#include <fmt/core.h>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/tcc/tcc.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int run(const run_options_s &opts) {
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

  std::string source = common::read_file(opts.input_file);

  ingestion::parser_c parser(source.c_str(), source.size());
  auto parse_result = parser.parse();

  if (!parse_result.success) {
    fmt::print(stderr, "Error: Parse failed\n");
    if (!parse_result.error_message.empty()) {
      fmt::print(stderr, "  {}\n", parse_result.error_message);
      fmt::print(stderr, "  at line {}, column {}\n", parse_result.error_line,
                 parse_result.error_column);
    }
    return 1;
  }

  validation::type_checker_c type_checker;
  for (auto &decl : parse_result.declarations) {
    type_checker.check(decl.get());
  }

  if (type_checker.has_errors()) {
    fmt::print(stderr, "Error: Type check failed\n");
    for (const auto &err : type_checker.errors()) {
      fmt::print(stderr, "  {}\n", err);
    }
    return 1;
  }

  emitc::emitter_c emitter;

  for (auto &decl : parse_result.declarations) {
    emitter.collect_declarations(decl.get());
  }
  emitter.emit_forward_declarations();

  for (auto &decl : parse_result.declarations) {
    emitter.emit(decl.get());
  }
  emitter.finalize();

  auto emit_result = emitter.result();

  if (emit_result.has_errors()) {
    fmt::print(stderr, "Error: Emit failed\n");
    for (const auto &err : emit_result.errors) {
      fmt::print(stderr, "  {}\n", err.message);
    }
    return 1;
  }

  std::string c_source;
  for (const auto &chunk : emit_result.chunks) {
    c_source += chunk;
  }

  truk::tcc::tcc_compiler_c compiler;

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

  auto run_result = compiler.compile_and_run(c_source, opts.argc, opts.argv);

  if (!run_result.success) {
    fmt::print(stderr, "Error: {}\n", run_result.error_message);
    return 1;
  }

  return run_result.exit_code;
}

} // namespace truk::commands
