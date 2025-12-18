#pragma once

#include <language/node.hpp>
#include <language/visitor.hpp>
#include <truk/core/exceptions.hpp>
#include <truk/emitc/builtin_handler.hpp>
#include <truk/emitc/expression_visitor.hpp>
#include <truk/emitc/type_registry.hpp>
#include <truk/emitc/variable_registry.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace truk::emitc {

class emitter_exception_c : public truk::core::truk_exception_c {
public:
  emitter_exception_c(const std::string &message)
      : truk_exception_c("emitter", message) {}
  emitter_exception_c(int error_code, const std::string &message)
      : truk_exception_c("emitter", error_code, message) {}
};

enum class emission_phase_e {
  COLLECTION,
  FORWARD_DECLARATION,
  STRUCT_DEFINITION,
  FUNCTION_DEFINITION,
  EXPRESSION_GENERATION,
  FINALIZATION
};

const char *emission_phase_name(emission_phase_e phase);

struct defer_scope_s {
  enum class scope_type_e { FUNCTION, LAMBDA, BLOCK, LOOP };

  std::vector<const truk::language::nodes::defer_c *> defers;
  scope_type_e type;
  const truk::language::nodes::base_c *owner_node;
  defer_scope_s *parent;

  defer_scope_s(scope_type_e t, const truk::language::nodes::base_c *owner,
                defer_scope_s *p)
      : type(t), owner_node(owner), parent(p) {}
};

struct error_s {
  std::string message;
  const truk::language::nodes::base_c *node;
  std::size_t source_index;
  emission_phase_e phase;
  std::string node_context;

  error_s(std::string msg, const truk::language::nodes::base_c *n,
          std::size_t idx, emission_phase_e p, std::string ctx)
      : message(std::move(msg)), node(n), source_index(idx), phase(p),
        node_context(std::move(ctx)) {}
};

struct compilation_unit_metadata_s {
  std::unordered_set<std::string> defined_functions;
  std::unordered_set<std::string> defined_structs;
  std::unordered_set<std::string> extern_structs;
  std::vector<std::string> test_functions;
  bool has_test_setup{false};
  bool has_test_teardown{false};
  bool has_main_function{false};
  int main_function_count{0};

  bool is_library() const { return !has_main_function; }
  bool has_multiple_mains() const { return main_function_count > 1; }
  bool has_tests() const { return !test_functions.empty(); }
};

enum class assembly_type_e { APPLICATION, LIBRARY };

struct assembly_result_s {
  assembly_type_e type;
  std::string source;
  std::string header;
  std::string header_name;

  assembly_result_s(assembly_type_e t, std::string src)
      : type(t), source(std::move(src)) {}
  assembly_result_s(assembly_type_e t, std::string src, std::string hdr,
                    std::string hdr_name = "")
      : type(t), source(std::move(src)), header(std::move(hdr)),
        header_name(std::move(hdr_name)) {}
};

struct result_c {
  std::vector<error_s> errors;
  std::vector<std::string> chunks;
  compilation_unit_metadata_s metadata;

  bool has_errors() const { return !errors.empty(); }
  std::string assemble_code() const;
  assembly_result_s assemble(assembly_type_e type,
                             const std::string &header_name = "") const;
  std::string assemble_test_runner() const;
};

class emitter_c : public truk::language::nodes::visitor_if {
  friend class builtin_handler_if;
  friend class make_builtin_handler_c;
  friend class delete_builtin_handler_c;
  friend class len_builtin_handler_c;
  friend class sizeof_builtin_handler_c;
  friend class panic_builtin_handler_c;
  friend class each_builtin_handler_c;
  friend class va_arg_builtin_handler_c;
  friend class expression_visitor_c;

public:
  emitter_c();
  ~emitter_c() override = default;

  emitter_c &add_declaration(const truk::language::nodes::base_c *decl);
  emitter_c &add_declarations(
      const std::vector<std::unique_ptr<truk::language::nodes::base_c>> &decls);
  emitter_c &
  set_c_imports(const std::vector<truk::language::nodes::c_import_s> &imports);
  emitter_c &set_declaration_file_map(
      const std::unordered_map<const truk::language::nodes::base_c *,
                               std::string> &map) {
    _decl_to_file = map;
    return *this;
  }
  emitter_c &set_file_to_shards_map(
      const std::unordered_map<std::string, std::vector<std::string>> &map) {
    _file_to_shards = map;
    return *this;
  }

  result_c finalize();

  void visit(const truk::language::nodes::primitive_type_c &node) override;
  void visit(const truk::language::nodes::named_type_c &node) override;
  void visit(const truk::language::nodes::pointer_type_c &node) override;
  void visit(const truk::language::nodes::array_type_c &node) override;
  void visit(const truk::language::nodes::function_type_c &node) override;
  void visit(const truk::language::nodes::map_type_c &node) override;
  void visit(const truk::language::nodes::tuple_type_c &node) override;
  void visit(
      const truk::language::nodes::generic_type_instantiation_c &node) override;
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
  void visit(const truk::language::nodes::match_c &node) override;
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
  void collect_declarations(const truk::language::nodes::base_c *root);
  void emit_forward_declarations();
  void emit(const truk::language::nodes::base_c *root);
  void internal_finalize();

  void add_error(const std::string &msg,
                 const truk::language::nodes::base_c *node);

  void collect_and_emit_generic_instantiations();
  void emit_generic_instantiation(
      const truk::language::nodes::struct_c *generic_def,
      const std::vector<const truk::language::nodes::type_c *> &type_args,
      const std::string &mangled_name);
  std::string emit_type_with_substitution(
      const truk::language::nodes::type_c *type,
      const std::unordered_map<
          std::string, const truk::language::nodes::type_c *> &substitutions);
  const truk::language::nodes::type_c *substitute_type(
      const truk::language::nodes::type_c *type,
      const std::unordered_map<
          std::string, const truk::language::nodes::type_c *> &substitutions);

  std::string emit_type(const truk::language::nodes::type_c *type);
  std::string emit_type_for_sizeof(const truk::language::nodes::type_c *type);
  std::string
  emit_array_pointer_type(const truk::language::nodes::type_c *array_type,
                          const std::string &identifier = "");
  std::string
  get_slice_type_name(const truk::language::nodes::type_c *element_type);
  void ensure_slice_typedef(const truk::language::nodes::type_c *element_type);
  bool is_slice_type(const truk::language::nodes::type_c *type);
  std::string
  get_map_type_name(const truk::language::nodes::type_c *key_type,
                    const truk::language::nodes::type_c *value_type);
  void ensure_map_typedef(const truk::language::nodes::type_c *key_type,
                          const truk::language::nodes::type_c *value_type);
  bool is_map_type(const truk::language::nodes::type_c *type);
  bool is_array_type(const truk::language::nodes::type_c *type);
  std::string get_array_dimensions(const truk::language::nodes::type_c *type);
  std::string get_tuple_type_name(
      const std::vector<const truk::language::nodes::type_c *> &element_types);
  void ensure_tuple_typedef(
      const std::vector<const truk::language::nodes::type_c *> &element_types);
  std::string get_map_hash_fn(const truk::language::nodes::type_c *key_type);
  std::string get_map_cmp_fn(const truk::language::nodes::type_c *key_type);
  int get_key_size(const truk::language::nodes::type_c *key_type);
  void register_variable_type(const std::string &name,
                              const truk::language::nodes::type_c *type);
  bool is_variable_slice(const std::string &name);
  bool is_variable_map(const std::string &name);
  bool is_variable_string_ptr(const std::string &name);
  bool is_private_identifier(const std::string &name) const;

  std::string emit_expression(const truk::language::nodes::base_c *node);
  std::string
  emit_expr_binary_op(const truk::language::nodes::binary_op_c &node);
  std::string emit_expr_unary_op(const truk::language::nodes::unary_op_c &node);
  std::string emit_expr_cast(const truk::language::nodes::cast_c &node);
  std::string emit_expr_call(const truk::language::nodes::call_c &node);
  std::string emit_expr_index(const truk::language::nodes::index_c &node);
  std::string
  emit_expr_member_access(const truk::language::nodes::member_access_c &node);
  std::string emit_expr_literal(const truk::language::nodes::literal_c &node);
  std::string
  emit_expr_identifier(const truk::language::nodes::identifier_c &node);
  std::string
  emit_expr_array_literal(const truk::language::nodes::array_literal_c &node);
  std::string
  emit_expr_struct_literal(const truk::language::nodes::struct_literal_c &node);
  std::string get_binary_op_string(truk::language::nodes::binary_op_e op);
  std::string get_unary_op_string(truk::language::nodes::unary_op_e op);
  std::string process_char_literal(const std::string &lexeme,
                                   const truk::language::nodes::base_c *node);

  std::vector<const truk::language::nodes::base_c *> _declarations;
  std::unordered_map<const truk::language::nodes::base_c *, std::string>
      _decl_to_file;
  std::unordered_map<std::string, std::vector<std::string>> _file_to_shards;
  result_c _result;
  std::stringstream _current_expr;
  std::stringstream _header;
  std::stringstream _forward_decls;
  std::stringstream _structs;
  std::stringstream _functions;
  int _indent_level{0};
  std::unordered_set<std::string> _function_names;
  std::unordered_set<std::string> _enum_type_names;
  std::unordered_set<std::string> _extern_enum_type_names;
  type_registry_c _type_registry;
  variable_registry_c _variable_registry;
  builtin_registry_c _builtin_registry;
  bool _in_expression{false};
  bool _collecting_declarations{false};
  bool _skip_lambda_generation{false};
  std::string _current_function_name;
  const truk::language::nodes::type_c *_current_function_return_type{nullptr};
  int _lambda_counter{0};
  int _temp_counter{0};
  int _match_counter{0};
  std::unordered_set<std::string> _generated_tuple_typedefs;
  std::vector<const truk::language::nodes::type_c *>
      _current_tuple_return_types;
  std::vector<std::unique_ptr<defer_scope_s>> _defer_scope_stack;
  defer_scope_s *_current_defer_scope{nullptr};
  emission_phase_e _current_phase{emission_phase_e::COLLECTION};
  std::string _current_node_context;
  std::vector<truk::language::nodes::c_import_s> _c_imports;

  std::unordered_map<std::string, const truk::language::nodes::struct_c *>
      _generic_definitions;

  void push_defer_scope(defer_scope_s::scope_type_e type,
                        const truk::language::nodes::base_c *owner);
  void pop_defer_scope();
  void emit_scope_defers(defer_scope_s *scope);
  void emit_all_remaining_defers();
  defer_scope_s *find_enclosing_loop_scope();
};

} // namespace truk::emitc
