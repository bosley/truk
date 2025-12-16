#pragma once

#include <language/node.hpp>
#include <string>
#include <unordered_map>

namespace truk::emitc {

class type_registry_c;

class variable_registry_c {
public:
  variable_registry_c() = default;
  ~variable_registry_c() = default;

  void register_variable(const std::string &name,
                         const truk::language::nodes::type_c *type,
                         type_registry_c &type_registry);
  bool is_slice(const std::string &name) const;
  bool is_map(const std::string &name) const;
  const truk::language::nodes::type_c *get_type(const std::string &name) const;

private:
  std::unordered_map<std::string, bool> _variable_is_slice;
  std::unordered_map<std::string, bool> _variable_is_map;
  std::unordered_map<std::string, const truk::language::nodes::type_c *>
      _variable_types;
};

} // namespace truk::emitc
