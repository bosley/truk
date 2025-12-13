#pragma once

#include <language/node.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace truk::commands {

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
  void build_dependency_graph();
  void analyze_dependencies(const truk::language::nodes::base_c *decl,
                            std::unordered_set<std::string> &deps);
  void analyze_type_dependencies(const truk::language::nodes::type_c *type,
                                 std::unordered_set<std::string> &deps);
  void analyze_expr_dependencies(const truk::language::nodes::base_c *expr,
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

} // namespace truk::commands
