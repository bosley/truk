#pragma once

#include <language/node.hpp>
#include <language/visitor.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

  void collect_declarations(const truk::language::nodes::base_c *root);
  void emit_forward_declarations();
  void emit(const truk::language::nodes::base_c *root);

  void finalize();

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
  void visit(const truk::language::nodes::cast_c &node) override;
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
  std::string emit_type(const truk::language::nodes::type_c *type);
  std::string emit_type_for_sizeof(const truk::language::nodes::type_c *type);
  std::string
  emit_array_pointer_type(const truk::language::nodes::type_c *array_type,
                          const std::string &identifier = "");
  std::string
  get_slice_type_name(const truk::language::nodes::type_c *element_type);
  void ensure_slice_typedef(const truk::language::nodes::type_c *element_type);
  bool is_slice_type(const truk::language::nodes::type_c *type);
  void register_variable_type(const std::string &name,
                              const truk::language::nodes::type_c *type);
  bool is_variable_slice(const std::string &name);

  result_c _result;
  std::stringstream _current_expr;
  std::stringstream _header;
  std::stringstream _forward_decls;
  std::stringstream _structs;
  std::stringstream _functions;
  int _indent_level{0};
  std::unordered_set<std::string> _slice_types_emitted;
  std::unordered_set<std::string> _struct_names;
  std::unordered_set<std::string> _function_names;
  std::unordered_map<std::string, bool> _variable_is_slice;
  bool _in_expression{false};
  bool _collecting_declarations{false};
  std::string _current_function_name;
  const truk::language::nodes::type_c *_current_function_return_type{nullptr};
};

} // namespace truk::emitc
