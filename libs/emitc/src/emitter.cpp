#include <language/builtins.hpp>
#include <language/keywords.hpp>
#include <set>
#include <truk/emitc/builtin_handler.hpp>
#include <truk/emitc/cdef.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/emitc/instantiation_collector.hpp>

namespace truk::emitc {

using namespace truk::language;
using namespace truk::language::nodes;

const char *emission_phase_name(emission_phase_e phase) {
  switch (phase) {
  case emission_phase_e::COLLECTION:
    return "collection";
  case emission_phase_e::FORWARD_DECLARATION:
    return "forward declaration";
  case emission_phase_e::STRUCT_DEFINITION:
    return "struct definition";
  case emission_phase_e::FUNCTION_DEFINITION:
    return "function definition";
  case emission_phase_e::EXPRESSION_GENERATION:
    return "expression generation";
  case emission_phase_e::FINALIZATION:
    return "finalization";
  default:
    return "unknown";
  }
}

emitter_c::emitter_c() : _collecting_declarations(false) {
  register_builtin_handlers(_builtin_registry);
}

emitter_c &emitter_c::add_declaration(const base_c *decl) {
  if (decl) {
    _declarations.push_back(decl);
  }
  return *this;
}

emitter_c &
emitter_c::add_declarations(const std::vector<std::unique_ptr<base_c>> &decls) {
  for (const auto &decl : decls) {
    add_declaration(decl.get());
  }
  return *this;
}

emitter_c &emitter_c::set_c_imports(
    const std::vector<language::nodes::c_import_s> &imports) {
  _c_imports = imports;
  return *this;
}

result_c emitter_c::finalize() {
  try {
    _current_phase = emission_phase_e::COLLECTION;
    for (const auto *decl : _declarations) {
      collect_declarations(decl);
    }

    _current_phase = emission_phase_e::FORWARD_DECLARATION;
    emit_forward_declarations();

    _current_phase = emission_phase_e::FUNCTION_DEFINITION;
    for (const auto *decl : _declarations) {
      emit(decl);
    }

    _current_phase = emission_phase_e::FINALIZATION;
    internal_finalize();
  } catch (const emitter_exception_c &e) {
    add_error(e.what(), nullptr);
  } catch (const std::exception &e) {
    add_error(std::string("Unexpected error: ") + e.what(), nullptr);
  }

  return _result;
}

void emitter_c::add_error(const std::string &msg, const base_c *node) {
  std::size_t source_index = node ? node->source_index() : 0;
  _result.errors.emplace_back(msg, node, source_index, _current_phase,
                              _current_node_context);
}

void emitter_c::collect_declarations(const base_c *root) {
  _collecting_declarations = true;
  if (root) {
    root->accept(*this);
  }
  _collecting_declarations = false;
}

void emitter_c::emit_forward_declarations() {
  for (const auto *decl : _declarations) {
    if (auto *fn = decl->as_fn()) {
      if (fn->is_extern()) {
        continue;
      }

      if (fn->name().name == "main") {
        continue;
      }

      if (fn->name().name.rfind("test_", 0) == 0) {
        continue;
      }

      bool is_private = is_private_identifier(fn->name().name);
      bool is_library = _result.metadata.is_library();

      if (auto func_return = fn->return_type()->as_function_type()) {
        std::string ret_type = emit_type(func_return->return_type());

        if (is_private && is_library) {
          _forward_decls << "static ";
        }

        _forward_decls << ret_type << " (*" << fn->name().name << "(";
      } else {
        std::string return_type = emit_type(fn->return_type());

        if (is_private && is_library) {
          _forward_decls << "static ";
        }

        _forward_decls << return_type << " " << fn->name().name << "(";
      }

      bool has_variadic = false;
      for (size_t i = 0; i < fn->params().size(); ++i) {
        const auto &param = fn->params()[i];

        if (param.is_variadic) {
          has_variadic = true;
          if (i > 0) {
            _forward_decls << ", ";
          }
          _forward_decls << "...";
        } else {
          if (i > 0) {
            _forward_decls << ", ";
          }

          if (auto func = param.type.get()->as_function_type()) {
            std::string ret_type = emit_type(func->return_type());
            _forward_decls << ret_type << " (*" << param.name.name << ")(";

            const auto &func_param_types = func->param_types();
            for (size_t j = 0; j < func_param_types.size(); ++j) {
              if (j > 0) {
                _forward_decls << ", ";
              }
              _forward_decls << emit_type(func_param_types[j].get());
            }

            if (func_param_types.empty()) {
              _forward_decls << "void";
            }

            if (func->has_variadic()) {
              if (!func_param_types.empty()) {
                _forward_decls << ", ";
              }
              _forward_decls << "...";
            }

            _forward_decls << ")";
          } else {
            std::string param_type = emit_type(param.type.get());
            _forward_decls << param_type;
          }

          const type_c *current_type = param.type.get();
          while (auto arr = current_type->as_array_type()) {
            if (arr->size().has_value()) {
              _forward_decls << "[" << arr->size().value() << "]";
              current_type = arr->element_type();
            } else {
              break;
            }
          }
        }
      }

      if (fn->params().empty()) {
        _forward_decls << "void";
      }

      if (auto func_return = fn->return_type()->as_function_type()) {
        _forward_decls << "))(";
        const auto &func_param_types = func_return->param_types();
        for (size_t j = 0; j < func_param_types.size(); ++j) {
          if (j > 0) {
            _forward_decls << ", ";
          }
          _forward_decls << emit_type(func_param_types[j].get());
        }
        if (func_param_types.empty()) {
          _forward_decls << "void";
        }
        if (func_return->has_variadic()) {
          if (!func_param_types.empty()) {
            _forward_decls << ", ";
          }
          _forward_decls << "...";
        }
        _forward_decls << ");\n";
      } else {
        _forward_decls << ");\n";
      }
    }
  }
}

void emitter_c::emit(const base_c *root) {
  if (root) {
    root->accept(*this);
  }
}

void emitter_c::internal_finalize() {
  collect_and_emit_generic_instantiations();

  std::stringstream final_header;

  final_header << cdef::emit_system_includes();
  final_header << cdef::emit_runtime_types();
  final_header << cdef::emit_runtime_declarations();
  final_header << cdef::emit_runtime_macros();

  std::set<std::string> system_includes = {"stdbool.h", "stdint.h", "stdlib.h",
                                           "stdio.h",   "string.h", "stdarg.h"};

  bool has_user_imports = false;
  for (const auto &import : _c_imports) {
    if (import.is_angle_bracket && system_includes.count(import.path)) {
      continue;
    }
    has_user_imports = true;
    if (import.is_angle_bracket) {
      final_header << "#include <" << import.path << ">\n";
    } else {
      final_header << "#include \"" << import.path << "\"\n";
    }
  }
  if (has_user_imports) {
    final_header << "\n";
  }

  final_header << cdef::emit_runtime_implementation();

  if (_type_registry.has_maps()) {
    if (embedded::runtime_files.count("include/sxs/ds/map.h")) {
      final_header << cdef::strip_pragma_and_includes(
          embedded::runtime_files.at("include/sxs/ds/map.h").content);
    }
    if (embedded::runtime_files.count("src/ds/map.c")) {
      final_header << cdef::strip_pragma_and_includes(
          embedded::runtime_files.at("src/ds/map.c").content);
    }
  }

  if (_result.metadata.has_tests()) {
    if (embedded::runtime_files.count("include/sxs/test.h")) {
      final_header << cdef::strip_pragma_and_includes(
          embedded::runtime_files.at("include/sxs/test.h").content);
    }
    if (embedded::runtime_files.count("src/test.c")) {
      final_header << cdef::strip_pragma_and_includes(
          embedded::runtime_files.at("src/test.c").content);
    }
  }

  final_header << "typedef struct {\n  __truk_void* data;\n  __truk_u64 "
                  "len;\n} truk_slice_void;\n\n";

  _result.chunks.push_back(final_header.str());
  _result.chunks.push_back(_structs.str());
  _result.chunks.push_back(_forward_decls.str());
  _result.chunks.push_back(_header.str());
  _result.chunks.push_back(_functions.str());

  _result.metadata.defined_functions = _function_names;
  _result.metadata.defined_structs = _type_registry.get_struct_names();
  _result.metadata.extern_structs = _type_registry.get_extern_struct_names();

  _result.metadata.main_function_count = 0;
  std::string main_file;
  for (const auto &func_name : _function_names) {
    if (func_name == "main") {
      _result.metadata.main_function_count++;
      for (const auto &[decl, file] : _decl_to_file) {
        if (auto *fn = decl->as_fn()) {
          if (fn->name().name == "main") {
            main_file = file;
            break;
          }
        }
      }
    }
  }
  _result.metadata.has_main_function = _result.metadata.main_function_count > 0;

  if (_result.metadata.has_main_function && !main_file.empty()) {
    auto shard_it = _file_to_shards.find(main_file);
    if (shard_it != _file_to_shards.end() && !shard_it->second.empty()) {
      add_error(
          "Shard declarations are not allowed in files containing a main "
          "function. Shards are for sharing implementation details between "
          "library files, not for application entry points",
          nullptr);
    }
  }
}

std::string emitter_c::emit_type(const type_c *type) {
  return _type_registry.get_c_type(type);
}

std::string emitter_c::emit_type_for_sizeof(const type_c *type) {
  return _type_registry.get_c_type_for_sizeof(type);
}

std::string emitter_c::emit_array_pointer_type(const type_c *array_type,
                                               const std::string &identifier) {
  return _type_registry.get_array_pointer_type(array_type, identifier);
}

std::string emitter_c::get_slice_type_name(const type_c *element_type) {
  return _type_registry.get_slice_type_name(element_type);
}

void emitter_c::ensure_slice_typedef(const type_c *element_type) {
  _type_registry.ensure_slice_typedef(element_type, _header, _structs);
}

bool emitter_c::is_slice_type(const type_c *type) {
  return _type_registry.is_slice_type(type);
}

std::string emitter_c::get_map_type_name(const type_c *key_type,
                                         const type_c *value_type) {
  return _type_registry.get_map_type_name(key_type, value_type);
}

void emitter_c::ensure_map_typedef(const type_c *key_type,
                                   const type_c *value_type) {
  _type_registry.ensure_map_typedef(key_type, value_type, _structs);
}

std::string emitter_c::get_tuple_type_name(
    const std::vector<const type_c *> &element_types) {
  std::string name = "__truk_tuple";
  for (const auto *elem : element_types) {
    std::string elem_type = emit_type(elem);
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

void emitter_c::ensure_tuple_typedef(
    const std::vector<const type_c *> &element_types) {
  std::string tuple_name = get_tuple_type_name(element_types);

  if (_generated_tuple_typedefs.find(tuple_name) !=
      _generated_tuple_typedefs.end()) {
    return;
  }

  for (const auto *elem_type : element_types) {
    if (auto map = elem_type->as_map_type()) {
      ensure_map_typedef(map->key_type(), map->value_type());
    }
  }

  _structs << "typedef struct {\n";
  for (size_t i = 0; i < element_types.size(); ++i) {
    const type_c *elem_type = element_types[i];

    std::vector<size_t> array_dims;
    const type_c *base_type = elem_type;
    while (auto arr = base_type->as_array_type()) {
      if (arr->size().has_value()) {
        array_dims.push_back(arr->size().value());
        base_type = arr->element_type();
      } else {
        break;
      }
    }

    if (auto func = base_type->as_function_type()) {
      std::string ret_type = emit_type(func->return_type());
      _structs << "  " << ret_type << " (*_" << i;
      for (size_t dim : array_dims) {
        _structs << "[" << dim << "]";
      }
      _structs << ")(";
      const auto &param_types = func->param_types();
      for (size_t j = 0; j < param_types.size(); ++j) {
        if (j > 0)
          _structs << ", ";
        _structs << emit_type(param_types[j].get());
      }
      if (param_types.empty())
        _structs << "void";
      if (func->has_variadic()) {
        if (!param_types.empty())
          _structs << ", ";
        _structs << "...";
      }
      _structs << ");\n";
    } else {
      _structs << "  " << emit_type(elem_type) << " _" << i;
      const type_c *current_type = elem_type;
      while (auto arr = current_type->as_array_type()) {
        if (arr->size().has_value()) {
          _structs << "[" << arr->size().value() << "]";
          current_type = arr->element_type();
        } else {
          break;
        }
      }
      _structs << ";\n";
    }
  }
  _structs << "} " << tuple_name << ";\n\n";

  _generated_tuple_typedefs.insert(tuple_name);
}

bool emitter_c::is_map_type(const type_c *type) {
  return _type_registry.is_map_type(type);
}

bool emitter_c::is_array_type(const type_c *type) {
  if (auto arr = type->as_array_type()) {
    return arr->size().has_value();
  }
  return false;
}

std::string emitter_c::get_array_dimensions(const type_c *type) {
  std::string dims;
  const type_c *current = type;
  while (auto arr = current->as_array_type()) {
    if (arr->size().has_value()) {
      dims += "[" + std::to_string(arr->size().value()) + "]";
      current = arr->element_type();
    } else {
      break;
    }
  }
  return dims;
}

std::string emitter_c::get_map_hash_fn(const type_c *key_type) {
  if (auto *ptr = key_type->as_pointer_type()) {
    return "__truk_map_hash_str";
  }
  if (auto *prim = key_type->as_primitive_type()) {
    switch (prim->keyword()) {
    case keywords_e::I8:
      return "__truk_map_hash_i8";
    case keywords_e::I16:
      return "__truk_map_hash_i16";
    case keywords_e::I32:
      return "__truk_map_hash_i32";
    case keywords_e::I64:
      return "__truk_map_hash_i64";
    case keywords_e::U8:
      return "__truk_map_hash_u8";
    case keywords_e::U16:
      return "__truk_map_hash_u16";
    case keywords_e::U32:
      return "__truk_map_hash_u32";
    case keywords_e::U64:
      return "__truk_map_hash_u64";
    case keywords_e::F32:
      return "__truk_map_hash_f32";
    case keywords_e::F64:
      return "__truk_map_hash_f64";
    case keywords_e::BOOL:
      return "__truk_map_hash_bool";
    default:
      break;
    }
  }
  return "__truk_map_hash_str";
}

std::string emitter_c::get_map_cmp_fn(const type_c *key_type) {
  if (auto *ptr = key_type->as_pointer_type()) {
    return "__truk_map_cmp_str";
  }
  return "__truk_map_cmp_mem";
}

int emitter_c::get_key_size(const type_c *key_type) {
  if (auto *ptr = key_type->as_pointer_type()) {
    return sizeof(void *);
  }
  if (auto *prim = key_type->as_primitive_type()) {
    switch (prim->keyword()) {
    case keywords_e::I8:
    case keywords_e::U8:
    case keywords_e::BOOL:
      return 1;
    case keywords_e::I16:
    case keywords_e::U16:
      return 2;
    case keywords_e::I32:
    case keywords_e::U32:
    case keywords_e::F32:
      return 4;
    case keywords_e::I64:
    case keywords_e::U64:
    case keywords_e::F64:
      return 8;
    default:
      break;
    }
  }
  return sizeof(void *);
}

void emitter_c::register_variable_type(const std::string &name,
                                       const type_c *type) {
  _variable_registry.register_variable(name, type, _type_registry);
}

bool emitter_c::is_variable_slice(const std::string &name) {
  return _variable_registry.is_slice(name);
}

bool emitter_c::is_variable_map(const std::string &name) {
  return _variable_registry.is_map(name);
}

bool emitter_c::is_variable_string_ptr(const std::string &name) {
  return _variable_registry.is_string_ptr(name);
}

void emitter_c::visit(const primitive_type_c &node) {
  _current_expr << emit_type(&node);
}

void emitter_c::visit(const named_type_c &node) {
  _current_expr << node.name().name;
}

void emitter_c::visit(const pointer_type_c &node) {
  _current_expr << emit_type(&node);
}

void emitter_c::visit(const array_type_c &node) {
  _current_expr << emit_type(&node);
}

void emitter_c::visit(const function_type_c &node) {}

void emitter_c::visit(const map_type_c &node) {
  _current_expr << get_map_type_name(node.key_type(), node.value_type());
}

void emitter_c::visit(const tuple_type_c &node) {
  std::vector<const type_c *> element_types;
  for (const auto &elem : node.element_types()) {
    element_types.push_back(elem.get());
  }
  _current_expr << get_tuple_type_name(element_types);
}

void emitter_c::visit(const generic_type_instantiation_c &node) {}

void emitter_c::visit(const fn_c &node) {
  if (_collecting_declarations) {
    _function_names.insert(node.name().name);

    if (node.name().name.rfind("test_", 0) == 0) {
      if (node.params().size() == 1) {
        const auto &param = node.params()[0];
        if (auto ptr_type = param.type.get()->as_pointer_type()) {
          if (auto named_type = ptr_type->pointee_type()->as_named_type()) {
            if (named_type->name().name == "__truk_test_context_s") {
              if (auto prim_ret = node.return_type()->as_primitive_type()) {
                if (prim_ret->keyword() == keywords_e::VOID) {
                  if (node.name().name == "test_setup") {
                    _result.metadata.has_test_setup = true;
                  } else if (node.name().name == "test_teardown") {
                    _result.metadata.has_test_teardown = true;
                  } else {
                    _result.metadata.test_functions.push_back(node.name().name);
                  }
                }
              }
            }
          }
        }
      }
    }

    return;
  }

  if (node.is_extern()) {
    return;
  }

  emission_phase_e saved_phase = _current_phase;
  std::string saved_context = _current_node_context;

  _current_phase = emission_phase_e::FUNCTION_DEFINITION;
  _current_node_context = "function '" + node.name().name + "'";

  bool is_private = is_private_identifier(node.name().name);
  bool is_library = _result.metadata.is_library();

  if (auto func_return = node.return_type()->as_function_type()) {
    // Function returning a function pointer - needs special syntax
    std::string ret_type = emit_type(func_return->return_type());

    if (is_private && is_library) {
      _functions << "static ";
    }

    _functions << ret_type << " (*" << node.name().name << "(";
  } else {
    std::string return_type = emit_type(node.return_type());

    if (is_private && is_library) {
      _functions << "static ";
    }

    _functions << return_type << " " << node.name().name << "(";
  }

  bool has_variadic = false;
  std::string variadic_name;
  size_t non_variadic_count = 0;

  for (size_t i = 0; i < node.params().size(); ++i) {
    const auto &param = node.params()[i];

    if (param.is_variadic) {
      has_variadic = true;
      variadic_name = param.name.name;
      register_variable_type(param.name.name, param.type.get());
      if (i > 0)
        _functions << ", ";
      _functions << "...";
    } else {
      if (i > 0)
        _functions << ", ";
      register_variable_type(param.name.name, param.type.get());

      if (auto func = param.type.get()->as_function_type()) {
        std::string ret_type = emit_type(func->return_type());
        _functions << ret_type << " (*" << param.name.name << ")(";

        const auto &func_param_types = func->param_types();
        for (size_t j = 0; j < func_param_types.size(); ++j) {
          if (j > 0) {
            _functions << ", ";
          }
          _functions << emit_type(func_param_types[j].get());
        }

        if (func_param_types.empty()) {
          _functions << "void";
        }

        if (func->has_variadic()) {
          if (!func_param_types.empty()) {
            _functions << ", ";
          }
          _functions << "...";
        }

        _functions << ")";
      } else {
        std::string param_type = emit_type(param.type.get());
        _functions << param_type << " " << param.name.name;
      }

      const type_c *current_type = param.type.get();
      while (auto arr = current_type->as_array_type()) {
        if (arr->size().has_value()) {
          _functions << "[" << arr->size().value() << "]";
          current_type = arr->element_type();
        } else {
          ensure_slice_typedef(arr->element_type());
          break;
        }
      }
      non_variadic_count++;
    }
  }

  _functions << ")";

  // If returning a function pointer, add the return type parameters
  if (auto func_return = node.return_type()->as_function_type()) {
    _functions << ")(";

    const auto &ret_param_types = func_return->param_types();
    for (size_t i = 0; i < ret_param_types.size(); ++i) {
      if (i > 0) {
        _functions << ", ";
      }
      _functions << emit_type(ret_param_types[i].get());
    }

    if (ret_param_types.empty()) {
      _functions << "void";
    }

    if (func_return->has_variadic()) {
      if (!ret_param_types.empty()) {
        _functions << ", ";
      }
      _functions << "...";
    }

    _functions << ")";
  }

  _current_function_name = node.name().name;
  _current_function_return_type = node.return_type();

  if (auto tuple = node.return_type()->as_tuple_type()) {
    std::vector<const type_c *> elem_types;
    for (const auto &elem : tuple->element_types()) {
      elem_types.push_back(elem.get());
    }
    ensure_tuple_typedef(elem_types);
    _current_tuple_return_types = elem_types;
  } else {
    _current_tuple_return_types.clear();
  }

  if (node.body()) {
    _functions << " ";
    if (has_variadic) {
      _functions << "{\n";
      _indent_level++;

      push_defer_scope(defer_scope_s::scope_type_e::FUNCTION, &node);

      _functions << cdef::indent(_indent_level) << "va_list __truk_va_args;\n";
      _functions << cdef::indent(_indent_level) << "va_start(__truk_va_args, ";
      _functions << node.params()[non_variadic_count - 1].name.name;
      _functions << ");\n";

      auto *body_block = node.body()->as_block();
      if (body_block) {
        for (const auto &stmt : body_block->statements()) {
          stmt->accept(*this);
        }
      }

      emit_scope_defers(_current_defer_scope);
      pop_defer_scope();

      _functions << cdef::indent(_indent_level) << "va_end(__truk_va_args);\n";
      _indent_level--;
      _functions << "}\n";
    } else {
      push_defer_scope(defer_scope_s::scope_type_e::FUNCTION, &node);

      if (auto *body_block = node.body()->as_block()) {
        _functions << "{\n";
        _indent_level++;
        for (const auto &stmt : body_block->statements()) {
          stmt->accept(*this);
        }
        emit_scope_defers(_current_defer_scope);
        _indent_level--;
        _functions << cdef::indent(_indent_level) << "}";
      } else {
        node.body()->accept(*this);
        emit_scope_defers(_current_defer_scope);
      }

      pop_defer_scope();
    }
  }

  _functions << "\n";

  _current_function_name = "";

  _current_phase = saved_phase;
  _current_node_context = saved_context;
}

void emitter_c::visit(const lambda_c &node) {
  if (_collecting_declarations) {
    return;
  }

  // Skip lambda generation if it's going to be inlined (e.g., in each())
  if (_skip_lambda_generation) {
    return;
  }

  emission_phase_e saved_phase = _current_phase;
  std::string saved_context = _current_node_context;
  std::string saved_function_name = _current_function_name;
  const type_c *saved_return_type = _current_function_return_type;
  std::vector<const type_c *> saved_tuple_return_types =
      _current_tuple_return_types;
  std::stringstream saved_functions;

  std::string lambda_name =
      "__truk_lambda_" + std::to_string(++_lambda_counter);
  _current_node_context = "lambda '" + lambda_name + "'";

  std::string return_type = emit_type(node.return_type());

  _header << "static " << return_type << " " << lambda_name << "(";

  saved_functions.swap(_functions);
  _functions.str("");
  _functions.clear();

  _functions << "static " << return_type << " " << lambda_name << "(";

  for (size_t i = 0; i < node.params().size(); ++i) {
    const auto &param = node.params()[i];
    if (i > 0) {
      _header << ", ";
      _functions << ", ";
    }

    register_variable_type(param.name.name, param.type.get());

    if (auto func = param.type.get()->as_function_type()) {
      std::string ret_type = emit_type(func->return_type());
      std::string func_param = ret_type + " (*" + param.name.name + ")(";

      const auto &func_param_types = func->param_types();
      for (size_t j = 0; j < func_param_types.size(); ++j) {
        if (j > 0) {
          func_param += ", ";
        }
        func_param += emit_type(func_param_types[j].get());
      }

      if (func_param_types.empty()) {
        func_param += "void";
      }

      if (func->has_variadic()) {
        if (!func_param_types.empty()) {
          func_param += ", ";
        }
        func_param += "...";
      }

      func_param += ")";

      _header << func_param;
      _functions << func_param;
    } else {
      std::string param_type = emit_type(param.type.get());
      _header << param_type << " " << param.name.name;
      _functions << param_type << " " << param.name.name;
    }

    const type_c *current_type = param.type.get();
    while (auto arr = current_type->as_array_type()) {
      if (arr->size().has_value()) {
        _header << "[" << arr->size().value() << "]";
        _functions << "[" << arr->size().value() << "]";
        current_type = arr->element_type();
      } else {
        ensure_slice_typedef(arr->element_type());
        break;
      }
    }
  }

  if (node.params().empty()) {
    _header << "void";
    _functions << "void";
  }

  _header << ");\n";
  _functions << ")";

  _current_phase = emission_phase_e::FUNCTION_DEFINITION;
  _current_function_name = lambda_name;
  _current_function_return_type = node.return_type();

  if (auto tuple = node.return_type()->as_tuple_type()) {
    std::vector<const type_c *> elem_types;
    for (const auto &elem : tuple->element_types()) {
      elem_types.push_back(elem.get());
    }
    ensure_tuple_typedef(elem_types);
    _current_tuple_return_types = elem_types;
  } else {
    _current_tuple_return_types.clear();
  }

  if (node.body()) {
    _functions << " ";
    push_defer_scope(defer_scope_s::scope_type_e::LAMBDA, &node);

    if (auto *body_block = node.body()->as_block()) {
      _functions << "{\n";
      _indent_level++;
      for (const auto &stmt : body_block->statements()) {
        stmt->accept(*this);
      }
      emit_scope_defers(_current_defer_scope);
      _indent_level--;
      _functions << cdef::indent(_indent_level) << "}";
    } else {
      node.body()->accept(*this);
      emit_scope_defers(_current_defer_scope);
    }

    pop_defer_scope();
  }

  _functions << "\n";

  std::string lambda_def = _functions.str();
  _functions.swap(saved_functions);
  _header << lambda_def;

  _current_expr << lambda_name;

  _current_function_name = saved_function_name;
  _current_function_return_type = saved_return_type;
  _current_tuple_return_types = saved_tuple_return_types;
  _current_phase = saved_phase;
  _current_node_context = saved_context;
}

void emitter_c::visit(const struct_c &node) {
  if (_collecting_declarations) {
    _type_registry.register_struct_name(node.name().name);
    if (node.is_extern()) {
      _type_registry.register_extern_struct_name(node.name().name);
    }
    if (node.is_generic()) {
      _type_registry.register_generic_struct(node.name().name);
    }
    return;
  }

  if (node.is_extern()) {
    return;
  }

  if (node.is_generic()) {
    return;
  }

  emission_phase_e saved_phase = _current_phase;
  std::string saved_context = _current_node_context;

  _current_phase = emission_phase_e::STRUCT_DEFINITION;
  _current_node_context = "struct '" + node.name().name + "'";

  _structs << "typedef struct " << node.name().name << " " << node.name().name
           << ";\n";
  _structs << "struct " << node.name().name << " {\n";

  for (const auto &field : node.fields()) {
    std::string field_type = emit_type(field.type.get());
    _structs << "  " << field_type << " " << field.name.name;

    const type_c *current_type = field.type.get();
    while (auto arr = current_type->as_array_type()) {
      if (arr->size().has_value()) {
        _structs << "[" << arr->size().value() << "]";
        current_type = arr->element_type();
      } else {
        ensure_slice_typedef(arr->element_type());
        break;
      }
    }

    _structs << ";\n";
  }

  _structs << "};\n\n";

  _current_phase = saved_phase;
  _current_node_context = saved_context;
}

void emitter_c::visit(const enum_c &node) {
  if (_collecting_declarations) {
    _enum_type_names.insert(node.name().name);
    if (node.is_extern()) {
      _extern_enum_type_names.insert(node.name().name);
    }
    return;
  }

  if (node.is_extern()) {
    return;
  }

  emission_phase_e saved_phase = _current_phase;
  std::string saved_context = _current_node_context;

  _current_phase = emission_phase_e::STRUCT_DEFINITION;
  _current_node_context = "enum '" + node.name().name + "'";

  std::string backing_type_str = emit_type(node.backing_type());

  _structs << "typedef enum {\n";

  bool first = true;
  for (const auto &value : node.values()) {
    if (!first) {
      _structs << ",\n";
    }
    first = false;

    _structs << "  " << node.name().name << "_" << value.name.name;

    if (value.explicit_value.has_value()) {
      _structs << " = " << value.explicit_value.value();
    }
  }

  _structs << "\n} " << node.name().name << ";\n\n";

  _current_phase = saved_phase;
  _current_node_context = saved_context;
}

void emitter_c::visit(const var_c &node) {
  if (_collecting_declarations && _indent_level == 0) {
    return;
  }

  if (node.is_extern()) {
    return;
  }

  register_variable_type(node.name().name, node.type());

  if (auto map = node.type()->as_map_type()) {
    ensure_map_typedef(map->key_type(), map->value_type());
  }

  bool is_private = is_private_identifier(node.name().name);
  bool is_library = _result.metadata.is_library();

  std::vector<size_t> array_dims;
  const type_c *base_type = node.type();
  while (auto arr = base_type->as_array_type()) {
    if (arr->size().has_value()) {
      array_dims.push_back(arr->size().value());
      base_type = arr->element_type();
    } else {
      ensure_slice_typedef(arr->element_type());
      break;
    }
  }

  if (auto func = base_type->as_function_type()) {
    std::string ret_type = emit_type(func->return_type());
    std::string func_decl = ret_type + " (*" + node.name().name;

    for (size_t dim : array_dims) {
      func_decl += "[" + std::to_string(dim) + "]";
    }

    func_decl += ")(";

    const auto &param_types = func->param_types();
    for (size_t i = 0; i < param_types.size(); ++i) {
      if (i > 0) {
        func_decl += ", ";
      }
      func_decl += emit_type(param_types[i].get());
    }

    if (param_types.empty()) {
      func_decl += "void";
    }

    if (func->has_variadic()) {
      if (!param_types.empty()) {
        func_decl += ", ";
      }
      func_decl += "...";
    }

    func_decl += ")";

    if (_indent_level == 0) {
      if (is_private && is_library) {
        _functions << "static ";
      }
      _functions << func_decl;
    } else {
      _functions << cdef::indent(_indent_level) << func_decl;
    }
  } else {
    std::string type_str = emit_type(node.type());

    if (_indent_level == 0) {
      if (is_private && is_library) {
        _functions << "static ";
      }
      _functions << type_str << " " << node.name().name;
    } else {
      _functions << cdef::indent(_indent_level) << type_str << " "
                 << node.name().name;
    }

    const type_c *current_type = node.type();
    while (auto arr = current_type->as_array_type()) {
      if (arr->size().has_value()) {
        _functions << "[" << arr->size().value() << "]";
        current_type = arr->element_type();
      } else {
        break;
      }
    }
  }

  if (node.initializer()) {
    std::string init = emit_expression(node.initializer());
    _functions << " = " << init;
  }

  _functions << ";\n";
}

void emitter_c::visit(const let_c &node) {
  if (_collecting_declarations && _indent_level == 0) {
    return;
  }

  if (node.is_single()) {
    const std::string &var_name = node.names()[0].name;

    if (var_name == "_") {
      if (node.initializer()) {
        std::string init = emit_expression(node.initializer());
        _functions << cdef::indent(_indent_level) << "(void)(" << init
                   << ");\n";
      }
      return;
    }

    auto var_type = node.inferred_types()[0].get();
    if (!var_type) {
      add_error("Cannot determine type for let variable: " + var_name, &node);
      return;
    }

    register_variable_type(var_name, var_type);

    if (auto map = var_type->as_map_type()) {
      ensure_map_typedef(map->key_type(), map->value_type());
    }

    bool is_private = is_private_identifier(var_name);
    bool is_library = _result.metadata.is_library();

    if (auto func = var_type->as_function_type()) {
      std::string ret_type = emit_type(func->return_type());
      std::string func_decl = ret_type + " (*" + var_name + ")(";

      const auto &param_types = func->param_types();
      for (size_t i = 0; i < param_types.size(); ++i) {
        if (i > 0) {
          func_decl += ", ";
        }
        func_decl += emit_type(param_types[i].get());
      }

      if (param_types.empty()) {
        func_decl += "void";
      }

      if (func->has_variadic()) {
        if (!param_types.empty()) {
          func_decl += ", ";
        }
        func_decl += "...";
      }

      func_decl += ")";

      if (_indent_level == 0) {
        if (is_private && is_library) {
          _functions << "static ";
        }
        _functions << func_decl;
      } else {
        _functions << cdef::indent(_indent_level) << func_decl;
      }
    } else {
      std::string type_str = emit_type(var_type);

      if (_indent_level == 0) {
        if (is_private && is_library) {
          _functions << "static ";
        }
        _functions << type_str << " " << var_name;
      } else {
        _functions << cdef::indent(_indent_level) << type_str << " "
                   << var_name;
      }
    }

    const type_c *current_type = var_type;
    while (auto arr = current_type->as_array_type()) {
      if (arr->size().has_value()) {
        _functions << "[" << arr->size().value() << "]";
        current_type = arr->element_type();
      } else {
        ensure_slice_typedef(arr->element_type());
        break;
      }
    }

    if (node.initializer()) {
      std::string init = emit_expression(node.initializer());
      _functions << " = " << init;
    }

    _functions << ";\n";
  } else {
    std::string tmp_var = "__tmp_" + std::to_string(_temp_counter++);
    std::string init = emit_expression(node.initializer());

    std::vector<const type_c *> tuple_types;
    for (const auto &type : node.inferred_types()) {
      tuple_types.push_back(type.get());
    }
    std::string tuple_type = get_tuple_type_name(tuple_types);

    _functions << cdef::indent(_indent_level);
    _functions << tuple_type << " " << tmp_var << " = " << init << ";\n";

    for (size_t i = 0; i < node.names().size(); ++i) {
      const std::string &var_name = node.names()[i].name;

      if (var_name == "_") {
        continue;
      }

      auto var_type = node.inferred_types()[i].get();
      register_variable_type(var_name, var_type);

      _functions << cdef::indent(_indent_level);

      std::vector<size_t> array_dims;
      const type_c *base_type = var_type;
      while (auto arr = base_type->as_array_type()) {
        if (arr->size().has_value()) {
          array_dims.push_back(arr->size().value());
          base_type = arr->element_type();
        } else {
          break;
        }
      }

      if (auto func = base_type->as_function_type()) {
        std::string ret_type = emit_type(func->return_type());
        _functions << ret_type << " (*" << var_name;
        for (size_t dim : array_dims) {
          _functions << "[" << dim << "]";
        }
        _functions << ")(";
        const auto &param_types = func->param_types();
        for (size_t j = 0; j < param_types.size(); ++j) {
          if (j > 0)
            _functions << ", ";
          _functions << emit_type(param_types[j].get());
        }
        if (param_types.empty())
          _functions << "void";
        if (func->has_variadic()) {
          if (!param_types.empty())
            _functions << ", ";
          _functions << "...";
        }
        _functions << ")";
        if (!array_dims.empty()) {
          _functions << ";\n";
          _functions << cdef::indent(_indent_level);
          _functions << "memcpy(" << var_name << ", " << tmp_var << "._" << i
                     << ", sizeof(" << var_name << "));\n";
        } else {
          _functions << " = " << tmp_var << "._" << i << ";\n";
        }
      } else if (is_array_type(var_type)) {
        std::string type_str = emit_type(var_type);
        std::string dims = get_array_dimensions(var_type);
        _functions << type_str << " " << var_name << dims << ";\n";
        _functions << cdef::indent(_indent_level);
        _functions << "memcpy(" << var_name << ", " << tmp_var << "._" << i
                   << ", sizeof(" << var_name << "));\n";
      } else {
        std::string type_str = emit_type(var_type);
        _functions << type_str << " " << var_name << " = ";
        _functions << tmp_var << "._" << i << ";\n";
      }
    }
  }
}

void emitter_c::visit(const const_c &node) {
  if (_collecting_declarations && _indent_level == 0) {
    return;
  }

  register_variable_type(node.name().name, node.type());

  std::string type_str = emit_type(node.type());

  if (_indent_level == 0) {
    _functions << "const " << type_str << " " << node.name().name;
  } else {
    _functions << cdef::indent(_indent_level) << "const " << type_str << " "
               << node.name().name;
  }

  const type_c *current_type = node.type();
  while (auto arr = current_type->as_array_type()) {
    if (arr->size().has_value()) {
      _functions << "[" << arr->size().value() << "]";
      current_type = arr->element_type();
    } else {
      break;
    }
  }

  std::string value = emit_expression(node.value());
  _functions << " = " << value << ";\n";
}

void emitter_c::visit(const if_c &node) {
  std::string condition = emit_expression(node.condition());
  _functions << cdef::indent(_indent_level) << "if (" << condition << ") ";

  node.then_block()->accept(*this);

  if (node.else_block()) {
    _functions << " else ";
    node.else_block()->accept(*this);
  }

  _functions << "\n";
}

void emitter_c::visit(const while_c &node) {
  std::string condition = emit_expression(node.condition());
  _functions << cdef::indent(_indent_level) << "while (" << condition << ") ";

  push_defer_scope(defer_scope_s::scope_type_e::LOOP, &node);

  if (auto *body_block = node.body()->as_block()) {
    _functions << "{\n";
    _indent_level++;
    for (const auto &stmt : body_block->statements()) {
      stmt->accept(*this);
    }
    emit_scope_defers(_current_defer_scope);
    _indent_level--;
    _functions << cdef::indent(_indent_level) << "}";
  } else {
    node.body()->accept(*this);
    emit_scope_defers(_current_defer_scope);
  }

  pop_defer_scope();

  _functions << "\n";
}

void emitter_c::visit(const for_c &node) {
  _functions << cdef::indent(_indent_level) << "for (";

  if (node.init()) {
    if (node.init()->as_var()) {
      std::stringstream temp_functions;
      std::swap(temp_functions, _functions);

      node.init()->accept(*this);

      std::string init_str = _functions.str();
      std::swap(temp_functions, _functions);

      while (!init_str.empty() &&
             (init_str.back() == '\n' || init_str.back() == ' ')) {
        init_str.pop_back();
      }
      if (!init_str.empty() && init_str.back() == ';') {
        init_str.pop_back();
      }
      _functions << init_str;
    } else {
      std::string init_expr = emit_expression(node.init());
      _functions << init_expr;
    }
  }
  _functions << "; ";

  if (node.condition()) {
    std::string cond_expr = emit_expression(node.condition());
    _functions << cond_expr;
  }
  _functions << "; ";

  if (node.post()) {
    std::string post_expr = emit_expression(node.post());
    _functions << post_expr;
  }

  _functions << ") ";

  push_defer_scope(defer_scope_s::scope_type_e::LOOP, &node);

  if (auto *body_block = node.body()->as_block()) {
    _functions << "{\n";
    _indent_level++;
    for (const auto &stmt : body_block->statements()) {
      stmt->accept(*this);
    }
    emit_scope_defers(_current_defer_scope);
    _indent_level--;
    _functions << cdef::indent(_indent_level) << "}";
  } else {
    node.body()->accept(*this);
    emit_scope_defers(_current_defer_scope);
  }

  pop_defer_scope();

  _functions << "\n";
}

void emitter_c::visit(const return_c &node) {
  if (node.is_void()) {
    emit_all_remaining_defers();
    _functions << cdef::indent(_indent_level) << "return;\n";
    return;
  }

  if (node.is_single()) {
    const base_c *return_expr = node.expressions()[0].get();
    bool is_call = (return_expr->as_call() != nullptr);

    if (is_call) {
      std::string return_type = emit_type(_current_function_return_type);
      std::string tmp_var = "__return_value_" + std::to_string(_temp_counter++);
      std::string expr = emit_expression(return_expr);

      _functions << cdef::indent(_indent_level) << return_type << " " << tmp_var
                 << " = " << expr << ";\n";
      emit_all_remaining_defers();
      _functions << cdef::indent(_indent_level) << "return " << tmp_var
                 << ";\n";
    } else {
      emit_all_remaining_defers();
      std::string expr = emit_expression(return_expr);
      _functions << cdef::indent(_indent_level) << "return " << expr << ";\n";
    }
    return;
  }

  if (node.is_multiple()) {
    std::string tmp_var = "__result";
    _functions << cdef::indent(_indent_level);
    _functions << get_tuple_type_name(_current_tuple_return_types);
    _functions << " " << tmp_var << ";\n";

    for (size_t i = 0; i < node.expressions().size(); ++i) {
      std::string expr = emit_expression(node.expressions()[i].get());
      _functions << cdef::indent(_indent_level);

      if (i < _current_tuple_return_types.size() &&
          is_array_type(_current_tuple_return_types[i])) {
        _functions << "memcpy(" << tmp_var << "._" << i << ", " << expr
                   << ", sizeof(" << tmp_var << "._" << i << "));\n";
      } else {
        _functions << tmp_var << "._" << i << " = " << expr << ";\n";
      }
    }

    emit_all_remaining_defers();
    _functions << cdef::indent(_indent_level) << "return " << tmp_var << ";\n";
  }
}

void emitter_c::visit(const break_c &node) {
  defer_scope_s *loop_scope = find_enclosing_loop_scope();

  defer_scope_s *scope = _current_defer_scope;
  while (scope && scope != loop_scope) {
    emit_scope_defers(scope);
    scope = scope->parent;
  }

  _functions << cdef::indent(_indent_level) << "break;\n";
}

void emitter_c::visit(const continue_c &node) {
  defer_scope_s *loop_scope = find_enclosing_loop_scope();

  defer_scope_s *scope = _current_defer_scope;
  while (scope && scope != loop_scope) {
    emit_scope_defers(scope);
    scope = scope->parent;
  }

  _functions << cdef::indent(_indent_level) << "continue;\n";
}

void emitter_c::visit(const defer_c &node) {
  if (node.deferred_code() && _current_defer_scope) {
    _current_defer_scope->defers.push_back(&node);
  }
}

void emitter_c::visit(const match_c &node) {
  std::string scrutinee_expr = emit_expression(node.scrutinee());
  std::string temp_var = "_truk_match_" + std::to_string(_match_counter++);

  _functions << cdef::indent(_indent_level) << "{\n";
  _indent_level++;
  _functions << cdef::indent(_indent_level) << "auto " << temp_var << " = "
             << scrutinee_expr << ";\n";

  bool first_case = true;
  for (const auto &case_arm : node.cases()) {
    if (case_arm.is_wildcard) {
      _functions << cdef::indent(_indent_level) << "else ";

      if (auto *block = case_arm.body->as_block()) {
        case_arm.body->accept(*this);
        _functions << "\n";
      } else {
        _functions << "{\n";
        _indent_level++;
        case_arm.body->accept(*this);
        _indent_level--;
        _functions << cdef::indent(_indent_level) << "}\n";
      }
    } else {
      if (first_case) {
        _functions << cdef::indent(_indent_level) << "if (";
        first_case = false;
      } else {
        _functions << cdef::indent(_indent_level) << "else if (";
      }

      std::string pattern_expr = emit_expression(case_arm.pattern.get());
      _functions << temp_var << " == " << pattern_expr << ") ";

      if (auto *block = case_arm.body->as_block()) {
        case_arm.body->accept(*this);
        _functions << "\n";
      } else {
        _functions << "{\n";
        _indent_level++;
        case_arm.body->accept(*this);
        _indent_level--;
        _functions << cdef::indent(_indent_level) << "}\n";
      }
    }
  }

  _indent_level--;
  _functions << cdef::indent(_indent_level) << "}\n";
}

void emitter_c::visit(const binary_op_c &node) {
  _current_expr << emit_expr_binary_op(node);
}

void emitter_c::visit(const unary_op_c &node) {
  _current_expr << emit_expr_unary_op(node);
}

void emitter_c::visit(const cast_c &node) {
  _current_expr << emit_expr_cast(node);
}

void emitter_c::visit(const call_c &node) {
  if (_in_expression) {
    _current_expr << emit_expr_call(node);
  } else {
    std::string call_expr = emit_expr_call(node);
    _functions << cdef::indent(_indent_level) << call_expr << ";\n";
  }
}

void emitter_c::visit(const index_c &node) {
  _current_expr << emit_expr_index(node);
}

void emitter_c::visit(const member_access_c &node) {
  _current_expr << emit_expr_member_access(node);
}

void emitter_c::visit(const literal_c &node) {
  _current_expr << emit_expr_literal(node);
}

void emitter_c::visit(const identifier_c &node) {
  _current_expr << emit_expr_identifier(node);
}

void emitter_c::visit(const assignment_c &node) {
  bool was_in_expr = _in_expression;

  if (auto idx = node.target()->as_index()) {
    bool is_slice = false;
    bool is_map = false;
    if (auto ident = idx->object()->as_identifier()) {
      is_slice = is_variable_slice(ident->id().name);
      is_map = is_variable_map(ident->id().name);
    } else if (auto inner_idx = idx->object()->as_index()) {
      is_slice = false;
      is_map = false;
    } else {
      is_slice = false;
      is_map = false;
    }

    if (is_map && !was_in_expr) {
      std::string obj_expr = emit_expression(idx->object());
      std::string idx_expr = emit_expression(idx->index());
      std::string value = emit_expression(node.value());

      bool key_is_slice = false;
      if (auto key_ident = idx->index()->as_identifier()) {
        key_is_slice = is_variable_slice(key_ident->id().name);
      }

      auto *key_literal = idx->index()->as_literal();
      bool key_is_string_literal =
          key_literal && key_literal->type() == literal_type_e::STRING;
      bool key_is_non_string_literal = key_literal && !key_is_string_literal;

      _functions << cdef::indent(_indent_level);
      _functions << "{ ";
      if (key_is_string_literal && !key_is_slice) {
        _functions << "const __truk_u8* __truk_key_tmp = " << idx_expr << "; ";
        _functions << "(" << obj_expr << ").tmp = " << value << "; ";
        _functions << "__truk_map_set_(&(" << obj_expr
                   << ").base, &__truk_key_tmp, &(" << obj_expr
                   << ").tmp, sizeof((" << obj_expr << ").tmp)); }\n";
      } else if (key_is_non_string_literal && !key_is_slice) {
        _functions << "typeof(" << idx_expr << ") __truk_key_tmp = " << idx_expr
                   << "; ";
        _functions << "(" << obj_expr << ").tmp = " << value << "; ";
        _functions << "__truk_map_set_(&(" << obj_expr
                   << ").base, &__truk_key_tmp, &(" << obj_expr
                   << ").tmp, sizeof((" << obj_expr << ").tmp)); }\n";
      } else {
        _functions << "(" << obj_expr << ").tmp = " << value << "; ";
        if (key_is_slice) {
          _functions << "__truk_map_set_(&(" << obj_expr << ").base, &(("
                     << idx_expr << ").data), &(" << obj_expr
                     << ").tmp, sizeof((" << obj_expr << ").tmp)); }\n";
        } else {
          _functions << "__truk_map_set_(&(" << obj_expr << ").base, &("
                     << idx_expr << "), &(" << obj_expr << ").tmp, sizeof(("
                     << obj_expr << ").tmp)); }\n";
        }
      }
      return;
    }

    if (is_slice && !was_in_expr) {
      std::string obj_expr = emit_expression(idx->object());
      std::string idx_expr = emit_expression(idx->index());
      std::string value = emit_expression(node.value());

      _functions << cdef::indent(_indent_level);
      _functions << "__truk_runtime_sxs_bounds_check(" << idx_expr << ", ("
                 << obj_expr << ").len);\n";
      _functions << cdef::indent(_indent_level);
      _functions << "(" << obj_expr << ").data[" << idx_expr << "] = " << value
                 << ";\n";
      return;
    }
  }

  std::string target = emit_expression(node.target());
  std::string value = emit_expression(node.value());

  if (was_in_expr) {
    _current_expr << target << " = " << value;
  } else {
    _functions << cdef::indent(_indent_level) << target << " = " << value
               << ";\n";
  }
}

void emitter_c::visit(const block_c &node) {
  _functions << "{\n";
  _indent_level++;

  push_defer_scope(defer_scope_s::scope_type_e::BLOCK, &node);

  for (const auto &stmt : node.statements()) {
    stmt->accept(*this);
  }

  emit_scope_defers(_current_defer_scope);
  pop_defer_scope();

  _indent_level--;
  _functions << cdef::indent(_indent_level) << "}";
}

void emitter_c::visit(const array_literal_c &node) {
  _current_expr << emit_expr_array_literal(node);
}

void emitter_c::visit(const struct_literal_c &node) {
  _current_expr << emit_expr_struct_literal(node);
}

void emitter_c::visit(const type_param_c &node) {}

std::string emitter_c::get_binary_op_string(binary_op_e op) {
  switch (op) {
  case binary_op_e::ADD:
    return "+";
  case binary_op_e::SUB:
    return "-";
  case binary_op_e::MUL:
    return "*";
  case binary_op_e::DIV:
    return "/";
  case binary_op_e::MOD:
    return "%";
  case binary_op_e::EQ:
    return "==";
  case binary_op_e::NE:
    return "!=";
  case binary_op_e::LT:
    return "<";
  case binary_op_e::LE:
    return "<=";
  case binary_op_e::GT:
    return ">";
  case binary_op_e::GE:
    return ">=";
  case binary_op_e::AND:
    return "&&";
  case binary_op_e::OR:
    return "||";
  case binary_op_e::BITWISE_AND:
    return "&";
  case binary_op_e::BITWISE_OR:
    return "|";
  case binary_op_e::BITWISE_XOR:
    return "^";
  case binary_op_e::LEFT_SHIFT:
    return "<<";
  case binary_op_e::RIGHT_SHIFT:
    return ">>";
  }
  return "";
}

std::string emitter_c::get_unary_op_string(unary_op_e op) {
  switch (op) {
  case unary_op_e::NEG:
    return "-";
  case unary_op_e::NOT:
    return "!";
  case unary_op_e::BITWISE_NOT:
    return "~";
  case unary_op_e::ADDRESS_OF:
    return "&";
  case unary_op_e::DEREF:
    return "*";
  }
  return "";
}

std::string emitter_c::emit_expr_binary_op(const binary_op_c &node) {
  std::string left = emit_expression(node.left());
  std::string right = emit_expression(node.right());
  std::string op = get_binary_op_string(node.op());
  return "(" + left + " " + op + " " + right + ")";
}

std::string emitter_c::emit_expr_unary_op(const unary_op_c &node) {
  std::string operand = emit_expression(node.operand());
  std::string op = get_unary_op_string(node.op());
  return "(" + op + operand + ")";
}

std::string emitter_c::emit_expr_cast(const cast_c &node) {
  std::string expr = emit_expression(node.expression());
  std::string type = emit_type(node.target_type());
  return "((" + type + ")" + expr + ")";
}

std::string emitter_c::emit_expr_literal(const literal_c &node) {
  switch (node.type()) {
  case literal_type_e::INTEGER: {
    const std::string &val = node.value();
    if (val.size() >= 2 && val[0] == '0' && val[1] == 'b') {
      unsigned long long result = 0;
      for (size_t i = 2; i < val.size(); ++i) {
        result = (result << 1) | (val[i] - '0');
      }
      return std::to_string(result);
    } else if (val.size() >= 2 && val[0] == '0' && val[1] == 'o') {
      unsigned long long result = 0;
      for (size_t i = 2; i < val.size(); ++i) {
        result = (result << 3) | (val[i] - '0');
      }
      return std::to_string(result);
    } else {
      return val;
    }
  }
  case literal_type_e::FLOAT:
    return node.value();
  case literal_type_e::STRING:
    return node.value();
  case literal_type_e::CHAR:
    return process_char_literal(node.value(), &node);
  case literal_type_e::BOOL:
    return (node.value() == "true" ? "true" : "false");
  case literal_type_e::NIL:
    return "NULL";
  }
  return "";
}

std::string emitter_c::process_char_literal(const std::string &lexeme,
                                            const base_c *node) {
  std::string content = lexeme.substr(1, lexeme.length() - 2);

  if (content[0] == '\\') {
    char escape_char = content[1];
    switch (escape_char) {
    case 'n':
      return "'\\n'";
    case 't':
      return "'\\t'";
    case 'r':
      return "'\\r'";
    case '0':
      return "'\\0'";
    case '\\':
      return "'\\\\'";
    case '\'':
      return "'\\''";
    case '"':
      return "'\\\"'";
    case 'x':
      return "'" + content + "'";
    }
  }

  return "'" + content + "'";
}

std::string emitter_c::emit_expr_identifier(const identifier_c &node) {
  return node.id().name;
}

std::string emitter_c::emit_expr_member_access(const member_access_c &node) {
  if (auto *id_node = node.object()->as_identifier()) {
    if (_enum_type_names.find(id_node->id().name) != _enum_type_names.end()) {
      if (_extern_enum_type_names.find(id_node->id().name) !=
          _extern_enum_type_names.end()) {
        return node.field().name;
      }
      return id_node->id().name + "_" + node.field().name;
    }
  }
  std::string obj = emit_expression(node.object());
  return obj + "." + node.field().name;
}

std::string emitter_c::emit_expr_array_literal(const array_literal_c &node) {
  std::string result = "{";
  for (size_t i = 0; i < node.elements().size(); ++i) {
    if (i > 0)
      result += ", ";
    result += emit_expression(node.elements()[i].get());
  }
  result += "}";
  return result;
}

std::string emitter_c::emit_expr_struct_literal(const struct_literal_c &node) {
  std::string struct_name = node.struct_name().name;

  if (node.is_generic()) {
    struct_name = node.struct_name().name;
    for (const auto &arg : node.type_arguments()) {
      struct_name += "_" + _type_registry.mangle_type_for_name(arg.get());
    }
  }

  std::string result = "(" + struct_name + "){";
  for (size_t i = 0; i < node.field_initializers().size(); ++i) {
    if (i > 0)
      result += ", ";
    const auto &field_init = node.field_initializers()[i];
    result += "." + field_init.field_name.name + " = ";
    result += emit_expression(field_init.value.get());
  }
  result += "}";
  return result;
}

std::string emitter_c::emit_expr_index(const index_c &node) {
  std::string obj_expr = emit_expression(node.object());
  std::string idx_expr = emit_expression(node.index());

  bool is_slice = false;
  bool is_map = false;
  if (auto ident = node.object()->as_identifier()) {
    is_slice = is_variable_slice(ident->id().name);
    is_map = is_variable_map(ident->id().name);
  } else if (auto inner_idx = node.object()->as_index()) {
    if (auto inner_ident = inner_idx->object()->as_identifier()) {
      if (is_variable_slice(inner_ident->id().name)) {
        is_slice = false;
      } else {
        is_slice = false;
      }
    } else {
      is_slice = false;
    }
  } else {
    is_slice = false;
  }

  if (is_map) {
    bool key_is_slice = false;
    auto *key_literal = node.index()->as_literal();
    bool key_is_string_literal =
        key_literal && key_literal->type() == literal_type_e::STRING;
    bool key_is_non_string_literal = key_literal && !key_is_string_literal;

    if (auto key_ident = node.index()->as_identifier()) {
      key_is_slice = is_variable_slice(key_ident->id().name);
    }

    if (key_is_slice) {
      return "__truk_map_get_generic(&(" + obj_expr + "), &((" + idx_expr +
             ").data))";
    } else if (key_is_string_literal) {
      return "({ const __truk_u8* __truk_key_tmp = " + idx_expr +
             "; __truk_map_get_generic(&(" + obj_expr +
             "), &__truk_key_tmp); })";
    } else if (key_is_non_string_literal) {
      return "({ typeof(" + idx_expr + ") __truk_key_tmp = " + idx_expr +
             "; __truk_map_get_generic(&(" + obj_expr +
             "), &__truk_key_tmp); })";
    } else {
      return "__truk_map_get_generic(&(" + obj_expr + "), &(" + idx_expr + "))";
    }
  } else if (is_slice) {
    return "({ __truk_runtime_sxs_bounds_check(" + idx_expr + ", (" + obj_expr +
           ").len); (" + obj_expr + ").data[" + idx_expr + "]; })";
  } else {
    return obj_expr + "[" + idx_expr + "]";
  }
}

std::string emitter_c::emit_expr_call(const call_c &node) {
  if (auto ident = node.callee()->as_identifier()) {
    if (auto *handler = _builtin_registry.get_handler(ident->id().name)) {
      std::stringstream temp_expr;
      std::swap(temp_expr, _current_expr);
      handler->emit_call(node, *this);
      std::string result = _current_expr.str();
      std::swap(temp_expr, _current_expr);
      return result;
    }
  }

  std::string callee = emit_expression(node.callee());
  std::string result = callee + "(";

  for (size_t i = 0; i < node.arguments().size(); ++i) {
    if (i > 0)
      result += ", ";
    result += emit_expression(node.arguments()[i].get());
  }

  result += ")";
  return result;
}

std::string emitter_c::emit_expression(const base_c *node) {
  if (!node)
    return "";

  expression_visitor_c expr_visitor(*this);
  node->accept(expr_visitor);
  return expr_visitor.get_result();
}

void emitter_c::push_defer_scope(defer_scope_s::scope_type_e type,
                                 const base_c *owner) {
  auto scope =
      std::make_unique<defer_scope_s>(type, owner, _current_defer_scope);
  _current_defer_scope = scope.get();
  _defer_scope_stack.push_back(std::move(scope));
}

void emitter_c::pop_defer_scope() {
  if (!_defer_scope_stack.empty()) {
    _defer_scope_stack.pop_back();
    _current_defer_scope =
        _defer_scope_stack.empty() ? nullptr : _defer_scope_stack.back().get();
  }
}

void emitter_c::emit_scope_defers(defer_scope_s *scope) {
  if (!scope) {
    return;
  }

  for (auto it = scope->defers.rbegin(); it != scope->defers.rend(); ++it) {
    const auto *defer_node = *it;
    if (defer_node->deferred_code()) {
      if (auto block = defer_node->deferred_code()->as_block()) {
        _functions << cdef::indent(_indent_level) << "{\n";
        _indent_level++;
        for (const auto &stmt : block->statements()) {
          stmt->accept(*this);
        }
        _indent_level--;
        _functions << cdef::indent(_indent_level) << "}\n";
      } else {
        std::string expr = emit_expression(defer_node->deferred_code());
        _functions << cdef::indent(_indent_level) << expr << ";\n";
      }
    }
  }
}

void emitter_c::emit_all_remaining_defers() {
  std::vector<defer_scope_s *> scopes_to_emit;
  defer_scope_s *scope = _current_defer_scope;

  while (scope) {
    scopes_to_emit.push_back(scope);
    scope = scope->parent;
  }

  for (auto *s : scopes_to_emit) {
    emit_scope_defers(s);
  }
}

defer_scope_s *emitter_c::find_enclosing_loop_scope() {
  defer_scope_s *scope = _current_defer_scope;

  while (scope) {
    if (scope->type == defer_scope_s::scope_type_e::LOOP) {
      return scope;
    }
    scope = scope->parent;
  }

  return nullptr;
}

void emitter_c::visit(const import_c &node) {}

void emitter_c::visit(const cimport_c &node) {}

void emitter_c::visit(const shard_c &node) {}

void emitter_c::visit(const enum_value_access_c &node) {
  _current_expr << node.enum_name().name << "_" << node.value_name().name;
}

std::string result_c::assemble_code() const {
  std::string output;
  for (const auto &chunk : chunks) {
    output += chunk;
  }

  if (!metadata.has_main_function) {
    return output;
  }

  std::string mangled_output;
  int main_index = 0;
  bool has_args = false;

  size_t pos = 0;
  while ((pos = output.find("__truk_i32 main(", pos)) != std::string::npos) {
    size_t line_start = output.rfind('\n', pos);
    if (line_start == std::string::npos) {
      line_start = 0;
    } else {
      line_start++;
    }

    bool is_function_def = true;
    for (size_t i = line_start; i < pos; ++i) {
      if (output[i] != ' ' && output[i] != '\t' && output[i] != '\n') {
        is_function_def = false;
        break;
      }
    }

    if (is_function_def) {
      mangled_output += output.substr(0, pos);

      size_t paren_end = output.find(')', pos);
      std::string params = output.substr(pos + 16, paren_end - (pos + 16));
      has_args = params.find("argc") != std::string::npos;

      mangled_output +=
          "__truk_i32 truk_main_" + std::to_string(main_index) + "(";
      output = output.substr(pos + 16);
      main_index++;
      pos = 0;
    } else {
      pos += 16;
    }
  }
  mangled_output += output;

  /*
      NOTE: At one point I would like to add debug information and flags to
     emitter to inject callbacks that run before/after the user program and
     potentially pass something hidden to the user's function so we can "poke
     around" in a debug mode easily

      This is where that would have to happen, naturally as this is where we
     call into the user's provided main (in the compiled target) to run whatever
     instructions they provided with truk files

      It would be kind of neat if we were to hash the truk files that we get
     per-build to make a fingerprint or identity for the app then the runtime
     could setup a shared memory space on the host env on launch if not exist
     scoped to the identity of the app, then all individual compiled processes
     could communicate IPC. If we restrict it to this build fingerprint we can
     be certain that the "other" instance is the same as us (operationally
     certain, assumed in good-faith) and that we can freely talk with it

      Eventually if that was a good idea the runtime could do some security
     shit, but the idea of each app running in parallel and the program being
     written to interact with itself to solve the task is a big dream of mine
  */
  mangled_output += fmt::format(R"(
int main(int argc, char** argv) {{
  __truk_runtime_sxs_target_app_s app = {{
    .entry_fn = (__truk_void*)truk_main_0,
    .has_args = {},
    .argc = argc,
    .argv = (__truk_i8**)argv
  }};
  return __truk_runtime_sxs_start(&app);
}}
)",
                                has_args ? "true" : "false");

  return mangled_output;
}

assembly_result_s result_c::assemble(assembly_type_e type,
                                     const std::string &header_name) const {
  if (type == assembly_type_e::APPLICATION) {
    return assembly_result_s(assembly_type_e::APPLICATION, assemble_code());
  }

  if (chunks.size() < 4) {
    throw emitter_exception_c("Invalid emission state: expected at least 4 "
                              "chunks for library assembly");
  }

  std::string header_content;
  std::string source_content;

  header_content += "#pragma once\n\n";
  header_content += cdef::emit_library_header();
  header_content += chunks[1];
  header_content += chunks[2];

  std::string functions_chunk = chunks[3];
  std::stringstream function_declarations;

  size_t pos = 0;
  while (pos < functions_chunk.size()) {
    size_t func_start = functions_chunk.find_first_not_of(" \t\n", pos);
    if (func_start == std::string::npos)
      break;

    size_t open_brace = functions_chunk.find('{', func_start);
    if (open_brace == std::string::npos)
      break;

    size_t line_end = functions_chunk.rfind('\n', open_brace);
    if (line_end == std::string::npos || line_end < func_start) {
      line_end = func_start;
    }

    std::string signature =
        functions_chunk.substr(func_start, open_brace - func_start);
    size_t last_newline = signature.find_last_of('\n');
    if (last_newline != std::string::npos) {
      signature = signature.substr(last_newline + 1);
    }

    while (!signature.empty() &&
           (signature.back() == ' ' || signature.back() == '\t')) {
      signature.pop_back();
    }

    bool is_static = signature.find("static ") == 0;

    if (!signature.empty() && !is_static) {
      function_declarations << signature << ";\n";
    }

    int brace_count = 1;
    size_t search_pos = open_brace + 1;
    while (search_pos < functions_chunk.size() && brace_count > 0) {
      if (functions_chunk[search_pos] == '{') {
        brace_count++;
      } else if (functions_chunk[search_pos] == '}') {
        brace_count--;
      }
      search_pos++;
    }

    pos = search_pos;
  }

  header_content += function_declarations.str();

  if (!header_name.empty()) {
    source_content += "#include \"" + header_name + "\"\n\n";
  }
  source_content += chunks[3];

  return assembly_result_s(assembly_type_e::LIBRARY, source_content,
                           header_content, header_name);
}

std::string result_c::assemble_test_runner() const {
  std::string output;

  if (metadata.test_functions.empty()) {
    for (const auto &chunk : chunks) {
      output += chunk;
    }
    return output;
  }

  for (const auto &chunk : chunks) {
    output += chunk;
  }

  output += "\nint main(int argc, char** argv) {\n";
  output += "    int total_tests = 0;\n";
  output += "    int total_failed = 0;\n\n";

  for (const auto &test_name : metadata.test_functions) {
    output += "    {\n";
    output += "        __truk_test_context_s ctx = {0};\n";
    output += "        ctx.current_test_name = \"" + test_name + "\";\n";
    output += "        ctx.argc = argc;\n";
    output += "        ctx.argv = argv;\n";
    output +=
        "        printf(\"Running %s...\\n\", ctx.current_test_name);\n\n";

    if (metadata.has_test_setup) {
      output += "        test_setup(&ctx);\n";
    }

    output += "        " + test_name + "(&ctx);\n\n";

    if (metadata.has_test_teardown) {
      output += "        test_teardown(&ctx);\n";
    }

    output += "        total_tests++;\n";
    output += "        if (ctx.has_failed) {\n";
    output += "            printf(\"  FAILED (%d/%d assertions)\\n\", "
              "ctx.failed, ctx.failed + ctx.passed);\n";
    output += "            total_failed++;\n";
    output += "        } else {\n";
    output +=
        "            printf(\"  PASSED (%d assertions)\\n\", ctx.passed);\n";
    output += "        }\n";
    output += "    }\n\n";
  }

  output += "    printf(\"\\n%d/%d tests passed\\n\", total_tests - "
            "total_failed, total_tests);\n";
  output += "    return total_failed;\n";
  output += "}\n";

  return output;
}

bool emitter_c::is_private_identifier(const std::string &name) const {
  return !name.empty() && name[0] == '_';
}

void emitter_c::collect_and_emit_generic_instantiations() {
  for (const auto *decl : _declarations) {
    if (auto *struct_node = decl->as_struct()) {
      if (struct_node->is_generic()) {
        _generic_definitions[struct_node->name().name] = struct_node;
      }
    }
  }

  instantiation_collector_c collector(_generic_definitions, _type_registry);
  for (const auto *decl : _declarations) {
    decl->accept(collector);
  }

  for (const auto &[generic_def, type_args, mangled_name] :
       collector.get_instantiations()) {
    emit_generic_instantiation(generic_def, type_args, mangled_name);
  }
}

void emitter_c::emit_generic_instantiation(
    const struct_c *generic_def, const std::vector<const type_c *> &type_args,
    const std::string &mangled_name) {

  if (_type_registry.is_instantiation_emitted(mangled_name)) {
    return;
  }

  _type_registry.register_instantiation(generic_def->name().name, type_args,
                                        mangled_name);

  std::unordered_map<std::string, const type_c *> substitutions;
  for (size_t i = 0; i < generic_def->type_params().size(); ++i) {
    substitutions[generic_def->type_params()[i].name] = type_args[i];
  }

  _structs << "typedef struct " << mangled_name << " " << mangled_name << ";\n";
  _structs << "struct " << mangled_name << " {\n";

  for (const auto &field : generic_def->fields()) {
    std::string field_type =
        emit_type_with_substitution(field.type.get(), substitutions);
    _structs << "  " << field_type << " " << field.name.name;

    const type_c *current_type = field.type.get();
    while (auto arr = current_type->as_array_type()) {
      if (arr->size().has_value()) {
        _structs << "[" << arr->size().value() << "]";
        current_type = arr->element_type();
      } else {
        auto substituted = substitute_type(arr->element_type(), substitutions);
        ensure_slice_typedef(substituted);
        break;
      }
    }

    _structs << ";\n";
  }

  _structs << "};\n\n";
}

std::string emitter_c::emit_type_with_substitution(
    const type_c *type,
    const std::unordered_map<std::string, const type_c *> &substitutions) {

  if (auto named = type->as_named_type()) {
    auto it = substitutions.find(named->name().name);
    if (it != substitutions.end()) {
      return emit_type(it->second);
    }
  }

  if (auto ptr = type->as_pointer_type()) {
    return emit_type_with_substitution(ptr->pointee_type(), substitutions) +
           "*";
  }

  if (auto arr = type->as_array_type()) {
    if (arr->size().has_value()) {
      return emit_type_with_substitution(arr->element_type(), substitutions);
    } else {
      auto substituted = substitute_type(arr->element_type(), substitutions);
      return get_slice_type_name(substituted);
    }
  }

  return emit_type(type);
}

const type_c *emitter_c::substitute_type(
    const type_c *type,
    const std::unordered_map<std::string, const type_c *> &substitutions) {

  if (auto named = type->as_named_type()) {
    auto it = substitutions.find(named->name().name);
    if (it != substitutions.end()) {
      return it->second;
    }
  }

  return type;
}

} // namespace truk::emitc
