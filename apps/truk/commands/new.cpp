#include "new.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>

namespace truk::commands {

int new_project(const new_options_s &opts) {
  namespace fs = std::filesystem;

  if (opts.project_name.empty()) {
    fmt::print(stderr, "Error: Project name cannot be empty\n");
    return 1;
  }

  fs::path project_dir(opts.project_name);

  if (fs::exists(project_dir)) {
    fmt::print(stderr, "Error: Directory '{}' already exists\n",
               opts.project_name);
    return 1;
  }

  try {
    fs::create_directory(project_dir);
    fs::create_directory(project_dir / "apps");
    fs::create_directory(project_dir / "apps" / "main");
    fs::create_directory(project_dir / "libs");

    std::ofstream kit_file(project_dir / "truk.kit");
    kit_file << "project " << opts.project_name << "\n\n";
    kit_file << "application main {\n";
    kit_file << "    source = apps/main/main.truk\n";
    kit_file << "    output = build/main\n";
    kit_file << "}\n";
    kit_file.close();

    std::ofstream main_file(project_dir / "apps" / "main" / "main.truk");
    main_file << "fn main(): i32 {\n";
    main_file << "    return 0;\n";
    main_file << "}\n";
    main_file.close();

    fmt::print("Created project: {}\n", opts.project_name);
    fmt::print("\nNext steps:\n");
    fmt::print("  cd {}\n", opts.project_name);
    fmt::print("  truk build\n");
    fmt::print("  ./build/main\n");

    return 0;
  } catch (const std::exception &e) {
    fmt::print(stderr, "Error creating project: {}\n", e.what());
    return 1;
  }
}

} // namespace truk::commands
