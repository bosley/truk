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
#include <unordered_set>
#include <vector>

namespace truk::validation {

enum class type_kind_e {
  PRIMITIVE,
  STRUCT,
  ENUM,
  FUNCTION,
  POINTER,
  ARRAY,
  VOID_TYPE,
  MAP,
  TUPLE,
  UNTYPED_INTEGER,
  UNTYPED_FLOAT
};

enum class validator_stage_e {
  SYMBOL_COLLECTION,
  TYPE_RESOLUTION,
  CONTROL_FLOW_ANALYSIS,
  LAMBDA_CAPTURE_VALIDATION,
  TYPE_CHECKING,
  FINAL_VALIDATION
};

enum class symbol_scope_e { GLOBAL, FUNCTION_LOCAL, LAMBDA_LOCAL, PARAMETER };

enum class scope_kind_e { GLOBAL, FUNCTION, LAMBDA, BLOCK };

struct type_entry_s : public truk::core::memory_c<2048>::storeable_if {
  type_kind_e kind;
  std::string name;
  std::size_t pointer_depth{0};
  std::optional<std::size_t> array_size;

  std::vector<std::string> struct_field_names;
  std::unordered_map<std::string, std::unique_ptr<type_entry_s>> struct_fields;

  std::unique_ptr<type_entry_s> enum_backing_type;
  std::unordered_map<std::string, std::int64_t> enum_values;

  std::vector<std::unique_ptr<type_entry_s>> function_param_types;
  std::unique_ptr<type_entry_s> function_return_type;
  bool is_variadic{false};

  std::unique_ptr<type_entry_s> pointee_type;
  std::unique_ptr<type_entry_s> element_type;
  std::unique_ptr<type_entry_s> map_key_type;
  std::unique_ptr<type_entry_s> map_value_type;

  std::vector<std::unique_ptr<type_entry_s>> tuple_element_types;

  bool is_builtin{false};
  std::optional<truk::language::builtins::builtin_kind_e> builtin_kind;

  type_entry_s(type_kind_e k, std::string n) : kind(k), name(std::move(n)) {}

  type_entry_s(const type_entry_s &other)
      : kind(other.kind), name(other.name), pointer_depth(other.pointer_depth),
        array_size(other.array_size),
        struct_field_names(other.struct_field_names),
        enum_values(other.enum_values), is_variadic(other.is_variadic),
        is_builtin(other.is_builtin), builtin_kind(other.builtin_kind) {

    for (const auto &[field_name, field_type] : other.struct_fields) {
      struct_fields[field_name] = std::make_unique<type_entry_s>(*field_type);
    }

    if (other.enum_backing_type) {
      enum_backing_type =
          std::make_unique<type_entry_s>(*other.enum_backing_type);
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

    if (other.map_key_type) {
      map_key_type = std::make_unique<type_entry_s>(*other.map_key_type);
    }

    if (other.map_value_type) {
      map_value_type = std::make_unique<type_entry_s>(*other.map_value_type);
    }

    for (const auto &tuple_elem : other.tuple_element_types) {
      tuple_element_types.push_back(
          std::make_unique<type_entry_s>(*tuple_elem));
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
  symbol_scope_e scope_kind{symbol_scope_e::FUNCTION_LOCAL};
  const truk::language::nodes::base_c *declaring_node{nullptr};

  symbol_entry_s(std::string n, std::unique_ptr<type_entry_s> t,
                 bool mutable_flag, std::size_t decl_idx)
      : name(std::move(n)), type(std::move(t)), is_mutable(mutable_flag),
        declaration_index(decl_idx) {}

  symbol_entry_s(const symbol_entry_s &other)
      : name(other.name), is_mutable(other.is_mutable),
        declaration_index(other.declaration_index),
        scope_kind(other.scope_kind), declaring_node(other.declaring_node) {
    if (other.type) {
      type = std::make_unique<type_entry_s>(*other.type);
    }
  }

  storeable_if *clone() override { return new symbol_entry_s(*this); }

  ~symbol_entry_s() override = default;
};

struct type_error_s {
  std::string message;
  std::string file_path;
  std::size_t source_index;

  type_error_s(std::string msg, std::string file, std::size_t idx)
      : message(std::move(msg)), file_path(std::move(file)), source_index(idx) {
  }
};

struct scope_info_s {
  scope_kind_e kind;
  const truk::language::nodes::base_c *owner_node;
  scope_info_s *parent;
  std::unordered_map<std::string, symbol_entry_s *> symbols;
  std::vector<std::unique_ptr<scope_info_s>> children;

  scope_info_s(scope_kind_e k, const truk::language::nodes::base_c *owner,
               scope_info_s *p)
      : kind(k), owner_node(owner), parent(p) {}
};

struct symbol_collection_result_s {
  std::unordered_map<std::string, symbol_entry_s *> global_symbols;
  std::unique_ptr<scope_info_s> global_scope;
  std::unordered_map<const truk::language::nodes::base_c *, scope_info_s *>
      scope_map;
  std::vector<const truk::language::nodes::lambda_c *> lambdas;
  std::vector<type_error_s> errors;
};

struct type_resolution_result_s {
  std::unordered_map<const truk::language::nodes::base_c *, type_entry_s *>
      node_types;
  std::vector<type_error_s> errors;
};

struct control_flow_result_s {
  std::unordered_set<const truk::language::nodes::base_c *>
      nodes_with_control_flow;
  std::vector<type_error_s> errors;
};

struct lambda_capture_result_s {
  std::unordered_map<const truk::language::nodes::lambda_c *,
                     std::vector<std::string>>
      captured_vars;
  std::vector<type_error_s> errors;
};

class symbol_collector_c;
class lambda_capture_validator_c;

class type_checker_c : public truk::language::nodes::visitor_if {
public:
  type_checker_c();
  ~type_checker_c() override = default;

  void check(const truk::language::nodes::base_c *root);

  void set_declaration_file_map(
      const std::unordered_map<const truk::language::nodes::base_c *,
                               std::string> &map) {
    _decl_to_file = map;
  }

  void set_file_to_shards_map(
      const std::unordered_map<std::string, std::vector<std::string>> &map) {
    _file_to_shards = map;
  }

  const std::vector<type_error_s> &errors() const { return _detailed_errors; }
  bool has_errors() const { return !_detailed_errors.empty(); }

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::map_type_c &node) override;
  void visit(const truk::language::nodes::tuple_type_c &node) override;
  void visit(const truk::language::nodes::fn_c &node) override;
  void visit(const truk::language::nodes::lambda_c &node) override;
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
  void visit(const truk::language::nodes::shard_c &node) override;
  void visit(const truk::language::nodes::enum_value_access_c &node) override;

private:
  truk::core::memory_c<2048> _memory;
  std::vector<type_error_s> _detailed_errors;
  std::unique_ptr<type_entry_s> _current_expression_type;
  std::unique_ptr<type_entry_s> _current_function_return_type;
  bool _in_loop{false};

  std::unordered_map<const truk::language::nodes::base_c *, std::string>
      _decl_to_file;
  std::unordered_map<std::string, std::string> _struct_to_file;
  std::unordered_map<std::string, std::string> _function_to_file;
  std::unordered_map<std::string, std::string> _global_to_file;
  std::unordered_map<std::string, std::vector<std::string>> _file_to_shards;
  std::string _current_file;

  symbol_collection_result_s
  collect_symbols(const truk::language::nodes::base_c *root);
  type_resolution_result_s
  resolve_types(const truk::language::nodes::base_c *root,
                const symbol_collection_result_s &symbols);
  control_flow_result_s
  analyze_control_flow(const truk::language::nodes::base_c *root);
  lambda_capture_result_s
  validate_lambda_captures(const truk::language::nodes::base_c *root,
                           const symbol_collection_result_s &symbols);
  void perform_type_checking(const truk::language::nodes::base_c *root,
                             const symbol_collection_result_s &symbols,
                             const type_resolution_result_s &types);
  void final_validation(const symbol_collection_result_s &symbols,
                        const type_resolution_result_s &types,
                        const control_flow_result_s &control_flow,
                        const lambda_capture_result_s &lambda_captures);

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
  bool is_valid_map_key_type(const type_entry_s *type);
  bool is_compatible_for_assignment(const type_entry_s *target,
                                    const type_entry_s *source);
  bool is_type_identifier(const truk::language::nodes::identifier_c *id_node);

  void validate_builtin_call(const truk::language::nodes::call_c &node,
                             const type_entry_s &func_type);

  bool check_no_control_flow(const truk::language::nodes::base_c *node);
  bool check_no_break_or_continue(const truk::language::nodes::base_c *node);

  void report_error(const std::string &message, std::size_t source_index);

  std::unique_ptr<type_entry_s>
  resolve_untyped_literal(const type_entry_s *literal_type,
                          const type_entry_s *target_type);

  truk::language::nodes::type_ptr
  create_type_node_from_entry(const type_entry_s *entry);

  bool is_private_identifier(const std::string &name) const;
  std::string
  get_defining_file_for_struct(const std::string &struct_name) const;
  std::string
  get_defining_file_for_function(const std::string &func_name) const;
  std::string
  get_defining_file_for_global(const std::string &global_name) const;
  bool files_share_shard(const std::string &file1,
                         const std::string &file2) const;
};

class symbol_collector_c : public truk::language::nodes::visitor_if {
public:
  symbol_collector_c(
      truk::core::memory_c<2048> &memory,
      const std::unordered_map<const truk::language::nodes::base_c *,
                               std::string> &decl_to_file);

  symbol_collection_result_s collect(const truk::language::nodes::base_c *root);

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::map_type_c &node) override;
  void visit(const truk::language::nodes::tuple_type_c &node) override;
  void visit(const truk::language::nodes::fn_c &node) override;
  void visit(const truk::language::nodes::lambda_c &node) override;
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
  void visit(const truk::language::nodes::shard_c &node) override;
  void visit(const truk::language::nodes::enum_value_access_c &node) override;

private:
  truk::core::memory_c<2048> &_memory;
  const std::unordered_map<const truk::language::nodes::base_c *, std::string>
      &_decl_to_file;
  symbol_collection_result_s _result;
  scope_info_s *_current_scope{nullptr};
  std::string _current_file;
};

class lambda_capture_validator_c : public truk::language::nodes::visitor_if {
public:
  lambda_capture_validator_c(
      const symbol_collection_result_s &symbols,
      const std::unordered_map<const truk::language::nodes::base_c *,
                               std::string> &decl_to_file);

  lambda_capture_result_s validate(const truk::language::nodes::base_c *root);

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::map_type_c &node) override;
  void visit(const truk::language::nodes::tuple_type_c &node) override;
  void visit(const truk::language::nodes::fn_c &node) override;
  void visit(const truk::language::nodes::lambda_c &node) override;
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
  void visit(const truk::language::nodes::shard_c &node) override;
  void visit(const truk::language::nodes::enum_value_access_c &node) override;

private:
  const symbol_collection_result_s &_symbols;
  const std::unordered_map<const truk::language::nodes::base_c *, std::string>
      &_decl_to_file;
  lambda_capture_result_s _result;
  scope_info_s *_current_scope{nullptr};
  const truk::language::nodes::lambda_c *_current_lambda{nullptr};
  std::string _current_file;
};

} // namespace truk::validation
