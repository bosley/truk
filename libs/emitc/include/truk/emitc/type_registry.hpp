#pragma once

#include <language/node.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
  get_map_type_name(const truk::language::nodes::type_c *key_type,
                    const truk::language::nodes::type_c *value_type);
  void ensure_map_typedef(const truk::language::nodes::type_c *key_type,
                          const truk::language::nodes::type_c *value_type,
                          std::stringstream &structs_stream);
  bool is_map_type(const truk::language::nodes::type_c *type);
  bool has_maps() const;

  bool is_string_ptr_type(const truk::language::nodes::type_c *type);

  void register_struct_name(const std::string &name);
  void register_extern_struct_name(const std::string &name);
  bool is_extern_struct(const std::string &name) const;

  const std::unordered_set<std::string> &get_struct_names() const;
  const std::unordered_set<std::string> &get_extern_struct_names() const;

  void register_generic_struct(const std::string &name);
  bool is_generic_struct(const std::string &name) const;

  std::string get_instantiated_name(
      const std::string &base_name,
      const std::vector<const truk::language::nodes::type_c *> &type_args);

  void register_instantiation(
      const std::string &base_name,
      const std::vector<const truk::language::nodes::type_c *> &type_args,
      const std::string &mangled_name);

  bool is_instantiation_emitted(const std::string &mangled_name) const;
  std::string mangle_type_for_name(const truk::language::nodes::type_c *type);

private:
  std::unordered_set<std::string> _slice_types_emitted;
  std::unordered_set<std::string> _map_types_emitted;
  std::unordered_set<std::string> _struct_names;
  std::unordered_set<std::string> _extern_struct_names;

  std::unordered_set<std::string> _generic_struct_names;
  std::unordered_set<std::string> _emitted_instantiations;
  std::unordered_map<std::string, std::string> _instantiation_to_mangled;
};

} // namespace truk::emitc
