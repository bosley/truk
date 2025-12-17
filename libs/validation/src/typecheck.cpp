#include <sstream>
#include <truk/ingestion/parser.hpp>
#include <truk/validation/control_flow_checker.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::validation {

using namespace truk::language;
using namespace truk::language::nodes;

type_checker_c::type_checker_c() {
  register_builtin_types();
  register_builtin_functions();
}

void type_checker_c::check(const base_c *root) {
  if (!root) {
    return;
  }

  auto it = _decl_to_file.find(root);
  if (it != _decl_to_file.end()) {
    _current_file = it->second;
  }

  auto symbol_result = collect_symbols(root);
  _detailed_errors.insert(_detailed_errors.end(), symbol_result.errors.begin(),
                          symbol_result.errors.end());

  auto type_result = resolve_types(root, symbol_result);
  _detailed_errors.insert(_detailed_errors.end(), type_result.errors.begin(),
                          type_result.errors.end());

  auto control_flow_result = analyze_control_flow(root);
  _detailed_errors.insert(_detailed_errors.end(),
                          control_flow_result.errors.begin(),
                          control_flow_result.errors.end());

  auto lambda_capture_result = validate_lambda_captures(root, symbol_result);
  _detailed_errors.insert(_detailed_errors.end(),
                          lambda_capture_result.errors.begin(),
                          lambda_capture_result.errors.end());

  perform_type_checking(root, symbol_result, type_result);

  final_validation(symbol_result, type_result, control_flow_result,
                   lambda_capture_result);
}

void type_checker_c::push_scope() { _memory.push_ctx(); }

void type_checker_c::pop_scope() { _memory.pop_ctx(); }

void type_checker_c::register_builtin_types() {
  register_type("i8",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i8"));
  register_type("i16",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i16"));
  register_type("i32",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i32"));
  register_type("i64",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i64"));
  register_type("u8",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "u8"));
  register_type("u16",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "u16"));
  register_type("u32",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "u32"));
  register_type("u64",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "u64"));
  register_type("f32",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "f32"));
  register_type("f64",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "f64"));
  register_type("bool",
                std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "bool"));
  register_type("void",
                std::make_unique<type_entry_s>(type_kind_e::VOID_TYPE, "void"));
}

void type_checker_c::register_builtin_functions() {
  for (const auto &builtin : language::builtins::get_builtins()) {
    auto func_type =
        std::make_unique<type_entry_s>(type_kind_e::FUNCTION, builtin.name);

    func_type->is_builtin = true;
    func_type->builtin_kind = builtin.kind;
    func_type->is_variadic = builtin.is_variadic;

    register_symbol(builtin.name, std::move(func_type), false, 0);
  }
}

void type_checker_c::register_type(const std::string &name,
                                   std::unique_ptr<type_entry_s> type) {
  auto wrapper = std::make_unique<type_entry_s>(*type);
  _memory.set("__type__" + name, std::move(wrapper));
}

void type_checker_c::register_symbol(const std::string &name,
                                     std::unique_ptr<type_entry_s> type,
                                     bool is_mutable,
                                     std::size_t source_index) {
  auto symbol = std::make_unique<symbol_entry_s>(name, std::move(type),
                                                 is_mutable, source_index);
  _memory.set(name, std::move(symbol));
}

std::unique_ptr<type_entry_s>
type_checker_c::resolve_type(const type_c *type_node) {
  if (!type_node) {
    return nullptr;
  }

  if (auto *primitive = dynamic_cast<const primitive_type_c *>(type_node)) {
    std::string type_name =
        language::keywords_c::to_string(primitive->keyword());
    auto *base_type = lookup_type(type_name);
    if (!base_type) {
      return nullptr;
    }
    return std::make_unique<type_entry_s>(*base_type);
  }

  if (auto *named = dynamic_cast<const named_type_c *>(type_node)) {
    auto *base_type = lookup_type(named->name().name);
    if (!base_type) {
      return nullptr;
    }
    return std::make_unique<type_entry_s>(*base_type);
  }

  if (auto *pointer = dynamic_cast<const pointer_type_c *>(type_node)) {
    auto pointee = resolve_type(pointer->pointee_type());
    if (!pointee) {
      return nullptr;
    }

    auto resolved =
        std::make_unique<type_entry_s>(type_kind_e::POINTER, pointee->name);
    resolved->pointer_depth = pointee->pointer_depth + 1;
    resolved->pointee_type = std::move(pointee);
    return resolved;
  }

  if (auto *array = dynamic_cast<const array_type_c *>(type_node)) {
    auto element = resolve_type(array->element_type());
    if (!element) {
      return nullptr;
    }

    auto resolved =
        std::make_unique<type_entry_s>(type_kind_e::ARRAY, element->name);
    resolved->element_type = std::move(element);
    resolved->array_size = array->size();
    return resolved;
  }

  if (auto *function = dynamic_cast<const function_type_c *>(type_node)) {
    auto func_type =
        std::make_unique<type_entry_s>(type_kind_e::FUNCTION, "function");

    for (const auto &param_type : function->param_types()) {
      auto resolved_param = resolve_type(param_type.get());
      if (!resolved_param) {
        return nullptr;
      }
      func_type->function_param_types.push_back(std::move(resolved_param));
    }

    auto return_type = resolve_type(function->return_type());
    if (!return_type) {
      return nullptr;
    }
    func_type->function_return_type = std::move(return_type);

    return func_type;
  }

  if (auto *map = dynamic_cast<const map_type_c *>(type_node)) {
    auto key_type = resolve_type(map->key_type());
    auto value_type = resolve_type(map->value_type());

    if (!key_type || !value_type) {
      return nullptr;
    }

    auto resolved = std::make_unique<type_entry_s>(type_kind_e::MAP, "map");
    resolved->map_key_type = std::make_unique<type_entry_s>(*key_type);
    resolved->map_value_type = std::make_unique<type_entry_s>(*value_type);
    return resolved;
  }

  return nullptr;
}

std::string type_checker_c::get_type_name_for_error(const type_c *type_node) {
  if (!type_node) {
    return "<unknown>";
  }

  if (auto *primitive = dynamic_cast<const primitive_type_c *>(type_node)) {
    return language::keywords_c::to_string(primitive->keyword());
  }

  if (auto *named = dynamic_cast<const named_type_c *>(type_node)) {
    return named->name().name;
  }

  if (auto *pointer = dynamic_cast<const pointer_type_c *>(type_node)) {
    return "*" + get_type_name_for_error(pointer->pointee_type());
  }

  if (auto *array = dynamic_cast<const array_type_c *>(type_node)) {
    std::string size_str =
        array->size().has_value() ? std::to_string(array->size().value()) : "";
    return "[" + size_str + "]" +
           get_type_name_for_error(array->element_type());
  }

  if (auto *function = dynamic_cast<const function_type_c *>(type_node)) {
    return "fn";
  }

  if (auto *map = dynamic_cast<const map_type_c *>(type_node)) {
    return "map[" + get_type_name_for_error(map->key_type()) + ", " +
           get_type_name_for_error(map->value_type()) + "]";
  }

  return "<unknown>";
}

std::string type_checker_c::get_type_name_from_entry(const type_entry_s *type) {
  if (!type) {
    return "<unknown>";
  }

  std::string base_name = type->name;

  if (type->kind == type_kind_e::POINTER) {
    std::string result;
    for (std::size_t i = 0; i < type->pointer_depth; ++i) {
      result += "*";
    }
    result += base_name;
    return result;
  }

  if (type->kind == type_kind_e::ARRAY) {
    std::string size_str = type->array_size.has_value()
                               ? std::to_string(type->array_size.value())
                               : "";
    return "[" + size_str + "]" + base_name;
  }

  if (type->kind == type_kind_e::MAP) {
    if (type->map_key_type && type->map_value_type) {
      return "map[" + get_type_name_from_entry(type->map_key_type.get()) +
             ", " + get_type_name_from_entry(type->map_value_type.get()) + "]";
    }
    return "map[<unknown>, <unknown>]";
  }

  return base_name;
}

type_entry_s *type_checker_c::lookup_type(const std::string &name) {
  auto *item = _memory.get("__type__" + name, true);
  if (!item) {
    return nullptr;
  }
  return static_cast<type_entry_s *>(item);
}

symbol_entry_s *type_checker_c::lookup_symbol(const std::string &name) {
  auto *item = _memory.get(name, true);
  if (!item) {
    return nullptr;
  }
  return static_cast<symbol_entry_s *>(item);
}

bool type_checker_c::types_equal(const type_entry_s *a, const type_entry_s *b) {
  if (!a || !b) {
    return false;
  }

  if (a->kind == type_kind_e::UNTYPED_INTEGER ||
      a->kind == type_kind_e::UNTYPED_FLOAT ||
      b->kind == type_kind_e::UNTYPED_INTEGER ||
      b->kind == type_kind_e::UNTYPED_FLOAT) {
    return false;
  }

  if (a->kind != b->kind) {
    return false;
  }

  if (a->pointer_depth != b->pointer_depth) {
    return false;
  }

  if (a->name != b->name) {
    return false;
  }

  if (a->array_size != b->array_size) {
    return false;
  }

  if (a->kind == type_kind_e::ARRAY && a->element_type && b->element_type) {
    if (!types_equal(a->element_type.get(), b->element_type.get())) {
      return false;
    }
  }

  if (a->kind == type_kind_e::MAP) {
    if (a->map_key_type && b->map_key_type) {
      if (!types_equal(a->map_key_type.get(), b->map_key_type.get())) {
        return false;
      }
    }
    if (a->map_value_type && b->map_value_type) {
      if (!types_equal(a->map_value_type.get(), b->map_value_type.get())) {
        return false;
      }
    }
  }

  return true;
}

bool type_checker_c::is_numeric_type(const type_entry_s *type) {
  if (!type || type->kind != type_kind_e::PRIMITIVE) {
    return false;
  }

  return type->name == "i8" || type->name == "i16" || type->name == "i32" ||
         type->name == "i64" || type->name == "u8" || type->name == "u16" ||
         type->name == "u32" || type->name == "u64" || type->name == "f32" ||
         type->name == "f64";
}

bool type_checker_c::is_integer_type(const type_entry_s *type) {
  if (!type || type->kind != type_kind_e::PRIMITIVE) {
    return false;
  }

  return type->name == "i8" || type->name == "i16" || type->name == "i32" ||
         type->name == "i64" || type->name == "u8" || type->name == "u16" ||
         type->name == "u32" || type->name == "u64";
}

bool type_checker_c::is_boolean_type(const type_entry_s *type) {
  if (!type || type->kind != type_kind_e::PRIMITIVE) {
    return false;
  }

  return type->name == "bool";
}

bool type_checker_c::is_comparable_type(const type_entry_s *type) {
  if (!type) {
    return false;
  }

  if (is_numeric_type(type) || is_boolean_type(type)) {
    return true;
  }

  if (type->kind == type_kind_e::POINTER) {
    return true;
  }

  return false;
}

bool type_checker_c::is_valid_map_key_type(const type_entry_s *type) {
  if (!type) {
    return false;
  }

  if (type->kind == type_kind_e::PRIMITIVE) {
    return type->name == "i8" || type->name == "i16" || type->name == "i32" ||
           type->name == "i64" || type->name == "u8" || type->name == "u16" ||
           type->name == "u32" || type->name == "u64" || type->name == "f32" ||
           type->name == "f64" || type->name == "bool";
  }

  if (type->kind == type_kind_e::POINTER && type->pointer_depth == 1) {
    return type->name == "u8" || type->name == "i8";
  }

  return false;
}

bool type_checker_c::is_compatible_for_assignment(const type_entry_s *target,
                                                  const type_entry_s *source) {
  if (types_equal(target, source)) {
    return true;
  }

  if (is_numeric_type(target) && is_numeric_type(source)) {
    return true;
  }

  if (target->kind == type_kind_e::POINTER &&
      source->kind == type_kind_e::POINTER) {
    if (source->name == "void" || target->name == "void") {
      return true;
    }
    if ((target->name == "i8" && source->name == "u8") ||
        (target->name == "u8" && source->name == "i8")) {
      return true;
    }
  }

  if (target->kind == type_kind_e::FUNCTION &&
      source->kind == type_kind_e::FUNCTION) {
    if (target->function_param_types.size() !=
        source->function_param_types.size()) {
      return false;
    }

    for (size_t i = 0; i < target->function_param_types.size(); ++i) {
      if (!types_equal(target->function_param_types[i].get(),
                       source->function_param_types[i].get())) {
        return false;
      }
    }

    if (!types_equal(target->function_return_type.get(),
                     source->function_return_type.get())) {
      return false;
    }

    if (target->is_variadic != source->is_variadic) {
      return false;
    }

    return true;
  }

  return false;
}

void type_checker_c::report_error(const std::string &message,
                                  std::size_t source_index) {
  _detailed_errors.emplace_back(message, _current_file, source_index);
}

std::unique_ptr<type_entry_s>
type_checker_c::resolve_untyped_literal(const type_entry_s *literal_type,
                                        const type_entry_s *target_type) {

  if (!literal_type)
    return nullptr;

  if (literal_type->kind != type_kind_e::UNTYPED_INTEGER &&
      literal_type->kind != type_kind_e::UNTYPED_FLOAT) {
    return std::make_unique<type_entry_s>(*literal_type);
  }

  if (!target_type) {
    if (literal_type->kind == type_kind_e::UNTYPED_INTEGER) {
      return std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i32");
    } else {
      return std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "f64");
    }
  }

  if (literal_type->kind == type_kind_e::UNTYPED_INTEGER) {
    if (is_numeric_type(target_type) || is_integer_type(target_type)) {
      return std::make_unique<type_entry_s>(*target_type);
    }
  } else if (literal_type->kind == type_kind_e::UNTYPED_FLOAT) {
    if (is_numeric_type(target_type)) {
      return std::make_unique<type_entry_s>(*target_type);
    }
  }

  return literal_type->kind == type_kind_e::UNTYPED_INTEGER
             ? std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i32")
             : std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "f64");
}

bool type_checker_c::is_type_identifier(const identifier_c *id_node) {
  if (!id_node) {
    return false;
  }
  return lookup_type(id_node->id().name) != nullptr;
}

void type_checker_c::validate_builtin_call(const call_c &node,
                                           const type_entry_s &func_type) {
  if (!func_type.builtin_kind.has_value()) {
    report_error("Internal error: builtin has no kind", node.source_index());
    return;
  }

  const auto *builtin = language::builtins::lookup_builtin(func_type.name);
  if (!builtin) {
    report_error("Internal error: builtin not found in registry",
                 node.source_index());
    return;
  }

  if (func_type.builtin_kind == language::builtins::builtin_kind_e::MAKE) {
    if (node.arguments().empty()) {
      report_error("Builtin 'make' requires a type parameter",
                   node.source_index());
      return;
    }

    const auto *first_arg_type_param =
        dynamic_cast<const type_param_c *>(node.arguments()[0].get());
    if (!first_arg_type_param) {
      report_error(
          "Builtin 'make' requires a type parameter (use @type syntax)",
          node.source_index());
      return;
    }

    const type_c *type_param = first_arg_type_param->type();
    std::size_t actual_arg_count = node.arguments().size() - 1;

    if (actual_arg_count == 0) {
      auto resolved = resolve_type(type_param);
      if (!resolved) {
        report_error("Failed to resolve type for make", node.source_index());
        return;
      }

      if (resolved->kind == type_kind_e::MAP) {
        _current_expression_type = std::move(resolved);
        return;
      }

      auto return_type =
          std::make_unique<type_entry_s>(type_kind_e::POINTER, resolved->name);
      return_type->pointer_depth = resolved->pointer_depth + 1;
      return_type->pointee_type = std::move(resolved);
      _current_expression_type = std::move(return_type);
      return;
    } else if (actual_arg_count == 1) {
      node.arguments()[1]->accept(*this);
      auto count_type = std::move(_current_expression_type);
      if (!count_type || count_type->name != "u64") {
        report_error("Builtin 'make' array count must be u64",
                     node.source_index());
        return;
      }
      auto element = resolve_type(type_param);
      if (!element) {
        report_error("Failed to resolve element type for make",
                     node.source_index());
        return;
      }
      auto return_type =
          std::make_unique<type_entry_s>(type_kind_e::ARRAY, element->name);
      return_type->element_type = std::move(element);
      return_type->array_size = std::nullopt;
      _current_expression_type = std::move(return_type);
      return;
    } else {
      report_error("Builtin 'make' expects 1 or 2 arguments (type parameter + "
                   "optional count)",
                   node.source_index());
      return;
    }
  }

  if (func_type.builtin_kind == language::builtins::builtin_kind_e::DELETE) {
    if (node.arguments().size() != 1) {
      report_error("Builtin 'delete' expects 1 argument", node.source_index());
      return;
    }

    node.arguments()[0]->accept(*this);
    auto arg_type = std::move(_current_expression_type);

    if (!arg_type) {
      report_error("Failed to resolve argument type for delete",
                   node.source_index());
      return;
    }

    if (arg_type->kind != type_kind_e::POINTER &&
        arg_type->kind != type_kind_e::ARRAY &&
        arg_type->kind != type_kind_e::MAP) {
      report_error("Builtin 'delete' requires pointer, array, or map type",
                   node.source_index());
      return;
    }

    _current_expression_type.reset();
    return;
  }

  if (func_type.builtin_kind == language::builtins::builtin_kind_e::EACH) {
    if (node.arguments().size() != 3) {
      report_error("Builtin 'each' expects 3 arguments (collection, context, "
                   "and callback)",
                   node.source_index());
      return;
    }

    node.arguments()[0]->accept(*this);
    auto collection_type = std::move(_current_expression_type);

    bool is_map = collection_type && collection_type->kind == type_kind_e::MAP;
    bool is_slice = collection_type &&
                    collection_type->kind == type_kind_e::ARRAY &&
                    !collection_type->array_size.has_value();

    if (!is_map && !is_slice) {
      report_error("First argument to 'each' must be a map or slice",
                   node.source_index());
      return;
    }

    node.arguments()[1]->accept(*this);
    auto context_type = std::move(_current_expression_type);

    node.arguments()[2]->accept(*this);
    auto callback_type = std::move(_current_expression_type);
    if (!callback_type || callback_type->kind != type_kind_e::FUNCTION) {
      report_error("Third argument to 'each' must be a function",
                   node.source_index());
      return;
    }

    if (!callback_type->function_return_type ||
        callback_type->function_return_type->kind != type_kind_e::PRIMITIVE ||
        callback_type->function_return_type->name != "bool") {
      report_error("Callback to 'each' must return bool", node.source_index());
      return;
    }

    if (is_map) {
      if (callback_type->function_param_types.size() != 3) {
        report_error(
            "Callback to 'each' for map must take 3 parameters (key, value "
            "pointer, and context)",
            node.source_index());
        return;
      }

      auto &key_param = callback_type->function_param_types[0];

      if (!collection_type->map_key_type) {
        report_error("Map has no key type", node.source_index());
        return;
      }

      if (!types_equal(key_param.get(), collection_type->map_key_type.get())) {
        report_error(
            "First parameter of 'each' callback must match map key type: " +
                get_type_name_from_entry(collection_type->map_key_type.get()) +
                " but got " + get_type_name_from_entry(key_param.get()),
            node.source_index());
        return;
      }

      auto &value_param = callback_type->function_param_types[1];
      if (!value_param || value_param->kind != type_kind_e::POINTER) {
        report_error("Second parameter of 'each' callback for map must be a "
                     "pointer (value)",
                     node.source_index());
        return;
      }

      if (collection_type->map_value_type) {
        auto expected_value_type =
            std::make_unique<type_entry_s>(*collection_type->map_value_type);

        if (expected_value_type->kind == type_kind_e::POINTER) {
          expected_value_type->pointer_depth++;
        } else {
          auto pointee = std::move(expected_value_type);
          expected_value_type = std::make_unique<type_entry_s>(
              type_kind_e::POINTER, pointee->name);
          expected_value_type->pointer_depth = pointee->pointer_depth + 1;
          expected_value_type->pointee_type = std::move(pointee);
        }

        if (!types_equal(value_param.get(), expected_value_type.get())) {
          report_error(
              "Second parameter of 'each' callback must match map value type",
              node.source_index());
          return;
        }
      }
    } else {
      if (callback_type->function_param_types.size() != 2) {
        report_error(
            "Callback to 'each' for slice must take 2 parameters (element "
            "pointer and context)",
            node.source_index());
        return;
      }

      auto &element_param = callback_type->function_param_types[0];
      if (!element_param || element_param->kind != type_kind_e::POINTER) {
        report_error("First parameter of 'each' callback for slice must be a "
                     "pointer (element)",
                     node.source_index());
        return;
      }

      if (collection_type->element_type) {
        auto expected_element_type =
            std::make_unique<type_entry_s>(*collection_type->element_type);
        expected_element_type->pointer_depth = 1;
        expected_element_type->kind = type_kind_e::POINTER;

        if (!types_equal(element_param.get(), expected_element_type.get())) {
          report_error("First parameter of 'each' callback must match slice "
                       "element type",
                       node.source_index());
          return;
        }
      }
    }

    auto &context_param =
        callback_type
            ->function_param_types[callback_type->function_param_types.size() -
                                   1];
    if (!types_equal(context_param.get(), context_type.get())) {
      report_error("Last parameter of 'each' callback must match context type",
                   node.source_index());
      return;
    }

    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "void");
    return;
  }

  std::size_t expected_arg_start = 0;
  const type_c *type_param = nullptr;

  if (builtin->takes_type_param) {
    if (node.arguments().empty()) {
      report_error("Builtin '" + builtin->name + "' requires a type parameter",
                   node.source_index());
      return;
    }

    const auto *first_arg_type_param =
        dynamic_cast<const type_param_c *>(node.arguments()[0].get());
    if (!first_arg_type_param) {
      report_error("Builtin '" + builtin->name +
                       "' requires a type parameter (use @type syntax)",
                   node.source_index());
      return;
    }

    type_param = first_arg_type_param->type();
    expected_arg_start = 1;
  }

  auto signature = builtin->build_signature(type_param);

  if (!signature) {
    report_error("Failed to build signature for builtin '" + builtin->name +
                     "'",
                 node.source_index());
    return;
  }

  auto *func_sig = dynamic_cast<const function_type_c *>(signature.get());
  if (!func_sig) {
    report_error("Internal error: builtin signature is not a function type",
                 node.source_index());
    return;
  }

  std::size_t expected_param_count = func_sig->param_types().size();
  std::size_t actual_arg_count = node.arguments().size() - expected_arg_start;

  if (builtin->is_variadic) {
    if (actual_arg_count < expected_param_count) {
      report_error("Builtin '" + builtin->name + "' expects at least " +
                       std::to_string(expected_param_count) +
                       " argument(s) but got " +
                       std::to_string(actual_arg_count),
                   node.source_index());
      return;
    }
  } else {
    if (actual_arg_count != expected_param_count) {
      report_error("Builtin '" + builtin->name + "' expects " +
                       std::to_string(expected_param_count) +
                       " argument(s) but got " +
                       std::to_string(actual_arg_count),
                   node.source_index());
      return;
    }
  }

  for (std::size_t i = 0; i < expected_param_count; ++i) {
    node.arguments()[expected_arg_start + i]->accept(*this);

    auto expected_type = resolve_type(func_sig->param_types()[i].get());
    if (!expected_type) {
      report_error("Failed to resolve parameter type for builtin",
                   node.source_index());
      continue;
    }

    bool type_matches = false;
    if (_current_expression_type) {
      if (types_equal(_current_expression_type.get(), expected_type.get())) {
        type_matches = true;
      } else if (expected_type->kind == type_kind_e::POINTER &&
                 expected_type->name == "void" &&
                 _current_expression_type->kind == type_kind_e::POINTER) {
        type_matches = true;
      } else if (expected_type->kind == type_kind_e::ARRAY &&
                 expected_type->element_type &&
                 expected_type->element_type->name == "void" &&
                 _current_expression_type->kind == type_kind_e::ARRAY &&
                 expected_type->array_size ==
                     _current_expression_type->array_size) {
        type_matches = true;
      }
    }

    if (_current_expression_type && !type_matches) {
      report_error("Argument type mismatch in builtin '" + builtin->name + "'",
                   node.source_index());
    }
  }

  if (builtin->is_variadic) {
    for (std::size_t i = expected_param_count; i < actual_arg_count; ++i) {
      node.arguments()[expected_arg_start + i]->accept(*this);
    }
  }

  auto return_type = resolve_type(func_sig->return_type());
  if (return_type) {
    _current_expression_type = std::move(return_type);
  } else {
    _current_expression_type.reset();
  }
}

void type_checker_c::visit(const primitive_type_c &node) {
  auto keyword = node.keyword();
  std::string type_name;

  switch (keyword) {
  case keywords_e::I8:
    type_name = "i8";
    break;
  case keywords_e::I16:
    type_name = "i16";
    break;
  case keywords_e::I32:
    type_name = "i32";
    break;
  case keywords_e::I64:
    type_name = "i64";
    break;
  case keywords_e::U8:
    type_name = "u8";
    break;
  case keywords_e::U16:
    type_name = "u16";
    break;
  case keywords_e::U32:
    type_name = "u32";
    break;
  case keywords_e::U64:
    type_name = "u64";
    break;
  case keywords_e::F32:
    type_name = "f32";
    break;
  case keywords_e::F64:
    type_name = "f64";
    break;
  case keywords_e::BOOL:
    type_name = "bool";
    break;
  case keywords_e::VOID:
    type_name = "void";
    break;
  default:
    report_error("Unknown primitive type", node.source_index());
    return;
  }

  _current_expression_type =
      std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, type_name);
}

void type_checker_c::visit(const named_type_c &node) {
  auto *type = lookup_type(node.name().name);
  if (!type) {
    report_error("Unknown type: " + node.name().name, node.source_index());
    return;
  }

  _current_expression_type = std::make_unique<type_entry_s>(*type);
}

void type_checker_c::visit(const pointer_type_c &node) {
  node.pointee_type()->accept(*this);

  if (_current_expression_type) {
    _current_expression_type->pointer_depth++;
    _current_expression_type->kind = type_kind_e::POINTER;
  }
}

void type_checker_c::visit(const array_type_c &node) {
  node.element_type()->accept(*this);

  if (_current_expression_type) {
    auto element_type = std::move(_current_expression_type);
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::ARRAY, element_type->name);
    _current_expression_type->element_type = std::move(element_type);
    _current_expression_type->array_size = node.size();
  }
}

void type_checker_c::visit(const function_type_c &node) {
  auto func_type =
      std::make_unique<type_entry_s>(type_kind_e::FUNCTION, "function");

  for (const auto &param_type : node.param_types()) {
    param_type->accept(*this);
    if (_current_expression_type) {
      func_type->function_param_types.push_back(
          std::move(_current_expression_type));
    }
  }

  node.return_type()->accept(*this);
  if (_current_expression_type) {
    func_type->function_return_type = std::move(_current_expression_type);
  }

  _current_expression_type = std::move(func_type);
}

void type_checker_c::visit(const map_type_c &node) {
  auto key_type = resolve_type(node.key_type());
  auto value_type = resolve_type(node.value_type());

  if (!key_type) {
    report_error("Failed to resolve map key type", node.source_index());
    return;
  }

  if (!value_type) {
    report_error("Failed to resolve map value type", node.source_index());
    return;
  }

  if (!is_valid_map_key_type(key_type.get())) {
    report_error(
        "Invalid map key type: " + get_type_name_from_entry(key_type.get()) +
            ". Keys must be primitives (integers, floats, bool) or "
            "string pointers (*u8, *i8)",
        node.source_index());
    return;
  }

  auto map_type = std::make_unique<type_entry_s>(type_kind_e::MAP, "map");
  map_type->map_key_type = std::move(key_type);
  map_type->map_value_type = std::move(value_type);
  _current_expression_type = std::move(map_type);
}

void type_checker_c::visit(const fn_c &node) {
  auto it = _decl_to_file.find(&node);
  if (it != _decl_to_file.end()) {
    _function_to_file[node.name().name] = it->second;
    _current_file = it->second;
  }

  auto return_type = resolve_type(node.return_type());
  if (!return_type) {
    report_error("Unknown return type: " +
                     get_type_name_for_error(node.return_type()),
                 node.source_index());
    return;
  }

  auto func_type =
      std::make_unique<type_entry_s>(type_kind_e::FUNCTION, node.name().name);
  func_type->function_return_type =
      std::make_unique<type_entry_s>(*return_type);

  bool has_variadic = false;
  for (const auto &param : node.params()) {
    if (param.is_variadic) {
      has_variadic = true;
      func_type->is_variadic = true;
    } else {
      auto param_type = resolve_type(param.type.get());
      if (!param_type) {
        report_error("Unknown parameter type: " +
                         get_type_name_for_error(param.type.get()),
                     param.name.source_index);
        continue;
      }
      func_type->function_param_types.push_back(std::move(param_type));
    }
  }

  register_symbol(node.name().name, std::move(func_type), false,
                  node.source_index());

  push_scope();

  _current_function_return_type = std::move(return_type);

  for (const auto &param : node.params()) {
    auto param_type = resolve_type(param.type.get());
    if (param_type) {
      register_symbol(param.name.name, std::move(param_type), true,
                      param.name.source_index);
    }
  }

  if (node.body()) {
    node.body()->accept(*this);
  }

  _current_function_return_type.reset();

  pop_scope();
}

void type_checker_c::visit(const lambda_c &node) {
  auto return_type = resolve_type(node.return_type());
  if (!return_type) {
    report_error("Unknown return type in lambda: " +
                     get_type_name_for_error(node.return_type()),
                 node.source_index());
    return;
  }

  auto lambda_type =
      std::make_unique<type_entry_s>(type_kind_e::FUNCTION, "<lambda>");
  lambda_type->function_return_type =
      std::make_unique<type_entry_s>(*return_type);

  for (const auto &param : node.params()) {
    if (param.is_variadic) {
      lambda_type->is_variadic = true;
    } else {
      auto param_type = resolve_type(param.type.get());
      if (!param_type) {
        report_error("Unknown parameter type in lambda: " +
                         get_type_name_for_error(param.type.get()),
                     param.name.source_index);
        continue;
      }
      lambda_type->function_param_types.push_back(std::move(param_type));
    }
  }

  push_scope();

  _current_function_return_type = std::move(return_type);

  for (const auto &param : node.params()) {
    auto param_type = resolve_type(param.type.get());
    if (param_type) {
      register_symbol(param.name.name, std::move(param_type), true,
                      param.name.source_index);
    }
  }

  if (node.body()) {
    if (!check_no_break_or_continue(node.body())) {
      report_error("Lambda cannot contain break or continue statements",
                   node.source_index());
    }
    node.body()->accept(*this);
  }

  _current_function_return_type.reset();

  pop_scope();

  _current_expression_type = std::move(lambda_type);
}

void type_checker_c::visit(const struct_c &node) {
  auto it = _decl_to_file.find(&node);
  if (it != _decl_to_file.end()) {
    _struct_to_file[node.name().name] = it->second;
  }

  auto incomplete_type =
      std::make_unique<type_entry_s>(type_kind_e::STRUCT, node.name().name);
  register_type(node.name().name, std::move(incomplete_type));

  if (node.is_extern() && node.fields().empty()) {
    _memory.defer_hoist("__type__" + node.name().name);
    return;
  }

  for (const auto &field : node.fields()) {
    auto field_type = resolve_type(field.type.get());
    if (!field_type) {
      report_error("Unknown field type: " +
                       get_type_name_for_error(field.type.get()),
                   field.name.source_index);
      continue;
    }

    auto *registered_type = lookup_type(node.name().name);
    if (registered_type) {
      registered_type->struct_field_names.push_back(field.name.name);
      registered_type->struct_fields[field.name.name] = std::move(field_type);
    }
  }

  _memory.defer_hoist("__type__" + node.name().name);
}

void type_checker_c::visit(const var_c &node) {
  auto it = _decl_to_file.find(&node);
  if (it != _decl_to_file.end()) {
    _global_to_file[node.name().name] = it->second;
  }

  auto var_type = resolve_type(node.type());
  if (!var_type) {
    report_error("Unknown variable type: " +
                     get_type_name_for_error(node.type()),
                 node.source_index());
    return;
  }

  if (node.is_extern()) {
    if (node.initializer()) {
      report_error("extern var cannot have initializer", node.source_index());
    }
    register_symbol(node.name().name, std::move(var_type), false,
                    node.source_index());
    _memory.defer_hoist(node.name().name);
    return;
  }

  if (node.initializer()) {
    node.initializer()->accept(*this);

    if (_current_expression_type) {
      _current_expression_type = resolve_untyped_literal(
          _current_expression_type.get(), var_type.get());
      if (!is_compatible_for_assignment(var_type.get(),
                                        _current_expression_type.get())) {
        report_error("Type mismatch in variable initialization",
                     node.source_index());
      }
    }
  }

  register_symbol(node.name().name, std::move(var_type), true,
                  node.source_index());
}

void type_checker_c::visit(const const_c &node) {
  auto const_type = resolve_type(node.type());
  if (!const_type) {
    report_error("Unknown constant type: " +
                     get_type_name_for_error(node.type()),
                 node.source_index());
    return;
  }

  if (node.value()) {
    node.value()->accept(*this);

    if (_current_expression_type) {
      _current_expression_type = resolve_untyped_literal(
          _current_expression_type.get(), const_type.get());
      if (!is_compatible_for_assignment(const_type.get(),
                                        _current_expression_type.get())) {
        report_error("Type mismatch in constant initialization",
                     node.source_index());
      }
    }
  }

  register_symbol(node.name().name, std::move(const_type), false,
                  node.source_index());
}

void type_checker_c::visit(const if_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);

    if (_current_expression_type &&
        !is_boolean_type(_current_expression_type.get())) {
      report_error("If condition must be boolean type", node.source_index());
    }
  }

  if (node.then_block()) {
    node.then_block()->accept(*this);
  }

  if (node.else_block()) {
    node.else_block()->accept(*this);
  }
}

void type_checker_c::visit(const while_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);

    if (_current_expression_type &&
        !is_boolean_type(_current_expression_type.get())) {
      report_error("While condition must be boolean type", node.source_index());
    }
  }

  bool prev_in_loop = _in_loop;
  _in_loop = true;

  if (node.body()) {
    node.body()->accept(*this);
  }

  _in_loop = prev_in_loop;
}

void type_checker_c::visit(const for_c &node) {
  push_scope();

  if (node.init()) {
    node.init()->accept(*this);
  }

  if (node.condition()) {
    node.condition()->accept(*this);

    if (_current_expression_type &&
        !is_boolean_type(_current_expression_type.get())) {
      report_error("For condition must be boolean type", node.source_index());
    }
  }

  bool prev_in_loop = _in_loop;
  _in_loop = true;

  if (node.body()) {
    node.body()->accept(*this);
  }

  if (node.post()) {
    node.post()->accept(*this);
  }

  _in_loop = prev_in_loop;

  pop_scope();
}

void type_checker_c::visit(const return_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);

    if (_current_function_return_type) {
      if (!_current_expression_type) {
        report_error("Return expression has no type", node.source_index());
      } else {
        _current_expression_type =
            resolve_untyped_literal(_current_expression_type.get(),
                                    _current_function_return_type.get());
        if (!is_compatible_for_assignment(_current_function_return_type.get(),
                                          _current_expression_type.get())) {
          report_error("Return type mismatch", node.source_index());
        }
      }
    }
  } else {
    if (_current_function_return_type &&
        _current_function_return_type->name != "void") {
      report_error("Function must return a value", node.source_index());
    }
  }
}

void type_checker_c::visit(const break_c &node) {
  if (!_in_loop) {
    report_error("Break statement outside of loop", node.source_index());
  }
}

void type_checker_c::visit(const continue_c &node) {
  if (!_in_loop) {
    report_error("Continue statement outside of loop", node.source_index());
  }
}

void type_checker_c::visit(const defer_c &node) {
  if (node.deferred_code()) {
    if (!check_no_control_flow(node.deferred_code())) {
      report_error("Defer cannot contain return, break, or continue statements",
                   node.source_index());
    }
    node.deferred_code()->accept(*this);
  }
}

void type_checker_c::visit(const binary_op_c &node) {
  node.left()->accept(*this);
  auto left_type = std::move(_current_expression_type);

  node.right()->accept(*this);
  auto right_type = std::move(_current_expression_type);

  if (!left_type || !right_type) {
    report_error("Binary operation on invalid types", node.source_index());
    return;
  }

  if (left_type->kind == type_kind_e::UNTYPED_INTEGER ||
      left_type->kind == type_kind_e::UNTYPED_FLOAT) {
    left_type = resolve_untyped_literal(left_type.get(), right_type.get());
  }
  if (right_type->kind == type_kind_e::UNTYPED_INTEGER ||
      right_type->kind == type_kind_e::UNTYPED_FLOAT) {
    right_type = resolve_untyped_literal(right_type.get(), left_type.get());
  }

  switch (node.op()) {
  case binary_op_e::ADD:
  case binary_op_e::SUB:
  case binary_op_e::MUL:
  case binary_op_e::DIV:
  case binary_op_e::MOD:
    if (!is_numeric_type(left_type.get()) ||
        !is_numeric_type(right_type.get())) {
      report_error("Arithmetic operation requires numeric types",
                   node.source_index());
      return;
    }
    if (!types_equal(left_type.get(), right_type.get())) {
      std::string left_name = get_type_name_from_entry(left_type.get());
      std::string right_name = get_type_name_from_entry(right_type.get());
      report_error("Cannot perform arithmetic on " + left_name + " and " +
                       right_name + " (hint: use explicit cast)",
                   node.source_index());
      return;
    }
    _current_expression_type = std::move(left_type);
    break;

  case binary_op_e::EQ:
  case binary_op_e::NE:
  case binary_op_e::LT:
  case binary_op_e::LE:
  case binary_op_e::GT:
  case binary_op_e::GE:
    if (!is_comparable_type(left_type.get()) ||
        !is_comparable_type(right_type.get())) {
      report_error(
          "Comparison operation requires comparable types (numeric, bool, or "
          "pointer)",
          node.source_index());
      return;
    }
    if (!types_equal(left_type.get(), right_type.get())) {
      if (is_numeric_type(left_type.get()) &&
          is_numeric_type(right_type.get())) {
      } else if (left_type->kind == type_kind_e::POINTER &&
                 right_type->kind == type_kind_e::POINTER) {
        if (left_type->name == "void" || right_type->name == "void") {
        } else {
          std::string left_name = get_type_name_from_entry(left_type.get());
          std::string right_name = get_type_name_from_entry(right_type.get());
          report_error("Cannot compare " + left_name + " with " + right_name,
                       node.source_index());
          return;
        }
      } else {
        std::string left_name = get_type_name_from_entry(left_type.get());
        std::string right_name = get_type_name_from_entry(right_type.get());
        report_error("Cannot compare " + left_name + " with " + right_name,
                     node.source_index());
        return;
      }
    }
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "bool");
    break;

  case binary_op_e::AND:
  case binary_op_e::OR:
    if (!is_boolean_type(left_type.get()) ||
        !is_boolean_type(right_type.get())) {
      report_error("Logical operation requires boolean types",
                   node.source_index());
      return;
    }
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "bool");
    break;

  case binary_op_e::BITWISE_AND:
  case binary_op_e::BITWISE_OR:
  case binary_op_e::BITWISE_XOR:
  case binary_op_e::LEFT_SHIFT:
  case binary_op_e::RIGHT_SHIFT:
    if (!is_integer_type(left_type.get()) ||
        !is_integer_type(right_type.get())) {
      report_error("Bitwise operation requires integer types",
                   node.source_index());
      return;
    }
    if (!types_equal(left_type.get(), right_type.get())) {
      report_error("Bitwise operation type mismatch", node.source_index());
      return;
    }
    _current_expression_type = std::move(left_type);
    break;
  }
}

void type_checker_c::visit(const unary_op_c &node) {
  node.operand()->accept(*this);

  if (!_current_expression_type) {
    report_error("Unary operation on invalid type", node.source_index());
    return;
  }

  _current_expression_type =
      resolve_untyped_literal(_current_expression_type.get(), nullptr);

  switch (node.op()) {
  case unary_op_e::NEG:
    if (!is_numeric_type(_current_expression_type.get())) {
      report_error("Negation requires numeric type", node.source_index());
    }
    break;

  case unary_op_e::NOT:
    if (!is_boolean_type(_current_expression_type.get())) {
      report_error("Logical NOT requires boolean type", node.source_index());
    }
    break;

  case unary_op_e::BITWISE_NOT:
    if (!is_integer_type(_current_expression_type.get())) {
      report_error("Bitwise NOT requires integer type", node.source_index());
    }
    break;

  case unary_op_e::ADDRESS_OF: {
    if (_current_expression_type->kind == type_kind_e::FUNCTION) {
      report_error(
          "Cannot take address of function/lambda (functions are already "
          "function pointers)",
          node.source_index());
      return;
    }
    auto pointee = std::move(_current_expression_type);
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::POINTER, pointee->name);
    _current_expression_type->pointer_depth = pointee->pointer_depth + 1;
    _current_expression_type->pointee_type = std::move(pointee);
    break;
  }

  case unary_op_e::DEREF:
    if (_current_expression_type->pointer_depth == 0) {
      report_error("Dereference requires pointer type", node.source_index());
    } else {
      if (_current_expression_type->pointee_type) {
        _current_expression_type = std::make_unique<type_entry_s>(
            *_current_expression_type->pointee_type);
      } else {
        _current_expression_type->pointer_depth--;
        if (_current_expression_type->pointer_depth == 0) {
          auto *base_type = lookup_type(_current_expression_type->name);
          if (base_type) {
            _current_expression_type->kind = base_type->kind;
          } else {
            _current_expression_type->kind = type_kind_e::PRIMITIVE;
          }
        }
      }
    }
    break;
  }
}

void type_checker_c::visit(const cast_c &node) {
  node.expression()->accept(*this);

  if (!_current_expression_type) {
    report_error("Cast expression has invalid type", node.source_index());
    return;
  }

  auto target_type = resolve_type(node.target_type());
  if (!target_type) {
    report_error("Cast to unknown type", node.source_index());
    return;
  }

  _current_expression_type = std::move(target_type);
}

void type_checker_c::visit(const call_c &node) {
  std::string func_name;
  if (auto *id_node = dynamic_cast<const identifier_c *>(node.callee())) {
    func_name = id_node->id().name;
  }

  node.callee()->accept(*this);

  if (!_current_expression_type ||
      _current_expression_type->kind != type_kind_e::FUNCTION) {
    report_error("Call target is not a function", node.source_index());
    return;
  }

  auto func_type = std::move(_current_expression_type);

  if (!func_name.empty() && is_private_identifier(func_name)) {
    std::string func_file = get_defining_file_for_function(func_name);
    if (!func_file.empty() && func_file != _current_file &&
        !files_share_shard(func_file, _current_file)) {
      report_error("Cannot call private function '" + func_name +
                       "' from outside its defining file or shard",
                   node.source_index());
      return;
    }
  }

  if (func_type->is_builtin) {
    validate_builtin_call(node, *func_type);
    return;
  }

  std::size_t min_args = func_type->function_param_types.size();

  if (func_type->is_variadic) {
    if (node.arguments().size() < min_args) {
      report_error("Too few arguments for variadic function",
                   node.source_index());
      return;
    }
  } else {
    if (node.arguments().size() != min_args) {
      report_error("Argument count mismatch", node.source_index());
      return;
    }
  }

  for (std::size_t i = 0; i < node.arguments().size(); ++i) {
    node.arguments()[i]->accept(*this);

    if (i < min_args) {
      if (_current_expression_type) {
        _current_expression_type =
            resolve_untyped_literal(_current_expression_type.get(),
                                    func_type->function_param_types[i].get());
        if (!is_compatible_for_assignment(
                func_type->function_param_types[i].get(),
                _current_expression_type.get())) {
          report_error("Argument type mismatch", node.source_index());
        }
      }
    }
  }

  if (func_type->function_return_type) {
    _current_expression_type =
        std::make_unique<type_entry_s>(*func_type->function_return_type);
  } else {
    _current_expression_type.reset();
  }
}

void type_checker_c::visit(const index_c &node) {
  node.object()->accept(*this);
  auto object_type = std::move(_current_expression_type);

  node.index()->accept(*this);
  auto index_type = std::move(_current_expression_type);

  if (!object_type) {
    report_error("Index operation on invalid type", node.source_index());
    return;
  }

  if (object_type->kind == type_kind_e::MAP) {
    if (!index_type) {
      report_error("Map index has invalid type", node.source_index());
      return;
    }

    if (!object_type->map_key_type) {
      report_error("Map has no key type", node.source_index());
      return;
    }

    if (index_type->kind == type_kind_e::ARRAY &&
        !index_type->array_size.has_value() && index_type->element_type &&
        (index_type->element_type->name == "i8" ||
         index_type->element_type->name == "u8")) {
      auto string_ptr_type =
          std::make_unique<type_entry_s>(type_kind_e::POINTER, "u8");
      string_ptr_type->pointer_depth = 1;
      index_type = std::move(string_ptr_type);
    }

    index_type = resolve_untyped_literal(index_type.get(),
                                         object_type->map_key_type.get());

    bool key_types_compatible =
        types_equal(index_type.get(), object_type->map_key_type.get());

    if (!key_types_compatible && index_type->kind == type_kind_e::POINTER &&
        object_type->map_key_type->kind == type_kind_e::POINTER &&
        index_type->pointer_depth == 1 &&
        object_type->map_key_type->pointer_depth == 1 &&
        ((index_type->name == "i8" &&
          object_type->map_key_type->name == "u8") ||
         (index_type->name == "u8" &&
          object_type->map_key_type->name == "i8"))) {
      key_types_compatible = true;
    }

    if (!key_types_compatible) {
      report_error(
          "Map key type mismatch: expected " +
              get_type_name_from_entry(object_type->map_key_type.get()) +
              " but got " + get_type_name_from_entry(index_type.get()),
          node.source_index());
      return;
    }

    if (!object_type->map_value_type) {
      report_error("Map has no value type", node.source_index());
      return;
    }

    auto value_type =
        std::make_unique<type_entry_s>(*object_type->map_value_type);
    auto ptr_type =
        std::make_unique<type_entry_s>(type_kind_e::POINTER, value_type->name);
    ptr_type->pointer_depth = value_type->pointer_depth + 1;
    ptr_type->pointee_type = std::move(value_type);
    _current_expression_type = std::move(ptr_type);
    return;
  }

  if (index_type) {
    if (index_type->kind == type_kind_e::UNTYPED_INTEGER) {
      auto u64_type = lookup_type("u64");
      if (u64_type) {
        index_type = std::make_unique<type_entry_s>(*u64_type);
      } else {
        index_type = resolve_untyped_literal(index_type.get(), nullptr);
      }
    }
  }

  if (!index_type || !is_integer_type(index_type.get())) {
    report_error("Index must be integer type", node.source_index());
    return;
  }

  if (object_type->kind == type_kind_e::ARRAY) {
    if (object_type->element_type) {
      _current_expression_type =
          std::make_unique<type_entry_s>(*object_type->element_type);
    } else {
      report_error("Array has no element type", node.source_index());
    }
  } else if (object_type->kind == type_kind_e::POINTER &&
             object_type->pointer_depth > 0) {
    object_type->pointer_depth--;
    _current_expression_type = std::move(object_type);
  } else {
    report_error("Index operation requires array, pointer, or map type",
                 node.source_index());
  }
}

void type_checker_c::visit(const member_access_c &node) {
  node.object()->accept(*this);

  if (!_current_expression_type) {
    report_error("Member access requires struct type", node.source_index());
    return;
  }

  if (_current_expression_type->kind == type_kind_e::POINTER) {
    report_error("Cannot use '.' on pointer type, use '->' instead",
                 node.source_index());
    return;
  }

  if (_current_expression_type->kind != type_kind_e::STRUCT) {
    report_error("Member access requires struct type", node.source_index());
    return;
  }

  auto struct_type = std::move(_current_expression_type);
  const auto &field_name = node.field().name;

  auto it = struct_type->struct_fields.find(field_name);
  if (it == struct_type->struct_fields.end()) {
    report_error("Struct has no field: " + field_name, node.source_index());
    return;
  }

  if (is_private_identifier(field_name)) {
    std::string struct_file = get_defining_file_for_struct(struct_type->name);
    if (!struct_file.empty() && struct_file != _current_file &&
        !files_share_shard(struct_file, _current_file)) {
      report_error("Cannot access private field '" + field_name +
                       "' of struct '" + struct_type->name +
                       "' from outside its defining file or shard",
                   node.source_index());
      return;
    }
  }

  _current_expression_type = std::make_unique<type_entry_s>(*it->second);
}

void type_checker_c::visit(const literal_c &node) {
  switch (node.type()) {
  case literal_type_e::INTEGER:
    _current_expression_type = std::make_unique<type_entry_s>(
        type_kind_e::UNTYPED_INTEGER, "untyped_int");
    break;
  case literal_type_e::FLOAT:
    _current_expression_type = std::make_unique<type_entry_s>(
        type_kind_e::UNTYPED_FLOAT, "untyped_float");
    break;
  case literal_type_e::STRING:
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::POINTER, "u8");
    _current_expression_type->pointer_depth = 1;
    break;
  case literal_type_e::BOOL:
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "bool");
    break;
  case literal_type_e::NIL:
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::POINTER, "void");
    _current_expression_type->pointer_depth = 1;
    break;
  }
}

void type_checker_c::visit(const identifier_c &node) {
  auto *symbol = lookup_symbol(node.id().name);
  if (!symbol) {
    report_error("Undefined identifier: " + node.id().name,
                 node.source_index());
    return;
  }

  if (is_private_identifier(node.id().name)) {
    std::string global_file = get_defining_file_for_global(node.id().name);
    if (!global_file.empty() && global_file != _current_file &&
        !files_share_shard(global_file, _current_file)) {
      report_error("Cannot access private global variable '" + node.id().name +
                       "' from outside its defining file or shard",
                   node.source_index());
      return;
    }
  }

  if (symbol->type) {
    _current_expression_type = std::make_unique<type_entry_s>(*symbol->type);
  }
}

void type_checker_c::visit(const assignment_c &node) {
  bool is_map_assignment = false;
  const type_entry_s *map_value_type = nullptr;

  if (auto *index = dynamic_cast<const index_c *>(node.target())) {
    index->object()->accept(*this);
    auto object_type = std::move(_current_expression_type);

    if (object_type && object_type->kind == type_kind_e::MAP) {
      is_map_assignment = true;
      map_value_type = object_type->map_value_type.get();

      index->index()->accept(*this);
      auto index_type = std::move(_current_expression_type);

      if (!index_type) {
        report_error("Map index has invalid type", node.source_index());
        return;
      }

      if (!object_type->map_key_type) {
        report_error("Map has no key type", node.source_index());
        return;
      }

      if (index_type->kind == type_kind_e::ARRAY &&
          !index_type->array_size.has_value() && index_type->element_type &&
          (index_type->element_type->name == "i8" ||
           index_type->element_type->name == "u8")) {
        auto string_ptr_type =
            std::make_unique<type_entry_s>(type_kind_e::POINTER, "u8");
        string_ptr_type->pointer_depth = 1;
        index_type = std::move(string_ptr_type);
      }

      index_type = resolve_untyped_literal(index_type.get(),
                                           object_type->map_key_type.get());

      bool key_types_compatible =
          types_equal(index_type.get(), object_type->map_key_type.get());

      if (!key_types_compatible && index_type->kind == type_kind_e::POINTER &&
          object_type->map_key_type->kind == type_kind_e::POINTER &&
          index_type->pointer_depth == 1 &&
          object_type->map_key_type->pointer_depth == 1 &&
          ((index_type->name == "i8" &&
            object_type->map_key_type->name == "u8") ||
           (index_type->name == "u8" &&
            object_type->map_key_type->name == "i8"))) {
        key_types_compatible = true;
      }

      if (!key_types_compatible) {
        report_error(
            "Map key type mismatch: expected " +
                get_type_name_from_entry(object_type->map_key_type.get()) +
                " but got " + get_type_name_from_entry(index_type.get()),
            node.source_index());
        return;
      }

      node.value()->accept(*this);
      auto value_type = std::move(_current_expression_type);

      if (!value_type || !map_value_type) {
        report_error("Assignment with invalid types", node.source_index());
        return;
      }

      value_type = resolve_untyped_literal(value_type.get(), map_value_type);

      if (!is_compatible_for_assignment(map_value_type, value_type.get())) {
        report_error("Assignment type mismatch", node.source_index());
      }

      _current_expression_type.reset();
      return;
    }
  }

  node.target()->accept(*this);
  auto target_type = std::move(_current_expression_type);

  node.value()->accept(*this);
  auto value_type = std::move(_current_expression_type);

  if (!target_type || !value_type) {
    report_error("Assignment with invalid types", node.source_index());
    return;
  }

  value_type = resolve_untyped_literal(value_type.get(), target_type.get());

  if (!is_compatible_for_assignment(target_type.get(), value_type.get())) {
    report_error("Assignment type mismatch", node.source_index());
  }

  _current_expression_type = std::move(target_type);
}

void type_checker_c::visit(const block_c &node) {
  push_scope();

  for (const auto &stmt : node.statements()) {
    if (stmt) {
      stmt->accept(*this);
    }
  }

  pop_scope();
}

void type_checker_c::visit(const array_literal_c &node) {
  if (node.elements().empty()) {
    report_error("Cannot infer type of empty array literal",
                 node.source_index());
    return;
  }

  node.elements()[0]->accept(*this);
  auto element_type =
      resolve_untyped_literal(_current_expression_type.get(), nullptr);

  for (std::size_t i = 1; i < node.elements().size(); ++i) {
    node.elements()[i]->accept(*this);

    if (_current_expression_type) {
      _current_expression_type = resolve_untyped_literal(
          _current_expression_type.get(), element_type.get());
    }

    if (!types_equal(element_type.get(), _current_expression_type.get())) {
      report_error("Array literal elements have inconsistent types",
                   node.source_index());
      return;
    }
  }

  auto array_type =
      std::make_unique<type_entry_s>(type_kind_e::ARRAY, element_type->name);
  array_type->element_type = std::move(element_type);
  array_type->array_size = node.elements().size();

  _current_expression_type = std::move(array_type);
}

void type_checker_c::visit(const struct_literal_c &node) {
  auto *struct_type = lookup_type(node.struct_name().name);
  if (!struct_type || struct_type->kind != type_kind_e::STRUCT) {
    report_error("Unknown struct type: " + node.struct_name().name,
                 node.source_index());
    return;
  }

  for (const auto &field_init : node.field_initializers()) {
    const auto &field_name = field_init.field_name.name;

    auto it = struct_type->struct_fields.find(field_name);
    if (it == struct_type->struct_fields.end()) {
      report_error("Struct has no field: " + field_name, node.source_index());
      continue;
    }

    field_init.value->accept(*this);

    if (_current_expression_type) {
      _current_expression_type = resolve_untyped_literal(
          _current_expression_type.get(), it->second.get());
      if (!is_compatible_for_assignment(it->second.get(),
                                        _current_expression_type.get())) {
        report_error("Field initializer type mismatch for: " + field_name,
                     node.source_index());
      }
    }
  }

  _current_expression_type = std::make_unique<type_entry_s>(*struct_type);
}

void type_checker_c::visit(const type_param_c &node) {
  _current_expression_type.reset();
}

bool type_checker_c::check_no_control_flow(const base_c *node) {
  if (!node) {
    return true;
  }

  control_flow_checker_c checker;
  node->accept(checker);
  return !checker.has_control_flow();
}

bool type_checker_c::check_no_break_or_continue(const base_c *node) {
  if (!node) {
    return true;
  }

  control_flow_checker_c checker;
  node->accept(checker);
  return !checker.has_break_or_continue();
}

void type_checker_c::visit(const import_c &node) {}

void type_checker_c::visit(const cimport_c &node) {}

void type_checker_c::visit(const shard_c &node) {}

bool type_checker_c::is_private_identifier(const std::string &name) const {
  return !name.empty() && name[0] == '_';
}

std::string type_checker_c::get_defining_file_for_struct(
    const std::string &struct_name) const {
  auto it = _struct_to_file.find(struct_name);
  if (it != _struct_to_file.end()) {
    return it->second;
  }
  return "";
}

std::string type_checker_c::get_defining_file_for_function(
    const std::string &func_name) const {
  auto it = _function_to_file.find(func_name);
  if (it != _function_to_file.end()) {
    return it->second;
  }
  return "";
}

std::string type_checker_c::get_defining_file_for_global(
    const std::string &global_name) const {
  auto it = _global_to_file.find(global_name);
  if (it != _global_to_file.end()) {
    return it->second;
  }
  return "";
}

bool type_checker_c::files_share_shard(const std::string &file1,
                                       const std::string &file2) const {
  auto it1 = _file_to_shards.find(file1);
  auto it2 = _file_to_shards.find(file2);

  if (it1 == _file_to_shards.end() || it2 == _file_to_shards.end()) {
    return false;
  }

  for (const auto &shard1 : it1->second) {
    for (const auto &shard2 : it2->second) {
      if (shard1 == shard2) {
        return true;
      }
    }
  }

  return false;
}

symbol_collection_result_s type_checker_c::collect_symbols(const base_c *root) {
  symbol_collector_c collector(_memory, _decl_to_file);
  return collector.collect(root);
}

type_resolution_result_s
type_checker_c::resolve_types(const base_c *root,
                              const symbol_collection_result_s &symbols) {
  type_resolution_result_s result;
  return result;
}

control_flow_result_s type_checker_c::analyze_control_flow(const base_c *root) {
  control_flow_result_s result;

  control_flow_checker_c checker;
  if (root) {
    root->accept(checker);
  }

  if (checker.has_control_flow()) {
    result.nodes_with_control_flow.insert(root);
  }

  return result;
}

lambda_capture_result_s type_checker_c::validate_lambda_captures(
    const base_c *root, const symbol_collection_result_s &symbols) {
  lambda_capture_validator_c validator(symbols, _decl_to_file);
  return validator.validate(root);
}

void type_checker_c::perform_type_checking(
    const base_c *root, const symbol_collection_result_s &symbols,
    const type_resolution_result_s &types) {
  root->accept(*this);
}

void type_checker_c::final_validation(
    const symbol_collection_result_s &symbols,
    const type_resolution_result_s &types,
    const control_flow_result_s &control_flow,
    const lambda_capture_result_s &lambda_captures) {}

symbol_collector_c::symbol_collector_c(
    truk::core::memory_c<2048> &memory,
    const std::unordered_map<const base_c *, std::string> &decl_to_file)
    : _memory(memory), _decl_to_file(decl_to_file) {
  _result.global_scope =
      std::make_unique<scope_info_s>(scope_kind_e::GLOBAL, nullptr, nullptr);
  _current_scope = _result.global_scope.get();
}

symbol_collection_result_s symbol_collector_c::collect(const base_c *root) {
  if (root) {
    auto it = _decl_to_file.find(root);
    if (it != _decl_to_file.end()) {
      _current_file = it->second;
    }
    root->accept(*this);
  }
  return std::move(_result);
}

void symbol_collector_c::visit(const primitive_type_c &node) {}
void symbol_collector_c::visit(const named_type_c &node) {}
void symbol_collector_c::visit(const pointer_type_c &node) {}
void symbol_collector_c::visit(const array_type_c &node) {}
void symbol_collector_c::visit(const function_type_c &node) {}
void symbol_collector_c::visit(const map_type_c &node) {}

void symbol_collector_c::visit(const fn_c &node) {
  auto it = _decl_to_file.find(&node);
  if (it != _decl_to_file.end()) {
    _current_file = it->second;
  }

  auto func_type =
      std::make_unique<type_entry_s>(type_kind_e::FUNCTION, node.name().name);
  auto func_symbol = std::make_unique<symbol_entry_s>(
      node.name().name, std::move(func_type), false, node.source_index());
  func_symbol->scope_kind = symbol_scope_e::GLOBAL;
  func_symbol->declaring_node = &node;

  auto *func_symbol_ptr = func_symbol.get();
  _memory.set(node.name().name, std::move(func_symbol));
  _result.global_symbols[node.name().name] = func_symbol_ptr;
  _current_scope->symbols[node.name().name] = func_symbol_ptr;

  auto func_scope = std::make_unique<scope_info_s>(scope_kind_e::FUNCTION,
                                                   &node, _current_scope);
  auto *func_scope_ptr = func_scope.get();
  _current_scope->children.push_back(std::move(func_scope));
  _result.scope_map[&node] = func_scope_ptr;

  auto prev_scope = _current_scope;
  _current_scope = func_scope_ptr;

  _memory.push_ctx();

  for (const auto &param : node.params()) {
    if (!param.is_variadic) {
      auto param_type =
          std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "param");
      auto symbol = std::make_unique<symbol_entry_s>(
          param.name.name, std::move(param_type), true,
          param.name.source_index);
      symbol->scope_kind = symbol_scope_e::PARAMETER;
      symbol->declaring_node = &node;

      auto *symbol_ptr = symbol.get();
      _memory.set(param.name.name, std::move(symbol));
      _current_scope->symbols[param.name.name] = symbol_ptr;
    }
  }

  if (node.body()) {
    node.body()->accept(*this);
  }

  _memory.pop_ctx();
  _current_scope = prev_scope;
}

void symbol_collector_c::visit(const lambda_c &node) {
  _result.lambdas.push_back(&node);

  auto lambda_scope = std::make_unique<scope_info_s>(scope_kind_e::LAMBDA,
                                                     &node, _current_scope);
  auto *lambda_scope_ptr = lambda_scope.get();
  _current_scope->children.push_back(std::move(lambda_scope));
  _result.scope_map[&node] = lambda_scope_ptr;

  auto prev_scope = _current_scope;
  _current_scope = lambda_scope_ptr;

  _memory.push_ctx();

  for (const auto &param : node.params()) {
    if (!param.is_variadic) {
      auto param_type =
          std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "param");
      auto symbol = std::make_unique<symbol_entry_s>(
          param.name.name, std::move(param_type), true,
          param.name.source_index);
      symbol->scope_kind = symbol_scope_e::PARAMETER;
      symbol->declaring_node = &node;

      auto *symbol_ptr = symbol.get();
      _memory.set(param.name.name, std::move(symbol));
      _current_scope->symbols[param.name.name] = symbol_ptr;
    }
  }

  if (node.body()) {
    node.body()->accept(*this);
  }

  _memory.pop_ctx();
  _current_scope = prev_scope;
}

void symbol_collector_c::visit(const struct_c &node) {
  auto it = _decl_to_file.find(&node);
  if (it != _decl_to_file.end()) {
    _current_file = it->second;
  }
}

void symbol_collector_c::visit(const var_c &node) {
  auto it = _decl_to_file.find(&node);
  if (it != _decl_to_file.end()) {
    _current_file = it->second;
  }

  if (node.initializer()) {
    node.initializer()->accept(*this);
  }

  auto var_type = std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "var");
  auto var_symbol = std::make_unique<symbol_entry_s>(
      node.name().name, std::move(var_type), true, node.source_index());

  if (_current_scope->kind == scope_kind_e::GLOBAL) {
    var_symbol->scope_kind = symbol_scope_e::GLOBAL;
    _result.global_symbols[node.name().name] = var_symbol.get();
  } else if (_current_scope->kind == scope_kind_e::LAMBDA ||
             (_current_scope->parent &&
              _current_scope->parent->kind == scope_kind_e::LAMBDA)) {
    var_symbol->scope_kind = symbol_scope_e::LAMBDA_LOCAL;
  } else {
    var_symbol->scope_kind = symbol_scope_e::FUNCTION_LOCAL;
  }

  var_symbol->declaring_node = &node;

  auto *var_symbol_ptr = var_symbol.get();
  _memory.set(node.name().name, std::move(var_symbol));
  _current_scope->symbols[node.name().name] = var_symbol_ptr;
}

void symbol_collector_c::visit(const const_c &node) {
  if (node.value()) {
    node.value()->accept(*this);
  }
}

void symbol_collector_c::visit(const if_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.then_block()) {
    node.then_block()->accept(*this);
  }
  if (node.else_block()) {
    node.else_block()->accept(*this);
  }
}

void symbol_collector_c::visit(const while_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.body()) {
    node.body()->accept(*this);
  }
}

void symbol_collector_c::visit(const for_c &node) {
  _memory.push_ctx();

  auto for_scope = std::make_unique<scope_info_s>(scope_kind_e::BLOCK, &node,
                                                  _current_scope);
  auto *for_scope_ptr = for_scope.get();
  _current_scope->children.push_back(std::move(for_scope));
  _result.scope_map[&node] = for_scope_ptr;

  auto prev_scope = _current_scope;
  _current_scope = for_scope_ptr;

  if (node.init()) {
    node.init()->accept(*this);
  }
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.post()) {
    node.post()->accept(*this);
  }
  if (node.body()) {
    node.body()->accept(*this);
  }

  _current_scope = prev_scope;
  _memory.pop_ctx();
}

void symbol_collector_c::visit(const return_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);
  }
}

void symbol_collector_c::visit(const break_c &node) {}
void symbol_collector_c::visit(const continue_c &node) {}

void symbol_collector_c::visit(const defer_c &node) {
  if (node.deferred_code()) {
    node.deferred_code()->accept(*this);
  }
}

void symbol_collector_c::visit(const binary_op_c &node) {
  if (node.left()) {
    node.left()->accept(*this);
  }
  if (node.right()) {
    node.right()->accept(*this);
  }
}

void symbol_collector_c::visit(const unary_op_c &node) {
  if (node.operand()) {
    node.operand()->accept(*this);
  }
}

void symbol_collector_c::visit(const cast_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);
  }
}

void symbol_collector_c::visit(const call_c &node) {
  if (node.callee()) {
    node.callee()->accept(*this);
  }
  for (const auto &arg : node.arguments()) {
    if (arg) {
      arg->accept(*this);
    }
  }
}

void symbol_collector_c::visit(const index_c &node) {
  if (node.object()) {
    node.object()->accept(*this);
  }
  if (node.index()) {
    node.index()->accept(*this);
  }
}

void symbol_collector_c::visit(const member_access_c &node) {
  if (node.object()) {
    node.object()->accept(*this);
  }
}

void symbol_collector_c::visit(const literal_c &node) {}
void symbol_collector_c::visit(const identifier_c &node) {}

void symbol_collector_c::visit(const assignment_c &node) {
  if (node.target()) {
    node.target()->accept(*this);
  }
  if (node.value()) {
    node.value()->accept(*this);
  }
}

void symbol_collector_c::visit(const block_c &node) {
  _memory.push_ctx();

  auto block_scope = std::make_unique<scope_info_s>(scope_kind_e::BLOCK, &node,
                                                    _current_scope);
  auto *block_scope_ptr = block_scope.get();
  _current_scope->children.push_back(std::move(block_scope));
  _result.scope_map[&node] = block_scope_ptr;

  auto prev_scope = _current_scope;
  _current_scope = block_scope_ptr;

  for (const auto &stmt : node.statements()) {
    if (stmt) {
      stmt->accept(*this);
    }
  }

  _current_scope = prev_scope;
  _memory.pop_ctx();
}

void symbol_collector_c::visit(const array_literal_c &node) {
  for (const auto &elem : node.elements()) {
    if (elem) {
      elem->accept(*this);
    }
  }
}

void symbol_collector_c::visit(const struct_literal_c &node) {
  for (const auto &field : node.field_initializers()) {
    if (field.value) {
      field.value->accept(*this);
    }
  }
}

void symbol_collector_c::visit(const type_param_c &node) {}
void symbol_collector_c::visit(const import_c &node) {}
void symbol_collector_c::visit(const cimport_c &node) {}
void symbol_collector_c::visit(const shard_c &node) {}

lambda_capture_validator_c::lambda_capture_validator_c(
    const symbol_collection_result_s &symbols,
    const std::unordered_map<const base_c *, std::string> &decl_to_file)
    : _symbols(symbols), _decl_to_file(decl_to_file) {
  _current_scope = symbols.global_scope.get();
}

lambda_capture_result_s
lambda_capture_validator_c::validate(const base_c *root) {
  if (root) {
    auto it = _decl_to_file.find(root);
    if (it != _decl_to_file.end()) {
      _current_file = it->second;
    }
    root->accept(*this);
  }
  return std::move(_result);
}

void lambda_capture_validator_c::visit(const primitive_type_c &node) {}
void lambda_capture_validator_c::visit(const named_type_c &node) {}
void lambda_capture_validator_c::visit(const pointer_type_c &node) {}
void lambda_capture_validator_c::visit(const array_type_c &node) {}
void lambda_capture_validator_c::visit(const function_type_c &node) {}
void lambda_capture_validator_c::visit(const map_type_c &node) {}

void lambda_capture_validator_c::visit(const fn_c &node) {
  auto it = _symbols.scope_map.find(&node);
  if (it != _symbols.scope_map.end()) {
    auto prev_scope = _current_scope;
    _current_scope = it->second;

    if (node.body()) {
      node.body()->accept(*this);
    }

    _current_scope = prev_scope;
  }
}

void lambda_capture_validator_c::visit(const lambda_c &node) {
  auto it = _symbols.scope_map.find(&node);
  if (it != _symbols.scope_map.end()) {
    auto prev_scope = _current_scope;
    auto prev_lambda = _current_lambda;

    _current_scope = it->second;
    _current_lambda = &node;

    if (node.body()) {
      node.body()->accept(*this);
    }

    _current_lambda = prev_lambda;
    _current_scope = prev_scope;
  }
}

void lambda_capture_validator_c::visit(const struct_c &node) {}

void lambda_capture_validator_c::visit(const var_c &node) {
  if (node.initializer()) {
    node.initializer()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const const_c &node) {
  if (node.value()) {
    node.value()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const if_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.then_block()) {
    node.then_block()->accept(*this);
  }
  if (node.else_block()) {
    node.else_block()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const while_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.body()) {
    node.body()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const for_c &node) {
  auto it = _symbols.scope_map.find(&node);
  if (it != _symbols.scope_map.end()) {
    auto prev_scope = _current_scope;
    _current_scope = it->second;

    if (node.init()) {
      node.init()->accept(*this);
    }
    if (node.condition()) {
      node.condition()->accept(*this);
    }
    if (node.post()) {
      node.post()->accept(*this);
    }
    if (node.body()) {
      node.body()->accept(*this);
    }

    _current_scope = prev_scope;
  }
}

void lambda_capture_validator_c::visit(const return_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const break_c &node) {}
void lambda_capture_validator_c::visit(const continue_c &node) {}

void lambda_capture_validator_c::visit(const defer_c &node) {
  if (node.deferred_code()) {
    node.deferred_code()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const binary_op_c &node) {
  if (node.left()) {
    node.left()->accept(*this);
  }
  if (node.right()) {
    node.right()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const unary_op_c &node) {
  if (node.operand()) {
    node.operand()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const cast_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const call_c &node) {
  if (node.callee()) {
    node.callee()->accept(*this);
  }
  for (const auto &arg : node.arguments()) {
    if (arg) {
      arg->accept(*this);
    }
  }
}

void lambda_capture_validator_c::visit(const index_c &node) {
  if (node.object()) {
    node.object()->accept(*this);
  }
  if (node.index()) {
    node.index()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const member_access_c &node) {
  if (node.object()) {
    node.object()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const literal_c &node) {}

void lambda_capture_validator_c::visit(const identifier_c &node) {
  const std::string &name = node.id().name;

  if (!_current_lambda) {
    return;
  }

  scope_info_s *search_scope = _current_scope;
  scope_info_s *found_in_scope = nullptr;

  while (search_scope) {
    auto it = search_scope->symbols.find(name);
    if (it != search_scope->symbols.end()) {
      found_in_scope = search_scope;
      break;
    }
    search_scope = search_scope->parent;
  }

  if (!found_in_scope) {
    return;
  }

  if (found_in_scope->kind == scope_kind_e::GLOBAL) {
    return;
  }

  scope_info_s *lambda_scope = nullptr;
  auto lambda_it = _symbols.scope_map.find(_current_lambda);
  if (lambda_it != _symbols.scope_map.end()) {
    lambda_scope = lambda_it->second;
  }

  if (!lambda_scope) {
    return;
  }

  bool is_lambda_local = false;
  scope_info_s *check_scope = found_in_scope;
  while (check_scope) {
    if (check_scope == lambda_scope) {
      is_lambda_local = true;
      break;
    }
    if (check_scope->kind == scope_kind_e::GLOBAL) {
      break;
    }
    check_scope = check_scope->parent;
  }

  if (!is_lambda_local) {
    std::string error_msg =
        "Lambda cannot capture variable '" + name +
        "' from enclosing scope. Use context parameter instead.";
    _result.errors.push_back(
        type_error_s(error_msg, _current_file, node.source_index()));
    _result.captured_vars[_current_lambda].push_back(name);
  }
}

void lambda_capture_validator_c::visit(const assignment_c &node) {
  if (node.target()) {
    node.target()->accept(*this);
  }
  if (node.value()) {
    node.value()->accept(*this);
  }
}

void lambda_capture_validator_c::visit(const block_c &node) {
  auto it = _symbols.scope_map.find(&node);
  if (it != _symbols.scope_map.end()) {
    auto prev_scope = _current_scope;
    _current_scope = it->second;

    for (const auto &stmt : node.statements()) {
      if (stmt) {
        stmt->accept(*this);
      }
    }

    _current_scope = prev_scope;
  } else {
    for (const auto &stmt : node.statements()) {
      if (stmt) {
        stmt->accept(*this);
      }
    }
  }
}

void lambda_capture_validator_c::visit(const array_literal_c &node) {
  for (const auto &elem : node.elements()) {
    if (elem) {
      elem->accept(*this);
    }
  }
}

void lambda_capture_validator_c::visit(const struct_literal_c &node) {
  for (const auto &field : node.field_initializers()) {
    if (field.value) {
      field.value->accept(*this);
    }
  }
}

void lambda_capture_validator_c::visit(const type_param_c &node) {}
void lambda_capture_validator_c::visit(const import_c &node) {}
void lambda_capture_validator_c::visit(const cimport_c &node) {}
void lambda_capture_validator_c::visit(const shard_c &node) {}

} // namespace truk::validation
