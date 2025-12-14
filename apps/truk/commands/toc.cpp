#include "toc.hpp"
#include <fmt/core.h>
#include <truk/core/error_display.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/file_utils.hpp>
#include <truk/ingestion/import_resolver.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int toc(const toc_options_s &opts) {
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
      fmt::print(stderr, "Import error in '{}': {}\n", err.file_path,
                 err.message);
      if (err.line > 0) {
        fmt::print(stderr, "  at line {}, column {}\n", err.line, err.column);
      }
    }
    return 1;
  }

  validation::type_checker_c type_checker;
  for (auto &decl : resolved.all_declarations) {
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
  auto emit_result = emitter.add_declarations(resolved.all_declarations)
                         .set_c_imports(resolved.c_imports)
                         .finalize();

  if (emit_result.has_errors()) {
    for (const auto &err : emit_result.errors) {
      std::string enhanced_message =
          fmt::format("{} (phase: {}, context: {})", err.message,
                      emitc::emission_phase_name(err.phase), err.node_context);
      fmt::print(stderr, "Emission error: {}\n", enhanced_message);
    }
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
      fmt::print(stderr, "Error: Could not write header file '{}'\n",
                 header_file);
      return 1;
    }

    if (!ingestion::write_file(source_file, assembly_result.source)) {
      fmt::print(stderr, "Error: Could not write source file '{}'\n",
                 source_file);
      return 1;
    }

    fmt::print("Successfully emitted library to '{}' and '{}'\n", header_file,
               source_file);
  } else {
    if (!ingestion::write_file(opts.output_file, assembly_result.source)) {
      fmt::print(stderr, "Error: Could not write output file '{}'\n",
                 opts.output_file);
      return 1;
    }

    fmt::print("Successfully emitted C code to '{}'\n", opts.output_file);
  }

  return 0;
}

} // namespace truk::commands
