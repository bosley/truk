#include "compile.hpp"
#include "../common/file_utils.hpp"
#include <fmt/core.h>
#include <truk/core/error_display.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/tcc/tcc.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int compile(const compile_options_s &opts) {
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
    if (!parse_result.error_message.empty() && parse_result.source_data) {
      core::error_display_c display;
      std::string source_str(parse_result.source_data, parse_result.source_len);
      display.show_error(opts.input_file, source_str, parse_result.error_line,
                         parse_result.error_column, parse_result.error_message);
    } else {
      fmt::print(stderr, "Error: Parse failed\n");
      if (!parse_result.error_message.empty()) {
        fmt::print(stderr, "  {}\n", parse_result.error_message);
        fmt::print(stderr, "  at line {}, column {}\n", parse_result.error_line,
                   parse_result.error_column);
      }
    }
    return 1;
  }

  validation::type_checker_c type_checker;
  for (auto &decl : parse_result.declarations) {
    type_checker.check(decl.get());
  }

  if (type_checker.has_errors()) {
    core::error_display_c display;
    const auto &detailed_errors = type_checker.detailed_errors();

    if (!detailed_errors.empty()) {
      for (const auto &err : detailed_errors) {
        display.show_error_at_index(opts.input_file, source, err.source_index,
                                    err.message);
      }
    } else {
      fmt::print(stderr, "Error: Type check failed\n");
      for (const auto &err : type_checker.errors()) {
        fmt::print(stderr, "  {}\n", err);
      }
    }
    return 1;
  }

  emitc::emitter_c emitter;
  auto emit_result =
      emitter.add_declarations(parse_result.declarations).finalize();

  if (emit_result.has_errors()) {
    core::error_display_c display;
    for (const auto &err : emit_result.errors) {
      std::string enhanced_message =
          fmt::format("{} (phase: {}, context: {})", err.message,
                      emitc::emission_phase_name(err.phase), err.node_context);
      display.show_error_at_index(opts.input_file, source, err.source_index,
                                  enhanced_message);
    }
    return 1;
  }

  std::string c_output;
  for (const auto &chunk : emit_result.chunks) {
    c_output += chunk;
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

  auto compile_result = compiler.compile_string(c_output, opts.output_file);

  if (!compile_result.success) {
    fmt::print(stderr, "Error: {}\n", compile_result.error_message);
    return 1;
  }

  fmt::print("Successfully compiled '{}' to '{}'\n", opts.input_file,
             opts.output_file);
  return 0;
}

} // namespace truk::commands
