#pragma once

#include <language/builtins.hpp>
#include <language/keywords.hpp>
#include <language/node.hpp>
#include <language/visitor.hpp>
#include <truk/core/memory.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace truk::validation {

enum class type_kind_e {
  PRIMITIVE,
  STRUCT,
  FUNCTION,
  POINTER,
  ARRAY,
  VOID_TYPE,
  MAP,
  UNTYPED_INTEGER,
  UNTYPED_FLOAT
};

struct type_entry_s : public truk::core::memory_c<2048>::storeable_if {
  type_kind_e kind;
  std::string name;
  std::size_t pointer_depth{0};
  std::optional<std::size_t> array_size;

  std::vector<std::string> struct_field_names;
  std::unordered_map<std::string, std::unique_ptr<type_entry_s>> struct_fields;

  std::vector<std::unique_ptr<type_entry_s>> function_param_types;
  std::unique_ptr<type_entry_s> function_return_type;
  bool is_variadic{false};

  std::unique_ptr<type_entry_s> pointee_type;
  std::unique_ptr<type_entry_s> element_type;
  std::unique_ptr<type_entry_s> map_value_type;

  bool is_builtin{false};
  std::optional<truk::language::builtins::builtin_kind_e> builtin_kind;

  type_entry_s(type_kind_e k, std::string n) : kind(k), name(std::move(n)) {}

  type_entry_s(const type_entry_s &other)
      : kind(other.kind), name(other.name), pointer_depth(other.pointer_depth),
        array_size(other.array_size),
        struct_field_names(other.struct_field_names),
        is_variadic(other.is_variadic), is_builtin(other.is_builtin),
        builtin_kind(other.builtin_kind) {

    for (const auto &[field_name, field_type] : other.struct_fields) {
      struct_fields[field_name] = std::make_unique<type_entry_s>(*field_type);
    }

    for (const auto &param_type : other.function_param_types) {
      function_param_types.push_back(
          std::make_unique<type_entry_s>(*param_type));
    }

    if (other.function_return_type) {
      function_return_type =
          std::make_unique<type_entry_s>(*other.function_return_type);
    }

    if (other.pointee_type) {
      pointee_type = std::make_unique<type_entry_s>(*other.pointee_type);
    }

    if (other.element_type) {
      element_type = std::make_unique<type_entry_s>(*other.element_type);
    }

    if (other.map_value_type) {
      map_value_type = std::make_unique<type_entry_s>(*other.map_value_type);
    }
  }

  storeable_if *clone() override { return new type_entry_s(*this); }

  ~type_entry_s() override = default;
};

struct symbol_entry_s : public truk::core::memory_c<2048>::storeable_if {
  std::string name;
  std::unique_ptr<type_entry_s> type;
  bool is_mutable;
  std::size_t declaration_index;

  symbol_entry_s(std::string n, std::unique_ptr<type_entry_s> t,
                 bool mutable_flag, std::size_t decl_idx)
      : name(std::move(n)), type(std::move(t)), is_mutable(mutable_flag),
        declaration_index(decl_idx) {}

  symbol_entry_s(const symbol_entry_s &other)
      : name(other.name), is_mutable(other.is_mutable),
        declaration_index(other.declaration_index) {
    if (other.type) {
      type = std::make_unique<type_entry_s>(*other.type);
    }
  }

  storeable_if *clone() override { return new symbol_entry_s(*this); }

  ~symbol_entry_s() override = default;
};

struct type_error_s {
  std::string message;
  std::size_t source_index;

  type_error_s(std::string msg, std::size_t idx)
      : message(std::move(msg)), source_index(idx) {}
};

class type_checker_c : public truk::language::nodes::visitor_if {
public:
  type_checker_c();
  ~type_checker_c() override = default;

  void check(const truk::language::nodes::base_c *root);

  const std::vector<std::string> &errors() const { return _errors; }
  const std::vector<type_error_s> &detailed_errors() const {
    return _detailed_errors;
  }
  bool has_errors() const { return !_errors.empty(); }

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::map_type_c &node) override;
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
  void visit(const truk::language::nodes::defer_c &node) override;
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
  void visit(const truk::language::nodes::import_c &node) override;
  void visit(const truk::language::nodes::cimport_c &node) override;

private:
  truk::core::memory_c<2048> _memory;
  std::vector<std::string> _errors;
  std::vector<type_error_s> _detailed_errors;
  std::unique_ptr<type_entry_s> _current_expression_type;
  std::unique_ptr<type_entry_s> _current_function_return_type;
  bool _in_loop{false};

  void push_scope();
  void pop_scope();

  void register_builtin_types();
  void register_builtin_functions();
  void register_type(const std::string &name,
                     std::unique_ptr<type_entry_s> type);
  void register_symbol(const std::string &name,
                       std::unique_ptr<type_entry_s> type, bool is_mutable,
                       std::size_t source_index);

  std::unique_ptr<type_entry_s>
  resolve_type(const truk::language::nodes::type_c *type_node);
  std::string
  get_type_name_for_error(const truk::language::nodes::type_c *type_node);
  std::string get_type_name_from_entry(const type_entry_s *type);
  type_entry_s *lookup_type(const std::string &name);
  symbol_entry_s *lookup_symbol(const std::string &name);

  bool types_equal(const type_entry_s *a, const type_entry_s *b);
  bool is_numeric_type(const type_entry_s *type);
  bool is_integer_type(const type_entry_s *type);
  bool is_boolean_type(const type_entry_s *type);
  bool is_comparable_type(const type_entry_s *type);
  bool is_compatible_for_assignment(const type_entry_s *target,
                                    const type_entry_s *source);
  bool is_type_identifier(const truk::language::nodes::identifier_c *id_node);

  void validate_builtin_call(const truk::language::nodes::call_c &node,
                             const type_entry_s &func_type);

  bool check_no_control_flow(const truk::language::nodes::base_c *node);

  void report_error(const std::string &message, std::size_t source_index);

  std::unique_ptr<type_entry_s>
  resolve_untyped_literal(const type_entry_s *literal_type,
                          const type_entry_s *target_type);
};

} // namespace truk::validation
