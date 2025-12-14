#include "truk/kit/kit.hpp"
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace truk::kit {

build_order_s resolve_build_order(const kit_config_s &config) {
  std::unordered_map<std::string, std::vector<std::string>> adjacency_list;
  std::unordered_map<std::string, int> in_degree;
  std::unordered_map<std::string, size_t> library_indices;

  for (size_t i = 0; i < config.libraries.size(); ++i) {
    const auto &[name, lib] = config.libraries[i];
    library_indices[name] = i;
    in_degree[name] = 0;
    adjacency_list[name] = {};
  }

  for (const auto &[name, lib] : config.libraries) {
    if (lib.depends.has_value()) {
      for (const auto &dep : lib.depends.value()) {
        if (library_indices.find(dep) == library_indices.end()) {
          throw kit_exception_c(exception_e::PARSE_ERROR, 0,
                                "Library '" + name +
                                    "' depends on unknown library '" + dep +
                                    "'");
        }
        adjacency_list[dep].push_back(name);
        in_degree[name]++;
      }
    }
  }

  std::queue<std::string> zero_in_degree;
  for (const auto &[name, degree] : in_degree) {
    if (degree == 0) {
      zero_in_degree.push(name);
    }
  }

  build_order_s result;
  std::unordered_set<std::string> processed;

  while (!zero_in_degree.empty()) {
    std::string current = zero_in_degree.front();
    zero_in_degree.pop();

    size_t idx = library_indices[current];
    result.libraries.push_back(config.libraries[idx]);
    processed.insert(current);

    for (const auto &neighbor : adjacency_list[current]) {
      in_degree[neighbor]--;
      if (in_degree[neighbor] == 0) {
        zero_in_degree.push(neighbor);
      }
    }
  }

  if (result.libraries.size() != config.libraries.size()) {
    std::stringstream cycle_msg;
    cycle_msg << "Circular dependency detected among libraries: ";
    bool first = true;
    for (const auto &[name, _] : config.libraries) {
      if (processed.find(name) == processed.end()) {
        if (!first)
          cycle_msg << ", ";
        cycle_msg << name;
        first = false;
      }
    }
    throw kit_exception_c(exception_e::PARSE_ERROR, 0, cycle_msg.str());
  }

  for (const auto &app : config.applications) {
    result.applications.push_back(app);
  }

  return result;
}

} // namespace truk::kit
