#include "compile.hpp"
#include <fmt/core.h>
#include <truk/core/error_reporter.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/file_utils.hpp>
#include <truk/ingestion/import_resolver.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/tcc/tcc.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int compile(const compile_options_s &opts) {
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

  ingestion::import_resolver_c resolver;
  for (const auto &path : opts.include_paths) {
    resolver.add_include_path(path);
  }
  auto resolved = resolver.resolve(opts.input_file);

  if (!resolved.success) {
    for (const auto &err : resolved.errors) {
      bool is_parse_error =
          err.type == ingestion::import_error_type_e::PARSE_ERROR;

      if (is_parse_error && err.line > 0) {
        try {
          std::string source = ingestion::read_file(err.file_path);
          reporter.report_parse_error(err.file_path, source, err.line,
                                      err.column, err.message);
        } catch (...) {
          reporter.report_import_error_with_type(err.file_path, err.message,
                                                 err.line, err.column, true);
        }
      } else {
        reporter.report_import_error_with_type(
            err.file_path, err.message, err.line, err.column, is_parse_error);
      }
    }
    reporter.print_summary();
    return 1;
  }

  validation::type_checker_c type_checker;
  type_checker.set_declaration_file_map(resolved.decl_to_file);
  type_checker.set_file_to_shards_map(resolved.file_to_shards);
  for (auto &decl : resolved.all_declarations) {
    type_checker.check(decl.get());
  }

  if (type_checker.has_errors()) {
    for (const auto &err : type_checker.errors()) {
      reporter.report_generic_error(core::error_phase_e::TYPE_CHECKING,
                                    err.message);
    }
    reporter.print_summary();
    return 1;
  }

  emitc::emitter_c emitter;
  auto emit_result = emitter.add_declarations(resolved.all_declarations)
                         .set_declaration_file_map(resolved.decl_to_file)
                         .set_file_to_shards_map(resolved.file_to_shards)
                         .set_c_imports(resolved.c_imports)
                         .finalize();

  if (emit_result.has_errors()) {
    for (const auto &err : emit_result.errors) {
      std::string phase_context =
          fmt::format("phase: {}, context: {}",
                      emitc::emission_phase_name(err.phase), err.node_context);
      reporter.report_generic_error(core::error_phase_e::CODE_EMISSION,
                                    err.message + " (" + phase_context + ")");
    }
    reporter.print_summary();
    return 1;
  }

  if (!emit_result.metadata.has_main_function) {
    reporter.report_generic_error(
        core::error_phase_e::CODE_EMISSION,
        "No main function found. Cannot compile to executable");
    reporter.print_summary();
    return 1;
  }

  if (emit_result.metadata.has_multiple_mains()) {
    fmt::print(stderr,
               "Warning: Multiple main functions detected. Using first one.\n");
  }

  auto assembly_result =
      emit_result.assemble(emitc::assembly_type_e::APPLICATION);
  std::string c_output = assembly_result.source;

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
    reporter.report_compilation_error(compile_result.error_message);
    reporter.print_summary();
    return 1;
  }

  fmt::print("Successfully compiled '{}' to '{}'\n", opts.input_file,
             opts.output_file);
  return 0;
}

} // namespace truk::commands
