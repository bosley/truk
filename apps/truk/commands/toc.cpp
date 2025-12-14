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
  auto emit_result =
      emitter.add_declarations(resolved.all_declarations).finalize();

  if (emit_result.has_errors()) {
    for (const auto &err : emit_result.errors) {
      std::string enhanced_message =
          fmt::format("{} (phase: {}, context: {})", err.message,
                      emitc::emission_phase_name(err.phase), err.node_context);
      fmt::print(stderr, "Emission error: {}\n", enhanced_message);
    }
    return 1;
  }

  std::string output;
  for (const auto &chunk : emit_result.chunks) {
    output += chunk;
  }

  if (!ingestion::write_file(opts.output_file, output)) {
    fmt::print(stderr, "Error: Could not write output file '{}'\n",
               opts.output_file);
    return 1;
  }

  fmt::print("Successfully emitted C code to '{}'\n", opts.output_file);
  return 0;
}

} // namespace truk::commands
