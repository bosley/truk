#include <sstream>
#include <truk/emitc/emitter.hpp>
#include <truk/emitc/expression_visitor.hpp>

namespace truk::emitc {

using namespace truk::language::nodes;

expression_visitor_c::expression_visitor_c(emitter_c &emitter)
    : _emitter(emitter), _result("") {}

void expression_visitor_c::visit(const binary_op_c &node) {
  _result = _emitter.emit_expr_binary_op(node);
}

void expression_visitor_c::visit(const unary_op_c &node) {
  _result = _emitter.emit_expr_unary_op(node);
}

void expression_visitor_c::visit(const cast_c &node) {
  _result = _emitter.emit_expr_cast(node);
}

void expression_visitor_c::visit(const call_c &node) {
  _result = _emitter.emit_expr_call(node);
}

void expression_visitor_c::visit(const index_c &node) {
  _result = _emitter.emit_expr_index(node);
}

void expression_visitor_c::visit(const member_access_c &node) {
  _result = _emitter.emit_expr_member_access(node);
}

void expression_visitor_c::visit(const literal_c &node) {
  _result = _emitter.emit_expr_literal(node);
}

void expression_visitor_c::visit(const identifier_c &node) {
  _result = _emitter.emit_expr_identifier(node);
}

void expression_visitor_c::visit(const array_literal_c &node) {
  _result = _emitter.emit_expr_array_literal(node);
}

void expression_visitor_c::visit(const struct_literal_c &node) {
  _result = _emitter.emit_expr_struct_literal(node);
}

void expression_visitor_c::visit(const assignment_c &node) {
  std::string target = _emitter.emit_expression(node.target());
  std::string value = _emitter.emit_expression(node.value());
  _result = target + " = " + value;
}

void expression_visitor_c::visit(const lambda_c &node) {
  std::stringstream temp_expr;
  std::swap(temp_expr, _emitter._current_expr);
  node.accept(_emitter);
  _result = _emitter._current_expr.str();
  std::swap(temp_expr, _emitter._current_expr);
}

void expression_visitor_c::visit(const primitive_type_c &node) {}

void expression_visitor_c::visit(const named_type_c &node) {}

void expression_visitor_c::visit(const pointer_type_c &node) {}

void expression_visitor_c::visit(const array_type_c &node) {}

void expression_visitor_c::visit(const function_type_c &node) {}

void expression_visitor_c::visit(const map_type_c &node) {}

void expression_visitor_c::visit(const tuple_type_c &node) {}

void expression_visitor_c::visit(const fn_c &node) {}

void expression_visitor_c::visit(const struct_c &node) {}

void expression_visitor_c::visit(const enum_c &node) {}

void expression_visitor_c::visit(const var_c &node) {}

void expression_visitor_c::visit(const const_c &node) {}

void expression_visitor_c::visit(const let_c &node) {}

void expression_visitor_c::visit(const if_c &node) {}

void expression_visitor_c::visit(const while_c &node) {}

void expression_visitor_c::visit(const for_c &node) {}

void expression_visitor_c::visit(const return_c &node) {}

void expression_visitor_c::visit(const break_c &node) {}

void expression_visitor_c::visit(const continue_c &node) {}

void expression_visitor_c::visit(const defer_c &node) {}

void expression_visitor_c::visit(const match_c &node) {}

void expression_visitor_c::visit(const block_c &node) {}

void expression_visitor_c::visit(const type_param_c &node) {}

void expression_visitor_c::visit(const import_c &node) {}

void expression_visitor_c::visit(const cimport_c &node) {}

void expression_visitor_c::visit(const shard_c &node) {}

void expression_visitor_c::visit(const enum_value_access_c &node) {}

} // namespace truk::emitc
