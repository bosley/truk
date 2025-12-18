#include <language/keywords.hpp>
#include <truk/emitc/cdef.hpp>
#include <truk/emitc/type_registry.hpp>

namespace truk::emitc {

using namespace truk::language;
using namespace truk::language::nodes;

std::string type_registry_c::get_c_type(const type_c *type) {
  if (!type) {
    return "__truk_void";
  }

  if (auto prim = type->as_primitive_type()) {
    switch (prim->keyword()) {
    case keywords_e::I8:
      return "__truk_i8";
    case keywords_e::I16:
      return "__truk_i16";
    case keywords_e::I32:
      return "__truk_i32";
    case keywords_e::I64:
      return "__truk_i64";
    case keywords_e::U8:
      return "__truk_u8";
    case keywords_e::U16:
      return "__truk_u16";
    case keywords_e::U32:
      return "__truk_u32";
    case keywords_e::U64:
      return "__truk_u64";
    case keywords_e::F32:
      return "__truk_f32";
    case keywords_e::F64:
      return "__truk_f64";
    case keywords_e::BOOL:
      return "__truk_bool";
    case keywords_e::VOID:
      return "__truk_void";
    default:
      return "__truk_void";
    }
  }

  if (auto named = type->as_named_type()) {
    const std::string &name = named->name().name;
    if (_extern_struct_names.count(name)) {
      return "struct " + name;
    }
    return name;
  }

  if (auto gen = type->as_generic_type_instantiation()) {
    std::vector<const type_c *> type_args;
    for (const auto &arg : gen->type_arguments()) {
      type_args.push_back(arg.get());
    }
    return get_instantiated_name(gen->base_name().name, type_args);
  }

  if (auto ptr = type->as_pointer_type()) {
    return get_c_type(ptr->pointee_type()) + "*";
  }

  if (auto arr = type->as_array_type()) {
    if (arr->size().has_value()) {
      return get_c_type(arr->element_type());
    } else {
      return get_slice_type_name(arr->element_type());
    }
  }

  if (auto map = type->as_map_type()) {
    return get_map_type_name(map->key_type(), map->value_type());
  }

  if (auto tuple = type->as_tuple_type()) {
    std::string name = "__truk_tuple";
    for (const auto &elem : tuple->element_types()) {
      std::string elem_type = get_c_type(elem.get());
      for (char c : elem_type) {
        if (c == '*') {
          name += "_ptr";
        } else if (c == '[' || c == ']' || c == '(' || c == ')' || c == ',') {
        } else if (c == ' ') {
          name += "_";
        } else if (std::isalnum(c) || c == '_') {
          name += c;
        }
      }
    }
    return name;
  }

  if (auto func = type->as_function_type()) {
    std::string ret_type = get_c_type(func->return_type());
    std::string func_type = ret_type + " (*)(";

    const auto &param_types = func->param_types();
    for (size_t i = 0; i < param_types.size(); ++i) {
      if (i > 0) {
        func_type += ", ";
      }
      func_type += get_c_type(param_types[i].get());
    }

    if (param_types.empty()) {
      func_type += "void";
    }

    if (func->has_variadic()) {
      if (!param_types.empty()) {
        func_type += ", ";
      }
      func_type += "...";
    }

    func_type += ")";
    return func_type;
  }

  return "__truk_void";
}

std::string type_registry_c::get_c_type_for_sizeof(const type_c *type) {
  if (!type) {
    return "__truk_void";
  }

  if (auto prim = type->as_primitive_type()) {
    return get_c_type(prim);
  }

  if (auto named = type->as_named_type()) {
    return named->name().name;
  }

  if (auto gen = type->as_generic_type_instantiation()) {
    std::vector<const type_c *> type_args;
    for (const auto &arg : gen->type_arguments()) {
      type_args.push_back(arg.get());
    }
    return get_instantiated_name(gen->base_name().name, type_args);
  }

  if (auto ptr = type->as_pointer_type()) {
    if (auto arr = ptr->pointee_type()->as_array_type()) {
      if (arr->size().has_value()) {
        std::string base = get_c_type_for_sizeof(arr->element_type());
        return base + "(*)[" + std::to_string(arr->size().value()) + "]";
      }
    }
    return get_c_type_for_sizeof(ptr->pointee_type()) + "*";
  }

  if (auto arr = type->as_array_type()) {
    if (arr->size().has_value()) {
      std::string base = get_c_type_for_sizeof(arr->element_type());
      return base + "[" + std::to_string(arr->size().value()) + "]";
    } else {
      return get_slice_type_name(arr->element_type());
    }
  }

  return "__truk_void";
}

std::string
type_registry_c::get_array_pointer_type(const type_c *array_type,
                                        const std::string &identifier) {
  if (!array_type) {
    return "";
  }

  auto arr = array_type->as_array_type();
  if (!arr || !arr->size().has_value()) {
    return "";
  }

  std::string base_type;
  std::vector<size_t> dimensions;

  const type_c *current = array_type;
  while (auto current_arr = current->as_array_type()) {
    if (current_arr->size().has_value()) {
      dimensions.push_back(current_arr->size().value());
      current = current_arr->element_type();
    } else {
      break;
    }
  }

  base_type = get_c_type(current);

  std::string result = base_type + " (*";
  if (!identifier.empty()) {
    result += identifier;
  }
  result += ")";
  for (size_t dim : dimensions) {
    result += "[" + std::to_string(dim) + "]";
  }

  return result;
}

std::string type_registry_c::get_slice_type_name(const type_c *element_type) {
  std::string elem_type_str = get_c_type_for_sizeof(element_type);
  std::string slice_name = "truk_slice_" + elem_type_str;
  for (auto &c : slice_name) {
    if (c == '*')
      c = 'p';
    if (c == '[' || c == ']')
      c = '_';
  }
  return slice_name;
}

void type_registry_c::ensure_slice_typedef(const type_c *element_type,
                                           std::stringstream &header_stream,
                                           std::stringstream &structs_stream) {
  std::string slice_name = get_slice_type_name(element_type);
  if (_slice_types_emitted.find(slice_name) == _slice_types_emitted.end()) {
    _slice_types_emitted.insert(slice_name);
    std::string elem_type_for_sizeof = get_c_type_for_sizeof(element_type);

    if (auto arr = element_type->as_array_type()) {
      if (arr->size().has_value()) {
        std::string pointer_type = get_array_pointer_type(element_type, "data");
        header_stream << "typedef struct {\n  " << pointer_type
                      << ";\n  __truk_u64 len;\n} " << slice_name << ";\n\n";
        return;
      }
    }

    header_stream << cdef::emit_slice_typedef(elem_type_for_sizeof, slice_name);
  }
}

bool type_registry_c::is_slice_type(const type_c *type) {
  if (auto arr = type->as_array_type()) {
    return !arr->size().has_value();
  }
  return false;
}

std::string type_registry_c::get_map_type_name(const type_c *key_type,
                                               const type_c *value_type) {
  std::string key_str = get_c_type_for_sizeof(key_type);
  std::string value_str = get_c_type_for_sizeof(value_type);
  std::string sanitized_key = key_str;
  std::string sanitized_value = value_str;

  for (auto &c : sanitized_key) {
    if (c == '*')
      c = 'p';
    if (c == '[' || c == ']' || c == ' ' || c == '(' || c == ')')
      c = '_';
  }
  for (auto &c : sanitized_value) {
    if (c == '*')
      c = 'p';
    if (c == '[' || c == ']' || c == ' ' || c == '(' || c == ')')
      c = '_';
  }
  return "__truk_map_" + sanitized_key + "_" + sanitized_value;
}

void type_registry_c::ensure_map_typedef(const type_c *key_type,
                                         const type_c *value_type,
                                         std::stringstream &structs_stream) {
  std::string map_name = get_map_type_name(key_type, value_type);

  if (_map_types_emitted.find(map_name) == _map_types_emitted.end()) {
    _map_types_emitted.insert(map_name);
    std::string value_str = get_c_type_for_sizeof(value_type);
    structs_stream << "typedef __truk_map_t(" << value_str << ") " << map_name
                   << ";\n\n";
  }
}

bool type_registry_c::is_map_type(const type_c *type) {
  return type->as_map_type() != nullptr;
}

bool type_registry_c::has_maps() const { return !_map_types_emitted.empty(); }

bool type_registry_c::is_string_ptr_type(const type_c *type) {
  if (auto ptr = type->as_pointer_type()) {
    if (auto pointee = ptr->pointee_type()) {
      if (auto prim = pointee->as_primitive_type()) {
        auto kw = prim->keyword();
        return kw == truk::language::keywords_e::U8 ||
               kw == truk::language::keywords_e::I8;
      }
    }
  }
  return false;
}

void type_registry_c::register_struct_name(const std::string &name) {
  _struct_names.insert(name);
}

void type_registry_c::register_extern_struct_name(const std::string &name) {
  _extern_struct_names.insert(name);
}

bool type_registry_c::is_extern_struct(const std::string &name) const {
  return _extern_struct_names.count(name) > 0;
}

const std::unordered_set<std::string> &
type_registry_c::get_struct_names() const {
  return _struct_names;
}

const std::unordered_set<std::string> &
type_registry_c::get_extern_struct_names() const {
  return _extern_struct_names;
}

void type_registry_c::register_generic_struct(const std::string &name) {
  _generic_struct_names.insert(name);
}

bool type_registry_c::is_generic_struct(const std::string &name) const {
  return _generic_struct_names.count(name) > 0;
}

std::string type_registry_c::get_instantiated_name(
    const std::string &base_name,
    const std::vector<const type_c *> &type_args) {
  std::string mangled = base_name;
  for (const auto *arg : type_args) {
    mangled += "_";
    mangled += mangle_type_for_name(arg);
  }
  return mangled;
}

void type_registry_c::register_instantiation(
    const std::string &base_name, const std::vector<const type_c *> &type_args,
    const std::string &mangled_name) {
  _emitted_instantiations.insert(mangled_name);
  std::string key = base_name;
  for (const auto *arg : type_args) {
    key += "_" + mangle_type_for_name(arg);
  }
  _instantiation_to_mangled[key] = mangled_name;
}

bool type_registry_c::is_instantiation_emitted(
    const std::string &mangled_name) const {
  return _emitted_instantiations.count(mangled_name) > 0;
}

std::string type_registry_c::mangle_type_for_name(const type_c *type) {
  if (auto prim = type->as_primitive_type()) {
    switch (prim->keyword()) {
    case keywords_e::I8:
      return "i8";
    case keywords_e::I16:
      return "i16";
    case keywords_e::I32:
      return "i32";
    case keywords_e::I64:
      return "i64";
    case keywords_e::U8:
      return "u8";
    case keywords_e::U16:
      return "u16";
    case keywords_e::U32:
      return "u32";
    case keywords_e::U64:
      return "u64";
    case keywords_e::F32:
      return "f32";
    case keywords_e::F64:
      return "f64";
    case keywords_e::BOOL:
      return "bool";
    default:
      return "void";
    }
  }
  if (auto named = type->as_named_type()) {
    return named->name().name;
  }
  if (auto ptr = type->as_pointer_type()) {
    return "ptr_" + mangle_type_for_name(ptr->pointee_type());
  }
  if (auto arr = type->as_array_type()) {
    if (arr->size().has_value()) {
      return "arr" + std::to_string(arr->size().value()) + "_" +
             mangle_type_for_name(arr->element_type());
    }
    return "slice_" + mangle_type_for_name(arr->element_type());
  }
  if (auto gen = type->as_generic_type_instantiation()) {
    std::string result = gen->base_name().name;
    for (const auto &arg : gen->type_arguments()) {
      result += "_" + mangle_type_for_name(arg.get());
    }
    return result;
  }
  return "unknown";
}

} // namespace truk::emitc
