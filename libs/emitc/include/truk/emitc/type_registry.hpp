#pragma once

#include <language/node.hpp>
#include <sstream>
#include <string>
#include <unordered_set>

namespace truk::emitc {

class type_registry_c {
public:
  type_registry_c() = default;
  ~type_registry_c() = default;

  std::string get_c_type(const truk::language::nodes::type_c *type);
  std::string get_c_type_for_sizeof(const truk::language::nodes::type_c *type);
  std::string
  get_array_pointer_type(const truk::language::nodes::type_c *array_type,
                         const std::string &identifier = "");

  std::string
  get_slice_type_name(const truk::language::nodes::type_c *element_type);
  void ensure_slice_typedef(const truk::language::nodes::type_c *element_type,
                            std::stringstream &header_stream,
                            std::stringstream &structs_stream);
  bool is_slice_type(const truk::language::nodes::type_c *type);

  std::string
  get_map_type_name(const truk::language::nodes::type_c *value_type);
  void ensure_map_typedef(const truk::language::nodes::type_c *value_type,
                          std::stringstream &structs_stream);
  bool is_map_type(const truk::language::nodes::type_c *type);

  void register_struct_name(const std::string &name);
  void register_extern_struct_name(const std::string &name);
  bool is_extern_struct(const std::string &name) const;

  const std::unordered_set<std::string> &get_struct_names() const;
  const std::unordered_set<std::string> &get_extern_struct_names() const;

private:
  std::unordered_set<std::string> _slice_types_emitted;
  std::unordered_set<std::string> _map_types_emitted;
  std::unordered_set<std::string> _struct_names;
  std::unordered_set<std::string> _extern_struct_names;
};

} // namespace truk::emitc
