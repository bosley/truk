#include "../include/typecheck.hpp"
#include <sstream>
#include <truk/ingestion/parser.hpp>

namespace truk::validation {

using namespace truk::language;
using namespace truk::language::nodes;

type_checker_c::type_checker_c() {
  register_builtin_types();
  register_builtin_functions();
}

void type_checker_c::check(const base_c *root) {
  if (root) {
    root->accept(*this);
  }
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

  return "<unknown>";
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
  }

  return false;
}

void type_checker_c::report_error(const std::string &message,
                                  std::size_t source_index) {
  std::ostringstream oss;
  oss << "[" << source_index << "] " << message;
  _errors.push_back(oss.str());
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

  if (actual_arg_count != expected_param_count) {
    report_error("Builtin '" + builtin->name + "' expects " +
                     std::to_string(expected_param_count) +
                     " argument(s) but got " + std::to_string(actual_arg_count),
                 node.source_index());
    return;
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

void type_checker_c::visit(const fn_c &node) {
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

  for (const auto &param : node.params()) {
    auto param_type = resolve_type(param.type.get());
    if (!param_type) {
      report_error("Unknown parameter type: " +
                       get_type_name_for_error(param.type.get()),
                   param.name.source_index);
      continue;
    }
    func_type->function_param_types.push_back(std::move(param_type));
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

void type_checker_c::visit(const struct_c &node) {
  auto incomplete_type =
      std::make_unique<type_entry_s>(type_kind_e::STRUCT, node.name().name);
  register_type(node.name().name, std::move(incomplete_type));

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
  auto var_type = resolve_type(node.type());
  if (!var_type) {
    report_error("Unknown variable type: " +
                     get_type_name_for_error(node.type()),
                 node.source_index());
    return;
  }

  if (node.initializer()) {
    node.initializer()->accept(*this);

    if (_current_expression_type &&
        !is_compatible_for_assignment(var_type.get(),
                                      _current_expression_type.get())) {
      report_error("Type mismatch in variable initialization",
                   node.source_index());
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

    if (_current_expression_type &&
        !is_compatible_for_assignment(const_type.get(),
                                      _current_expression_type.get())) {
      report_error("Type mismatch in constant initialization",
                   node.source_index());
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
      } else if (!types_equal(_current_function_return_type.get(),
                              _current_expression_type.get())) {
        report_error("Return type mismatch", node.source_index());
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

void type_checker_c::visit(const binary_op_c &node) {
  node.left()->accept(*this);
  auto left_type = std::move(_current_expression_type);

  node.right()->accept(*this);
  auto right_type = std::move(_current_expression_type);

  if (!left_type || !right_type) {
    report_error("Binary operation on invalid types", node.source_index());
    return;
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
      report_error("Arithmetic operation type mismatch", node.source_index());
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
    if (!types_equal(left_type.get(), right_type.get())) {
      report_error("Comparison operation type mismatch", node.source_index());
      return;
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
      _current_expression_type->pointer_depth--;
      if (_current_expression_type->pointer_depth == 0) {
        if (_current_expression_type->pointee_type) {
          _current_expression_type = std::make_unique<type_entry_s>(
              *_current_expression_type->pointee_type);
        } else {
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

void type_checker_c::visit(const call_c &node) {
  node.callee()->accept(*this);

  if (!_current_expression_type ||
      _current_expression_type->kind != type_kind_e::FUNCTION) {
    report_error("Call target is not a function", node.source_index());
    return;
  }

  auto func_type = std::move(_current_expression_type);

  if (func_type->is_builtin) {
    validate_builtin_call(node, *func_type);
    return;
  }

  if (node.arguments().size() != func_type->function_param_types.size()) {
    report_error("Argument count mismatch", node.source_index());
    return;
  }

  for (std::size_t i = 0; i < node.arguments().size(); ++i) {
    node.arguments()[i]->accept(*this);

    if (_current_expression_type &&
        !types_equal(_current_expression_type.get(),
                     func_type->function_param_types[i].get())) {
      report_error("Argument type mismatch", node.source_index());
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
    report_error("Index operation requires array or pointer type",
                 node.source_index());
  }
}

void type_checker_c::visit(const member_access_c &node) {
  node.object()->accept(*this);

  if (!_current_expression_type ||
      _current_expression_type->kind != type_kind_e::STRUCT) {
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

  _current_expression_type = std::make_unique<type_entry_s>(*it->second);
}

void type_checker_c::visit(const literal_c &node) {
  switch (node.type()) {
  case literal_type_e::INTEGER:
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "i32");
    break;
  case literal_type_e::FLOAT:
    _current_expression_type =
        std::make_unique<type_entry_s>(type_kind_e::PRIMITIVE, "f64");
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

  if (symbol->type) {
    _current_expression_type = std::make_unique<type_entry_s>(*symbol->type);
  }
}

void type_checker_c::visit(const assignment_c &node) {
  node.target()->accept(*this);
  auto target_type = std::move(_current_expression_type);

  node.value()->accept(*this);
  auto value_type = std::move(_current_expression_type);

  if (!target_type || !value_type) {
    report_error("Assignment with invalid types", node.source_index());
    return;
  }

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
  auto element_type = std::make_unique<type_entry_s>(*_current_expression_type);

  for (std::size_t i = 1; i < node.elements().size(); ++i) {
    node.elements()[i]->accept(*this);

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

    if (_current_expression_type &&
        !types_equal(it->second.get(), _current_expression_type.get())) {
      report_error("Field initializer type mismatch for: " + field_name,
                   node.source_index());
    }
  }

  _current_expression_type = std::make_unique<type_entry_s>(*struct_type);
}

void type_checker_c::visit(const type_param_c &node) {
  _current_expression_type.reset();
}

} // namespace truk::validation
