#pragma once

#include <language/node.hpp>
#include <language/visitor.hpp>

namespace truk::validation {

class control_flow_checker_c : public language::nodes::visitor_if {
public:
  control_flow_checker_c() = default;

  bool has_control_flow() const { return _has_control_flow; }
  bool has_break_or_continue() const { return _has_break_or_continue; }

  void set_in_loop(bool in_loop) { _in_loop = in_loop; }

  void visit(const language::nodes::primitive_type_c &node) override;
  void visit(const language::nodes::named_type_c &node) override;
  void visit(const language::nodes::pointer_type_c &node) override;
  void visit(const language::nodes::array_type_c &node) override;
  void visit(const language::nodes::function_type_c &node) override;
  void visit(const language::nodes::map_type_c &node) override;
  void visit(const language::nodes::tuple_type_c &node) override;
  void visit(const language::nodes::fn_c &node) override;
  void visit(const language::nodes::lambda_c &node) override;
  void visit(const language::nodes::struct_c &node) override;
  void visit(const language::nodes::enum_c &node) override;
  void visit(const language::nodes::var_c &node) override;
  void visit(const language::nodes::const_c &node) override;
  void visit(const language::nodes::let_c &node) override;
  void visit(const language::nodes::if_c &node) override;
  void visit(const language::nodes::while_c &node) override;
  void visit(const language::nodes::for_c &node) override;
  void visit(const language::nodes::return_c &node) override;
  void visit(const language::nodes::break_c &node) override;
  void visit(const language::nodes::continue_c &node) override;
  void visit(const language::nodes::defer_c &node) override;
  void visit(const language::nodes::binary_op_c &node) override;
  void visit(const language::nodes::unary_op_c &node) override;
  void visit(const language::nodes::cast_c &node) override;
  void visit(const language::nodes::call_c &node) override;
  void visit(const language::nodes::index_c &node) override;
  void visit(const language::nodes::member_access_c &node) override;
  void visit(const language::nodes::literal_c &node) override;
  void visit(const language::nodes::identifier_c &node) override;
  void visit(const language::nodes::assignment_c &node) override;
  void visit(const language::nodes::block_c &node) override;
  void visit(const language::nodes::array_literal_c &node) override;
  void visit(const language::nodes::struct_literal_c &node) override;
  void visit(const language::nodes::type_param_c &node) override;
  void visit(const language::nodes::import_c &node) override;
  void visit(const language::nodes::cimport_c &node) override;
  void visit(const language::nodes::shard_c &node) override;
  void visit(const language::nodes::enum_value_access_c &node) override;

private:
  bool _has_control_flow{false};
  bool _has_break_or_continue{false};
  bool _in_loop{false};

  void check_node(const language::nodes::base_c *node);
};

} // namespace truk::validation
