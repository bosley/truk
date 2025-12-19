#include <truk/emitc/type_registry.hpp>
#include <truk/emitc/variable_registry.hpp>

namespace truk::emitc {

using namespace truk::language::nodes;

void variable_registry_c::register_variable(const std::string &name,
                                            const type_c *type,
                                            type_registry_c &type_registry) {
  _variable_types[name] = type;
  _variable_is_slice[name] = type_registry.is_slice_type(type);
  _variable_is_map[name] = type_registry.is_map_type(type);
  _variable_is_string_ptr[name] = type_registry.is_string_ptr_type(type);
}

bool variable_registry_c::is_slice(const std::string &name) const {
  auto it = _variable_is_slice.find(name);
  return it != _variable_is_slice.end() && it->second;
}

bool variable_registry_c::is_map(const std::string &name) const {
  auto it = _variable_is_map.find(name);
  return it != _variable_is_map.end() && it->second;
}

bool variable_registry_c::is_string_ptr(const std::string &name) const {
  auto it = _variable_is_string_ptr.find(name);
  return it != _variable_is_string_ptr.end() && it->second;
}

const type_c *variable_registry_c::get_type(const std::string &name) const {
  auto it = _variable_types.find(name);
  return it != _variable_types.end() ? it->second : nullptr;
}

} // namespace truk::emitc
