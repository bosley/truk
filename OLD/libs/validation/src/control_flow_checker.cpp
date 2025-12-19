#include <truk/validation/control_flow_checker.hpp>

namespace truk::validation {

using namespace truk::language::nodes;

void control_flow_checker_c::check_node(const base_c *node) {
  if (node && !_has_control_flow) {
    node->accept(*this);
  }
}

void control_flow_checker_c::visit(const primitive_type_c &) {}

void control_flow_checker_c::visit(const named_type_c &) {}

void control_flow_checker_c::visit(const pointer_type_c &) {}

void control_flow_checker_c::visit(const array_type_c &) {}

void control_flow_checker_c::visit(const function_type_c &) {}

void control_flow_checker_c::visit(const map_type_c &) {}

void control_flow_checker_c::visit(const tuple_type_c &) {}

void control_flow_checker_c::visit(const fn_c &) {}

void control_flow_checker_c::visit(const lambda_c &) {}

void control_flow_checker_c::visit(const struct_c &) {}

void control_flow_checker_c::visit(const enum_c &) {}

void control_flow_checker_c::visit(const var_c &node) {
  check_node(node.initializer());
}

void control_flow_checker_c::visit(const const_c &node) {
  check_node(node.value());
}

void control_flow_checker_c::visit(const let_c &node) {
  if (node.initializer()) {
    check_node(node.initializer());
  }
}

void control_flow_checker_c::visit(const if_c &node) {
  check_node(node.condition());
  if (_has_control_flow)
    return;

  check_node(node.then_block());
  if (_has_control_flow)
    return;

  check_node(node.else_block());
}

void control_flow_checker_c::visit(const while_c &node) {
  check_node(node.condition());
  if (_has_control_flow)
    return;

  bool prev_in_loop = _in_loop;
  _in_loop = true;
  check_node(node.body());
  _in_loop = prev_in_loop;
}

void control_flow_checker_c::visit(const for_c &node) {
  check_node(node.init());
  if (_has_control_flow)
    return;

  check_node(node.condition());
  if (_has_control_flow)
    return;

  check_node(node.post());
  if (_has_control_flow)
    return;

  bool prev_in_loop = _in_loop;
  _in_loop = true;
  check_node(node.body());
  _in_loop = prev_in_loop;
}

void control_flow_checker_c::visit(const return_c &) {
  _has_control_flow = true;
}

void control_flow_checker_c::visit(const break_c &) {
  _has_control_flow = true;
  if (!_in_loop) {
    _has_break_or_continue = true;
  }
}

void control_flow_checker_c::visit(const continue_c &) {
  _has_control_flow = true;
  if (!_in_loop) {
    _has_break_or_continue = true;
  }
}

void control_flow_checker_c::visit(const defer_c &node) {
  check_node(node.deferred_code());
}

void control_flow_checker_c::visit(const match_c &node) {
  check_node(node.scrutinee());
  if (_has_control_flow)
    return;

  for (const auto &case_arm : node.cases()) {
    if (case_arm.pattern) {
      check_node(case_arm.pattern.get());
      if (_has_control_flow)
        return;
    }
    if (case_arm.body) {
      check_node(case_arm.body.get());
      if (_has_control_flow)
        return;
    }
  }
}

void control_flow_checker_c::visit(const binary_op_c &node) {
  check_node(node.left());
  if (_has_control_flow)
    return;

  check_node(node.right());
}

void control_flow_checker_c::visit(const unary_op_c &node) {
  check_node(node.operand());
}

void control_flow_checker_c::visit(const cast_c &node) {
  check_node(node.expression());
}

void control_flow_checker_c::visit(const call_c &node) {
  check_node(node.callee());
  if (_has_control_flow)
    return;

  for (const auto &arg : node.arguments()) {
    check_node(arg.get());
    if (_has_control_flow)
      return;
  }
}

void control_flow_checker_c::visit(const index_c &node) {
  check_node(node.object());
  if (_has_control_flow)
    return;

  check_node(node.index());
}

void control_flow_checker_c::visit(const member_access_c &node) {
  check_node(node.object());
}

void control_flow_checker_c::visit(const literal_c &) {}

void control_flow_checker_c::visit(const identifier_c &) {}

void control_flow_checker_c::visit(const assignment_c &node) {
  check_node(node.target());
  if (_has_control_flow)
    return;

  check_node(node.value());
}

void control_flow_checker_c::visit(const block_c &node) {
  for (const auto &stmt : node.statements()) {
    check_node(stmt.get());
    if (_has_control_flow)
      return;
  }
}

void control_flow_checker_c::visit(const array_literal_c &node) {
  for (const auto &elem : node.elements()) {
    check_node(elem.get());
    if (_has_control_flow)
      return;
  }
}

void control_flow_checker_c::visit(const struct_literal_c &node) {
  for (const auto &field_init : node.field_initializers()) {
    check_node(field_init.value.get());
    if (_has_control_flow)
      return;
  }
}

void control_flow_checker_c::visit(const type_param_c &) {}

void control_flow_checker_c::visit(const import_c &) {}

void control_flow_checker_c::visit(const cimport_c &) {}

void control_flow_checker_c::visit(const shard_c &) {}

void control_flow_checker_c::visit(const enum_value_access_c &) {}

} // namespace truk::validation
