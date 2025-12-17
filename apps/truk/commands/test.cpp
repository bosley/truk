#include "test.hpp"
#include <algorithm>
#include <filesystem>
#include <fmt/core.h>
#include <truk/core/error_reporter.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/file_utils.hpp>
#include <truk/ingestion/import_resolver.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/tcc/tcc.hpp>
#include <truk/validation/typecheck.hpp>

namespace fs = std::filesystem;

namespace truk::commands {

static std::vector<std::string> collect_truk_files(const std::string &path) {
  std::vector<std::string> files;

  if (fs::is_regular_file(path)) {
    if (path.ends_with(".truk")) {
      files.push_back(path);
    }
    return files;
  }

  if (fs::is_directory(path)) {
    for (const auto &entry : fs::recursive_directory_iterator(path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".truk") {
        files.push_back(entry.path().string());
      }
    }
    std::sort(files.begin(), files.end());
  }

  return files;
}

static int test_single_file(const test_options_s &opts, bool quiet = false) {
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
      if (!err.file_path.empty()) {
        try {
          std::string source = ingestion::read_file(err.file_path);
          reporter.report_typecheck_error(err.file_path, source,
                                          err.source_index, err.message);
        } catch (...) {
          reporter.report_generic_error(core::error_phase_e::TYPE_CHECKING,
                                        err.message + " (in " + err.file_path +
                                            ")");
        }
      } else {
        reporter.report_generic_error(core::error_phase_e::TYPE_CHECKING,
                                      err.message);
      }
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

  if (!emit_result.metadata.has_tests()) {
    if (!quiet) {
      reporter.report_generic_error(
          core::error_phase_e::CODE_EMISSION,
          "No test functions found. Tests must have signature: fn test_*(t: "
          "*test_context_s) : void");
      reporter.print_summary();
    }
    return -1;
  }

  std::string c_source = emit_result.assemble_test_runner();

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

  auto run_result = compiler.compile_and_run(c_source, 0, nullptr);

  if (!run_result.success) {
    reporter.report_compilation_error(run_result.error_message);
    reporter.print_summary();
    return 1;
  }

  return run_result.exit_code;
}

int test(const test_options_s &opts) {
  auto files = collect_truk_files(opts.input_file);

  if (files.empty()) {
    fmt::print(stderr, "Error: No .truk files found in: {}\n", opts.input_file);
    return 1;
  }

  bool is_multi_file = files.size() > 1;
  int total_failed = 0;
  int files_with_tests = 0;

  for (const auto &file : files) {
    if (is_multi_file) {
      fmt::print("\nTesting: {}\n", file);
    }

    test_options_s file_opts = opts;
    file_opts.input_file = file;

    int result = test_single_file(file_opts, is_multi_file);

    if (result == -1) {
      continue;
    }

    files_with_tests++;
    total_failed += result;
  }

  if (files_with_tests == 0) {
    fmt::print(stderr, "Error: No test functions found in any files\n");
    return 1;
  }

  if (is_multi_file) {
    fmt::print("\n========================================\n");
    fmt::print("Tested {} file(s), {} failure(s)\n", files_with_tests,
               total_failed);
  }

  return total_failed;
}

} // namespace truk::commands
