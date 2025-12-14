#include "test.hpp"
#include <cstdlib>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <truk/core/error_display.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/import_resolver.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/kit/kit.hpp>
#include <truk/tcc/tcc.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

namespace fs = std::filesystem;

static int
compile_truk_to_c(const std::string &input_file,
                  const std::vector<std::string> &import_search_paths,
                  std::string &c_output) {
  ingestion::import_resolver_c resolver;
  for (const auto &path : import_search_paths) {
    resolver.add_import_search_path(path);
  }
  auto resolved = resolver.resolve(input_file);

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
    const auto &detailed_errors = type_checker.detailed_errors();
    if (!detailed_errors.empty()) {
      for (const auto &err : detailed_errors) {
        fmt::print(stderr, "Type error: {}\n", err.message);
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

  c_output.clear();
  for (const auto &chunk : emit_result.chunks) {
    c_output += chunk;
  }

  return 0;
}

int test(const test_options_s &opts) {
  auto kit_path = kit::find_kit_file(opts.target_dir);
  if (!kit_path.has_value()) {
    fmt::print(stderr,
               "Error: No truk.kit found in '{}' or parent directories\n",
               opts.target_dir.string());
    return 1;
  }

  kit::kit_config_s config;
  try {
    config = kit::parse_kit_file(kit_path.value());
  } catch (const kit::kit_exception_c &e) {
    fmt::print(stderr, "Error parsing kit file: {}\n", e.what());
    return 1;
  }

  kit::build_order_s build_order;
  try {
    build_order = kit::resolve_build_order(config);
  } catch (const kit::kit_exception_c &e) {
    fmt::print(stderr, "Error resolving dependencies: {}\n", e.what());
    return 1;
  }

  int failed = 0;
  int passed = 0;

  for (const auto &[name, lib] : build_order.libraries) {
    if (!lib.test_file_path.has_value()) {
      continue;
    }

    if (opts.specific_test.has_value() && opts.specific_test.value() != name) {
      continue;
    }

    fmt::print("Testing library: {} ... ", name);
    std::flush(std::cout);

    std::string test_exe = "build/test_" + name;
    fs::path test_exe_path(test_exe);
    fs::create_directories(test_exe_path.parent_path());

    std::string c_output;
    std::vector<std::string> import_search_paths;
    if (lib.include_paths.has_value()) {
      import_search_paths = lib.include_paths.value();
    }
    import_search_paths.push_back(config.kit_file_directory.string());

    int result = compile_truk_to_c(lib.test_file_path.value(),
                                   import_search_paths, c_output);
    if (result != 0) {
      fmt::print("COMPILE FAILED\n");
      failed++;
      continue;
    }

    tcc::tcc_compiler_c compiler;
    compiler.set_output_type(tcc::OUTPUT_EXE);

    if (lib.include_paths.has_value()) {
      for (const auto &path : lib.include_paths.value()) {
        compiler.add_include_path(path);
      }
    }

    auto compile_result = compiler.compile_string(c_output, test_exe);
    if (!compile_result.success) {
      fmt::print("COMPILE FAILED\n");
      fmt::print(stderr, "  Error: {}\n", compile_result.error_message);
      failed++;
      continue;
    }

    int test_result = std::system(test_exe.c_str());
    if (test_result == 0) {
      fmt::print("PASS\n");
      passed++;
    } else {
      fmt::print("FAIL (exit code: {})\n", test_result);
      failed++;
    }
  }

  if (passed == 0 && failed == 0) {
    fmt::print("No tests found\n");
    return 0;
  }

  fmt::print("\n");
  if (failed == 0) {
    fmt::print("All tests passed ({} passed)\n", passed);
    return 0;
  } else {
    fmt::print("{} test(s) failed, {} passed\n", failed, passed);
    return 1;
  }
}

} // namespace truk::commands
