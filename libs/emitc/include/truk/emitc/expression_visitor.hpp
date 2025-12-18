#pragma once

#include <language/visitor.hpp>
#include <string>

namespace truk::emitc {

class emitter_c;

class expression_visitor_c : public truk::language::nodes::visitor_if {
public:
  explicit expression_visitor_c(emitter_c &emitter);
  ~expression_visitor_c() override = default;

  std::string get_result() const { return _result; }

  void visit(const truk::language::nodes::binary_op_c &node) override;
  void visit(const truk::language::nodes::unary_op_c &node) override;
  void visit(const truk::language::nodes::cast_c &node) override;
  void visit(const truk::language::nodes::call_c &node) override;
  void visit(const truk::language::nodes::index_c &node) override;
  void visit(const truk::language::nodes::member_access_c &node) override;
  void visit(const truk::language::nodes::literal_c &node) override;
  void visit(const truk::language::nodes::identifier_c &node) override;
  void visit(const truk::language::nodes::array_literal_c &node) override;
  void visit(const truk::language::nodes::struct_literal_c &node) override;
  void visit(const truk::language::nodes::assignment_c &node) override;
  void visit(const truk::language::nodes::lambda_c &node) override;

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::map_type_c &node) override;
  void visit(const truk::language::nodes::tuple_type_c &node) override;
  void visit(const truk::language::nodes::fn_c &node) override;
  void visit(const truk::language::nodes::struct_c &node) override;
  void visit(const truk::language::nodes::enum_c &node) override;
  void visit(const truk::language::nodes::var_c &node) override;
  void visit(const truk::language::nodes::const_c &node) override;
  void visit(const truk::language::nodes::let_c &node) override;
  void visit(const truk::language::nodes::if_c &node) override;
  void visit(const truk::language::nodes::while_c &node) override;
  void visit(const truk::language::nodes::for_c &node) override;
  void visit(const truk::language::nodes::return_c &node) override;
  void visit(const truk::language::nodes::break_c &node) override;
  void visit(const truk::language::nodes::continue_c &node) override;
  void visit(const truk::language::nodes::defer_c &node) override;
  void visit(const truk::language::nodes::match_c &node) override;
  void visit(const truk::language::nodes::block_c &node) override;
  void visit(const truk::language::nodes::type_param_c &node) override;
  void visit(const truk::language::nodes::import_c &node) override;
  void visit(const truk::language::nodes::cimport_c &node) override;
  void visit(const truk::language::nodes::shard_c &node) override;
  void visit(const truk::language::nodes::enum_value_access_c &node) override;

private:
  emitter_c &_emitter;
  std::string _result;
};

} // namespace truk::emitc
