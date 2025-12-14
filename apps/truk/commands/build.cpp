#include "build.hpp"
#include "compile.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
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

static int compile_library(const std::string &name,
                           const kit::target_library_c &lib,
                           const fs::path &kit_dir) {
  fmt::print("Building library: {}\n", name);

  std::string c_output;
  std::vector<std::string> import_search_paths;
  if (lib.include_paths.has_value()) {
    import_search_paths = lib.include_paths.value();
  }
  import_search_paths.push_back(kit_dir.string());

  int result = compile_truk_to_c(lib.source_entry_file_path,
                                 import_search_paths, c_output);
  if (result != 0) {
    return result;
  }

  fs::path output_path(lib.output_file_path);
  fs::create_directories(output_path.parent_path());

  std::ofstream out_file(lib.output_file_path);
  if (!out_file.is_open()) {
    fmt::print(stderr, "Error: Failed to write library C output to '{}'\n",
               lib.output_file_path);
    return 1;
  }
  out_file << c_output;
  out_file.close();

  return 0;
}

static int compile_application(const std::string &name,
                               const kit::target_application_c &app,
                               const kit::kit_config_s &config) {
  fmt::print("Building application: {}\n", name);

  std::string c_output;
  std::vector<std::string> import_search_paths;
  if (app.include_paths.has_value()) {
    import_search_paths = app.include_paths.value();
  }
  import_search_paths.push_back(config.kit_file_directory.string());

  int result = compile_truk_to_c(app.source_entry_file_path,
                                 import_search_paths, c_output);
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
      for (const auto &[name, lib] : config.libraries) {
        if (name == lib_name) {
          fs::path lib_path(lib.output_file_path);
          std::string ext = lib_path.extension().string();

          if (ext == ".c") {
            compiler.add_file(lib.output_file_path);
          } else if (ext == ".o" || ext == ".obj") {
            compiler.add_file(lib.output_file_path);
          } else if (ext == ".a" || ext == ".so" || ext == ".dylib") {
            compiler.add_library_path(lib_path.parent_path().string());

            std::string lib_filename = lib_path.filename().string();
            if (lib_filename.substr(0, 3) == "lib" && lib_filename.size() > 3) {
              std::string base_name = lib_filename.substr(3);
              size_t dot_pos = base_name.find('.');
              if (dot_pos != std::string::npos) {
                base_name = base_name.substr(0, dot_pos);
              }
              compiler.add_library(base_name);
            }
          }
          break;
        }
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

  kit::build_order_s build_order;
  try {
    build_order = kit::resolve_build_order(config);
  } catch (const kit::kit_exception_c &e) {
    fmt::print(stderr, "Error resolving dependencies: {}\n", e.what());
    return 1;
  }

  for (const auto &[name, lib] : build_order.libraries) {
    int result = compile_library(name, lib, config.kit_file_directory);
    if (result != 0) {
      fmt::print(stderr, "Failed to build library: {}\n", name);
      return 1;
    }
  }

  for (const auto &[name, app] : build_order.applications) {
    int result = compile_application(name, app, config);
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
