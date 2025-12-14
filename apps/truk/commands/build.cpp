#include "build.hpp"
#include "compile.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <truk/core/cache.hpp>
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
                  std::string &c_output,
                  std::vector<std::string> &source_files) {
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

  source_files = resolved.all_source_files;

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

static int compile_library(const std::string &name,
                           const kit::target_library_c &lib,
                           const fs::path &kit_dir,
                           core::cache_manager_c &cache) {
  auto cache_entry = cache.get_library_cache_paths(name);

  std::vector<std::string> import_search_paths;
  if (lib.include_paths.has_value()) {
    import_search_paths = lib.include_paths.value();
  }
  import_search_paths.push_back(kit_dir.string());

  std::string c_output;
  std::vector<std::string> source_files;
  int result = compile_truk_to_c(lib.source_entry_file_path,
                                 import_search_paths, c_output, source_files);
  if (result != 0) {
    return result;
  }

  if (!cache.needs_rebuild(name, source_files)) {
    fmt::print("Library '{}' is up to date\n", name);
    return 0;
  }

  fmt::print("Building library: {}\n", name);

  fs::create_directories(cache_entry.c_file.parent_path());

  std::ofstream c_file(cache_entry.c_file);
  if (!c_file.is_open()) {
    fmt::print(stderr, "Error: Failed to write C output to '{}'\n",
               cache_entry.c_file.string());
    return 1;
  }
  c_file << c_output;
  c_file.close();

  tcc::tcc_compiler_c compiler;

  if (lib.include_paths.has_value()) {
    for (const auto &path : lib.include_paths.value()) {
      compiler.add_include_path(path);
    }
  }

  auto obj_result =
      compiler.compile_to_object(c_output, cache_entry.o_file.string());
  if (!obj_result.success) {
    fmt::print(stderr, "Error compiling library '{}' to object: {}\n", name,
               obj_result.error_message);
    return 1;
  }

  auto ar_result = compiler.create_static_archive(cache_entry.o_file.string(),
                                                  cache_entry.a_file.string());
  if (!ar_result.success) {
    fmt::print(stderr, "Error creating archive for library '{}': {}\n", name,
               ar_result.error_message);
    return 1;
  }

  cache.update_metadata(name, source_files);

  return 0;
}

static int compile_application(const std::string &name,
                               const kit::target_application_c &app,
                               const kit::kit_config_s &config,
                               core::cache_manager_c &cache) {
  fmt::print("Building application: {}\n", name);

  std::string c_output;
  std::vector<std::string> source_files;
  std::vector<std::string> import_search_paths;
  if (app.include_paths.has_value()) {
    import_search_paths = app.include_paths.value();
  }
  import_search_paths.push_back(config.kit_file_directory.string());

  int result = compile_truk_to_c(app.source_entry_file_path,
                                 import_search_paths, c_output, source_files);
  if (result != 0) {
    return result;
  }

  fs::path output_path(app.output_file_path);
  fs::create_directories(output_path.parent_path());

  tcc::tcc_compiler_c compiler;
  compiler.set_output_type(tcc::OUTPUT_EXE);

  if (app.include_paths.has_value()) {
    for (const auto &path : app.include_paths.value()) {
      compiler.add_include_path(path);
    }
  }

  if (app.library_paths.has_value()) {
    for (const auto &path : app.library_paths.value()) {
      compiler.add_library_path(path);
    }
  }

  if (app.libraries.has_value()) {
    for (const auto &lib_name : app.libraries.value()) {
      auto lib_cache_entry = cache.get_library_cache_paths(lib_name);
      if (fs::exists(lib_cache_entry.o_file)) {
        compiler.add_file(lib_cache_entry.o_file.string());
      } else if (fs::exists(lib_cache_entry.a_file)) {
        compiler.add_file(lib_cache_entry.a_file.string());
      } else {
        fmt::print(stderr,
                   "Error: Library '{}' not found (checked .o and .a files)\n",
                   lib_name);
        return 1;
      }
    }
  }

  auto compile_result = compiler.compile_string(c_output, app.output_file_path);

  if (!compile_result.success) {
    fmt::print(stderr, "Error compiling application '{}': {}\n", name,
               compile_result.error_message);
    return 1;
  }

  return 0;
}

int build(const build_options_s &opts) {
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

  core::cache_manager_c cache(config.kit_file_directory);
  cache.ensure_cache_directories();

  kit::build_order_s build_order;
  try {
    build_order = kit::resolve_build_order(config);
  } catch (const kit::kit_exception_c &e) {
    fmt::print(stderr, "Error resolving dependencies: {}\n", e.what());
    return 1;
  }

  for (const auto &[name, lib] : build_order.libraries) {
    int result = compile_library(name, lib, config.kit_file_directory, cache);
    if (result != 0) {
      fmt::print(stderr, "Failed to build library: {}\n", name);
      return 1;
    }
  }

  for (const auto &[name, app] : build_order.applications) {
    int result = compile_application(name, app, config, cache);
    if (result != 0) {
      fmt::print(stderr, "Failed to build application: {}\n", name);
      return 1;
    }
  }

  std::string project_name =
      config.project_name.empty() ? "project" : config.project_name;
  fmt::print("Successfully built {}\n", project_name);
  return 0;
}

} // namespace truk::commands
