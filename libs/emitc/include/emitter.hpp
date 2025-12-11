#pragma once

#include <language/node.hpp>
#include <language/visitor.hpp>

#include <memory>
#include <string>
#include <vector>

namespace truk::emitc {

struct error_s {
  std::string message;
  const truk::language::nodes::base_c *node;
  std::size_t source_index;

  error_s(std::string msg, const truk::language::nodes::base_c *n,
          std::size_t idx)
      : message(std::move(msg)), node(n), source_index(idx) {}
};

struct result_c {
  std::vector<error_s> errors;
  std::vector<std::string> chunks;

  bool has_errors() const { return !errors.empty(); }
};

class emitter_c : public truk::language::nodes::visitor_if {
public:
  emitter_c();
  ~emitter_c() override = default;

  void emit(const truk::language::nodes::base_c *root);

  result_c result() const { return _result; }

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::fn_c &node) override;
  void visit(const truk::language::nodes::struct_c &node) override;
  void visit(const truk::language::nodes::var_c &node) override;
  void visit(const truk::language::nodes::const_c &node) override;
  void visit(const truk::language::nodes::if_c &node) override;
  void visit(const truk::language::nodes::while_c &node) override;
  void visit(const truk::language::nodes::for_c &node) override;
  void visit(const truk::language::nodes::return_c &node) override;
  void visit(const truk::language::nodes::break_c &node) override;
  void visit(const truk::language::nodes::continue_c &node) override;
  void visit(const truk::language::nodes::binary_op_c &node) override;
  void visit(const truk::language::nodes::unary_op_c &node) override;
  void visit(const truk::language::nodes::call_c &node) override;
  void visit(const truk::language::nodes::index_c &node) override;
  void visit(const truk::language::nodes::member_access_c &node) override;
  void visit(const truk::language::nodes::literal_c &node) override;
  void visit(const truk::language::nodes::identifier_c &node) override;
  void visit(const truk::language::nodes::assignment_c &node) override;
  void visit(const truk::language::nodes::block_c &node) override;
  void visit(const truk::language::nodes::array_literal_c &node) override;
  void visit(const truk::language::nodes::struct_literal_c &node) override;
  void visit(const truk::language::nodes::type_param_c &node) override;

private:
  result_c _result;
};

} // namespace truk::emitc
