#include "truk/core/core.hpp"
#include <cstring>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/typecheck.hpp>

int main(int argc, char **argv) {
  if (argc < 2) {
    fmt::print(stderr, "Usage: {} <input.truk> [-o <output.c>]\n", argv[0]);
    return 1;
  }

  std::string input_file = argv[1];
  std::string output_file = "truk.out";

  for (int i = 2; i < argc; ++i) {
    if (std::strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      output_file = argv[i + 1];
      ++i;
    }
  }

  std::ifstream file(input_file);
  if (!file.is_open()) {
    fmt::print(stderr, "Error: Could not open file '{}'\n", input_file);
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();
  file.close();

  truk::ingestion::parser_c parser(source.c_str(), source.size());
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

  truk::validation::type_checker_c type_checker;
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

  truk::emitc::emitter_c emitter;
  
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

  std::ofstream out(output_file);
  if (!out.is_open()) {
    fmt::print(stderr, "Error: Could not open output file '{}'\n", output_file);
    return 1;
  }

  for (const auto &chunk : emit_result.chunks) {
    out << chunk;
  }
  out.close();

  fmt::print("Successfully emitted C code to '{}'\n", output_file);
  return 0;
}
