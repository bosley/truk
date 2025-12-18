#include <truk/emitc/instantiation_collector.hpp>

namespace truk::emitc {

using namespace truk::language::nodes;

instantiation_collector_c::instantiation_collector_c(
    const std::unordered_map<std::string, const struct_c *> &generic_defs,
    type_registry_c &registry)
    : _generic_defs(generic_defs), _registry(registry) {}

const std::vector<
    std::tuple<const struct_c *, std::vector<const type_c *>, std::string>> &
instantiation_collector_c::get_instantiations() const {
  return _instantiations;
}

void instantiation_collector_c::visit(
    const generic_type_instantiation_c &node) {
  auto it = _generic_defs.find(node.base_name().name);
  if (it != _generic_defs.end()) {
    std::vector<const type_c *> type_args;
    for (const auto &arg : node.type_arguments()) {
      type_args.push_back(arg.get());
    }

    std::string mangled =
        _registry.get_instantiated_name(node.base_name().name, type_args);

    if (!_registry.is_instantiation_emitted(mangled)) {
      _instantiations.push_back({it->second, type_args, mangled});
    }
  }
}

void instantiation_collector_c::visit(const primitive_type_c &node) {}

void instantiation_collector_c::visit(const named_type_c &node) {}

void instantiation_collector_c::visit(const pointer_type_c &node) {
  if (node.pointee_type())
    node.pointee_type()->accept(*this);
}

void instantiation_collector_c::visit(const array_type_c &node) {
  if (node.element_type())
    node.element_type()->accept(*this);
}

void instantiation_collector_c::visit(const function_type_c &node) {
  for (const auto &param : node.param_types()) {
    if (param)
      param->accept(*this);
  }
  if (node.return_type())
    node.return_type()->accept(*this);
}

void instantiation_collector_c::visit(const map_type_c &node) {
  if (node.key_type())
    node.key_type()->accept(*this);
  if (node.value_type())
    node.value_type()->accept(*this);
}

void instantiation_collector_c::visit(const tuple_type_c &node) {
  for (const auto &elem : node.element_types()) {
    if (elem)
      elem->accept(*this);
  }
}

void instantiation_collector_c::visit(const var_c &node) {
  if (node.type())
    node.type()->accept(*this);
  if (node.initializer())
    node.initializer()->accept(*this);
}

void instantiation_collector_c::visit(const fn_c &node) {
  if (node.return_type())
    node.return_type()->accept(*this);
  for (const auto &param : node.params()) {
    if (param.type)
      param.type->accept(*this);
  }
  if (node.body())
    node.body()->accept(*this);
}

void instantiation_collector_c::visit(const lambda_c &node) {
  if (node.return_type())
    node.return_type()->accept(*this);
  for (const auto &param : node.params()) {
    if (param.type)
      param.type->accept(*this);
  }
  if (node.body())
    node.body()->accept(*this);
}

void instantiation_collector_c::visit(const struct_c &node) {
  for (const auto &field : node.fields()) {
    if (field.type)
      field.type->accept(*this);
  }
}

void instantiation_collector_c::visit(const enum_c &node) {
  if (node.backing_type())
    node.backing_type()->accept(*this);
}

void instantiation_collector_c::visit(const const_c &node) {
  if (node.type())
    node.type()->accept(*this);
  if (node.value())
    node.value()->accept(*this);
}

void instantiation_collector_c::visit(const let_c &node) {
  if (node.initializer())
    node.initializer()->accept(*this);
}

void instantiation_collector_c::visit(const if_c &node) {
  if (node.condition())
    node.condition()->accept(*this);
  if (node.then_block())
    node.then_block()->accept(*this);
  if (node.else_block())
    node.else_block()->accept(*this);
}

void instantiation_collector_c::visit(const while_c &node) {
  if (node.condition())
    node.condition()->accept(*this);
  if (node.body())
    node.body()->accept(*this);
}

void instantiation_collector_c::visit(const for_c &node) {
  if (node.init())
    node.init()->accept(*this);
  if (node.condition())
    node.condition()->accept(*this);
  if (node.post())
    node.post()->accept(*this);
  if (node.body())
    node.body()->accept(*this);
}

void instantiation_collector_c::visit(const return_c &node) {
  for (const auto &expr : node.expressions()) {
    if (expr)
      expr->accept(*this);
  }
}

void instantiation_collector_c::visit(const break_c &node) {}

void instantiation_collector_c::visit(const continue_c &node) {}

void instantiation_collector_c::visit(const defer_c &node) {
  if (node.deferred_code())
    node.deferred_code()->accept(*this);
}

void instantiation_collector_c::visit(const match_c &node) {
  if (node.scrutinee())
    node.scrutinee()->accept(*this);
  for (const auto &case_ : node.cases()) {
    if (case_.body)
      case_.body->accept(*this);
  }
}

void instantiation_collector_c::visit(const binary_op_c &node) {
  if (node.left())
    node.left()->accept(*this);
  if (node.right())
    node.right()->accept(*this);
}

void instantiation_collector_c::visit(const unary_op_c &node) {
  if (node.operand())
    node.operand()->accept(*this);
}

void instantiation_collector_c::visit(const cast_c &node) {
  if (node.expression())
    node.expression()->accept(*this);
  if (node.target_type())
    node.target_type()->accept(*this);
}

void instantiation_collector_c::visit(const call_c &node) {
  if (node.callee())
    node.callee()->accept(*this);
  for (const auto &arg : node.arguments()) {
    if (arg)
      arg->accept(*this);
  }
}

void instantiation_collector_c::visit(const index_c &node) {
  if (node.object())
    node.object()->accept(*this);
  if (node.index())
    node.index()->accept(*this);
}

void instantiation_collector_c::visit(const member_access_c &node) {
  if (node.object())
    node.object()->accept(*this);
}

void instantiation_collector_c::visit(const literal_c &node) {}

void instantiation_collector_c::visit(const identifier_c &node) {}

void instantiation_collector_c::visit(const assignment_c &node) {
  if (node.target())
    node.target()->accept(*this);
  if (node.value())
    node.value()->accept(*this);
}

void instantiation_collector_c::visit(const block_c &node) {
  for (const auto &stmt : node.statements()) {
    if (stmt)
      stmt->accept(*this);
  }
}

void instantiation_collector_c::visit(const array_literal_c &node) {
  for (const auto &elem : node.elements()) {
    if (elem)
      elem->accept(*this);
  }
}

void instantiation_collector_c::visit(const struct_literal_c &node) {
  for (const auto &field : node.field_initializers()) {
    if (field.value)
      field.value->accept(*this);
  }
}

void instantiation_collector_c::visit(const type_param_c &node) {
  if (node.type())
    node.type()->accept(*this);
}

void instantiation_collector_c::visit(const import_c &node) {}

void instantiation_collector_c::visit(const cimport_c &node) {}

void instantiation_collector_c::visit(const shard_c &node) {}

void instantiation_collector_c::visit(const enum_value_access_c &node) {}

} // namespace truk::emitc
