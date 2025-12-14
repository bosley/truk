#pragma once

#include <language/node.hpp>
#include <language/visitor.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace truk::ingestion {

struct import_error_s {
  std::string message;
  std::string file_path;
  std::size_t line;
  std::size_t column;
};

struct resolved_imports_s {
  std::vector<truk::language::nodes::base_ptr> all_declarations;
  std::vector<import_error_s> errors;
  bool success;
};

class dependency_visitor_c : public truk::language::nodes::visitor_if {
public:
  dependency_visitor_c(
      const std::unordered_map<
          std::string, const truk::language::nodes::base_c *> &symbol_to_decl,
      std::unordered_set<std::string> &deps,
      std::unordered_set<std::string> &local_scope)
      : _symbol_to_decl(symbol_to_decl), _deps(deps),
        _local_scope(local_scope) {}

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

private:
  const std::unordered_map<std::string, const truk::language::nodes::base_c *>
      &_symbol_to_decl;
  std::unordered_set<std::string> &_deps;
  std::unordered_set<std::string> &_local_scope;
};

class import_resolver_c {
public:
  import_resolver_c() = default;

  resolved_imports_s resolve(const std::string &entry_file);

private:
  void process_file(const std::string &file_path);
  void extract_imports_and_declarations(
      std::vector<truk::language::nodes::base_ptr> &parsed_decls,
      const std::string &file_path);
  std::vector<truk::language::nodes::base_ptr> topological_sort();
  void analyze_dependencies(const truk::language::nodes::base_c *decl,
                            std::unordered_set<std::string> &deps);

  std::unordered_set<std::string> _processed_files;
  std::vector<std::string> _import_stack;
  std::vector<truk::language::nodes::base_ptr> _all_declarations;
  std::unordered_map<std::string, const truk::language::nodes::base_c *>
      _symbol_to_decl;
  std::unordered_map<const truk::language::nodes::base_c *,
                     std::unordered_set<std::string>>
      _decl_dependencies;
  std::vector<import_error_s> _errors;
};

} // namespace truk::ingestion
