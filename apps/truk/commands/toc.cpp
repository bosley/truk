#include "toc.hpp"
#include "../common/file_utils.hpp"
#include <fmt/core.h>
#include <truk/core/error_display.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int toc(const toc_options_s &opts) {
  for (const auto &path : opts.include_paths) {
    fmt::print("Include path: {}\n", path);
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

  std::string output;
  for (const auto &chunk : emit_result.chunks) {
    output += chunk;
  }

  if (!common::write_file(opts.output_file, output)) {
    return 1;
  }

  fmt::print("Successfully emitted C code to '{}'\n", opts.output_file);
  return 0;
}

} // namespace truk::commands
