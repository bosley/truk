#include "toc.hpp"
#include <fmt/core.h>
#include <truk/core/error_reporter.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/file_utils.hpp>
#include <truk/ingestion/import_resolver.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int toc(const toc_options_s &opts) {
  core::error_reporter_c reporter;

  for (const auto &path : opts.include_paths) {
    fmt::print("Include path: {}\n", path);
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

  emitc::assembly_type_e assembly_type =
      emit_result.metadata.has_main_function
          ? emitc::assembly_type_e::APPLICATION
          : emitc::assembly_type_e::LIBRARY;

  std::string base_path = opts.output_file;
  size_t ext_pos = base_path.find_last_of('.');
  if (ext_pos != std::string::npos) {
    base_path = base_path.substr(0, ext_pos);
  }

  std::string header_file = base_path + ".h";
  std::string source_file = base_path + ".c";

  size_t last_slash = header_file.find_last_of("/\\");
  std::string header_basename = (last_slash != std::string::npos)
                                    ? header_file.substr(last_slash + 1)
                                    : header_file;

  auto assembly_result = emit_result.assemble(assembly_type, header_basename);

  if (assembly_type == emitc::assembly_type_e::LIBRARY) {

    if (!ingestion::write_file(header_file, assembly_result.header)) {
      reporter.report_file_error(header_file, "Could not write header file");
      reporter.print_summary();
      return 1;
    }

    if (!ingestion::write_file(source_file, assembly_result.source)) {
      reporter.report_file_error(source_file, "Could not write source file");
      reporter.print_summary();
      return 1;
    }

    fmt::print("Successfully emitted library to '{}' and '{}'\n", header_file,
               source_file);
  } else {
    if (!ingestion::write_file(opts.output_file, assembly_result.source)) {
      reporter.report_file_error(opts.output_file,
                                 "Could not write output file");
      reporter.print_summary();
      return 1;
    }

    fmt::print("Successfully emitted C code to '{}'\n", opts.output_file);
  }

  return 0;
}

} // namespace truk::commands
