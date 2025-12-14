#include "run.hpp"
#include <fmt/core.h>
#include <truk/core/error_display.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/file_utils.hpp>
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

  std::string source = ingestion::read_file(opts.input_file);

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
  auto emit_result = emitter.add_declarations(parse_result.declarations)
                         .set_c_imports(parse_result.c_imports)
                         .finalize();

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

  if (!emit_result.metadata.has_main_function) {
    fmt::print(stderr, "Error: No main function found. Cannot run program.\n");
    return 1;
  }

  if (emit_result.metadata.has_multiple_mains()) {
    fmt::print(stderr,
               "Warning: Multiple main functions detected. Using first one.\n");
  }

  auto assembly_result =
      emit_result.assemble(emitc::assembly_type_e::APPLICATION);
  std::string c_source = assembly_result.source;

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
