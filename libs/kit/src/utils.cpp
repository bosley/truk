#include "truk/kit/kit.hpp"
#include <filesystem>

namespace truk::kit {

std::optional<std::filesystem::path>
find_kit_file(const std::filesystem::path &start_dir) {
  std::filesystem::path current = std::filesystem::absolute(start_dir);

  while (true) {
    std::filesystem::path kit_path = current / "truk.kit";
    if (std::filesystem::exists(kit_path) &&
        std::filesystem::is_regular_file(kit_path)) {
      return kit_path;
    }

    std::filesystem::path parent = current.parent_path();
    if (parent == current) {
      break;
    }
    current = parent;
  }

  return std::nullopt;
}

std::filesystem::path resolve_path(const std::filesystem::path &base,
                                   const std::string &relative) {
  std::filesystem::path rel_path(relative);

  if (rel_path.is_absolute()) {
    return rel_path;
  }

  return (base / rel_path).lexically_normal();
}

} // namespace truk::kit
