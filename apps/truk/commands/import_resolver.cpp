#include "import_resolver.hpp"
#include "../common/file_utils.hpp"
#include <algorithm>
#include <fmt/core.h>
#include <truk/ingestion/parser.hpp>

namespace truk::commands {

using namespace truk::language::nodes;

resolved_imports_s import_resolver_c::resolve(const std::string &entry_file) {
  _processed_files.clear();
  _import_stack.clear();
  _all_declarations.clear();
  _symbol_to_decl.clear();
  _decl_dependencies.clear();
  _errors.clear();

  process_file(entry_file);

  resolved_imports_s result;
  result.success = _errors.empty();
  result.errors = std::move(_errors);

  if (result.success) {
    result.all_declarations = std::move(_all_declarations);
  }

  return result;
}

void import_resolver_c::process_file(const std::string &file_path) {
  std::string canonical = truk::common::canonicalize_path(file_path);

  if (std::find(_import_stack.begin(), _import_stack.end(), canonical) !=
      _import_stack.end()) {
    std::string cycle;
    for (const auto &f : _import_stack) {
      cycle += f + " -> ";
    }
    cycle += canonical;
    _errors.push_back({"Circular import detected: " + cycle, file_path, 0, 0});
    return;
  }

  if (_processed_files.count(canonical)) {
    return;
  }

  _import_stack.push_back(canonical);

  std::string source;
  try {
    source = truk::common::read_file(file_path);
  } catch (...) {
    _errors.push_back({"Failed to read file: " + file_path, file_path, 0, 0});
    _import_stack.pop_back();
    return;
  }

  truk::ingestion::parser_c parser(source.c_str(), source.size());
  auto parse_result = parser.parse();

  if (!parse_result.success) {
    _errors.push_back({parse_result.error_message, file_path,
                       parse_result.error_line, parse_result.error_column});
    _import_stack.pop_back();
    return;
  }

  extract_imports_and_declarations(parse_result.declarations, canonical);

  _import_stack.pop_back();
  _processed_files.insert(canonical);
}

void import_resolver_c::extract_imports_and_declarations(
    std::vector<base_ptr> &parsed_decls, const std::string &file_path) {

  for (auto &decl : parsed_decls) {
    if (auto *import_node = dynamic_cast<const import_c *>(decl.get())) {
      std::string resolved_path =
          truk::common::resolve_path(import_node->path(), file_path);
      process_file(resolved_path);
    } else {
      if (auto *fn_node = dynamic_cast<const fn_c *>(decl.get())) {
        _symbol_to_decl[fn_node->name().name] = decl.get();
      } else if (auto *struct_node =
                     dynamic_cast<const struct_c *>(decl.get())) {
        _symbol_to_decl[struct_node->name().name] = decl.get();
      } else if (auto *var_node = dynamic_cast<const var_c *>(decl.get())) {
        _symbol_to_decl[var_node->name().name] = decl.get();
      } else if (auto *const_node = dynamic_cast<const const_c *>(decl.get())) {
        _symbol_to_decl[const_node->name().name] = decl.get();
      }

      _all_declarations.push_back(std::move(decl));
    }
  }
}

void import_resolver_c::analyze_type_dependencies(
    const type_c *type, std::unordered_set<std::string> &deps) {
  if (!type) {
    return;
  }

  if (auto *named = dynamic_cast<const named_type_c *>(type)) {
    deps.insert(named->name().name);
  } else if (auto *ptr = dynamic_cast<const pointer_type_c *>(type)) {
    analyze_type_dependencies(ptr->pointee_type(), deps);
  } else if (auto *arr = dynamic_cast<const array_type_c *>(type)) {
    analyze_type_dependencies(arr->element_type(), deps);
  } else if (auto *func = dynamic_cast<const function_type_c *>(type)) {
    for (const auto &param : func->param_types()) {
      analyze_type_dependencies(param.get(), deps);
    }
    analyze_type_dependencies(func->return_type(), deps);
  }
}

void import_resolver_c::analyze_expr_dependencies(
    const base_c *expr, std::unordered_set<std::string> &deps) {
  if (!expr) {
    return;
  }

  if (auto *id = dynamic_cast<const identifier_c *>(expr)) {
    deps.insert(id->id().name);
  } else if (auto *call = dynamic_cast<const call_c *>(expr)) {
    analyze_expr_dependencies(call->callee(), deps);
    for (const auto &arg : call->arguments()) {
      analyze_expr_dependencies(arg.get(), deps);
    }
  } else if (auto *binop = dynamic_cast<const binary_op_c *>(expr)) {
    analyze_expr_dependencies(binop->left(), deps);
    analyze_expr_dependencies(binop->right(), deps);
  } else if (auto *unop = dynamic_cast<const unary_op_c *>(expr)) {
    analyze_expr_dependencies(unop->operand(), deps);
  } else if (auto *cast = dynamic_cast<const cast_c *>(expr)) {
    analyze_expr_dependencies(cast->expression(), deps);
    analyze_type_dependencies(cast->target_type(), deps);
  } else if (auto *idx = dynamic_cast<const index_c *>(expr)) {
    analyze_expr_dependencies(idx->object(), deps);
    analyze_expr_dependencies(idx->index(), deps);
  } else if (auto *member = dynamic_cast<const member_access_c *>(expr)) {
    analyze_expr_dependencies(member->object(), deps);
  } else if (auto *assign = dynamic_cast<const assignment_c *>(expr)) {
    analyze_expr_dependencies(assign->target(), deps);
    analyze_expr_dependencies(assign->value(), deps);
  } else if (auto *arr_lit = dynamic_cast<const array_literal_c *>(expr)) {
    for (const auto &elem : arr_lit->elements()) {
      analyze_expr_dependencies(elem.get(), deps);
    }
  } else if (auto *struct_lit = dynamic_cast<const struct_literal_c *>(expr)) {
    deps.insert(struct_lit->struct_name().name);
    for (const auto &field : struct_lit->field_initializers()) {
      analyze_expr_dependencies(field.value.get(), deps);
    }
  } else if (auto *block = dynamic_cast<const block_c *>(expr)) {
    for (const auto &stmt : block->statements()) {
      analyze_expr_dependencies(stmt.get(), deps);
    }
  } else if (auto *if_stmt = dynamic_cast<const if_c *>(expr)) {
    analyze_expr_dependencies(if_stmt->condition(), deps);
    analyze_expr_dependencies(if_stmt->then_block(), deps);
    if (if_stmt->else_block()) {
      analyze_expr_dependencies(if_stmt->else_block(), deps);
    }
  } else if (auto *while_stmt = dynamic_cast<const while_c *>(expr)) {
    analyze_expr_dependencies(while_stmt->condition(), deps);
    analyze_expr_dependencies(while_stmt->body(), deps);
  } else if (auto *for_stmt = dynamic_cast<const for_c *>(expr)) {
    if (for_stmt->init()) {
      analyze_expr_dependencies(for_stmt->init(), deps);
    }
    if (for_stmt->condition()) {
      analyze_expr_dependencies(for_stmt->condition(), deps);
    }
    if (for_stmt->post()) {
      analyze_expr_dependencies(for_stmt->post(), deps);
    }
    analyze_expr_dependencies(for_stmt->body(), deps);
  } else if (auto *ret = dynamic_cast<const return_c *>(expr)) {
    if (ret->expression()) {
      analyze_expr_dependencies(ret->expression(), deps);
    }
  } else if (auto *defer_stmt = dynamic_cast<const defer_c *>(expr)) {
    analyze_expr_dependencies(defer_stmt->deferred_code(), deps);
  }
}

void import_resolver_c::analyze_dependencies(
    const base_c *decl, std::unordered_set<std::string> &deps) {
  if (auto *fn_node = dynamic_cast<const fn_c *>(decl)) {
    analyze_type_dependencies(fn_node->return_type(), deps);
    for (const auto &param : fn_node->params()) {
      analyze_type_dependencies(param.type.get(), deps);
    }
    if (fn_node->body()) {
      analyze_expr_dependencies(fn_node->body(), deps);
    }
  } else if (auto *var_node = dynamic_cast<const var_c *>(decl)) {
    analyze_type_dependencies(var_node->type(), deps);
    if (var_node->initializer()) {
      analyze_expr_dependencies(var_node->initializer(), deps);
    }
  } else if (auto *const_node = dynamic_cast<const const_c *>(decl)) {
    analyze_type_dependencies(const_node->type(), deps);
    analyze_expr_dependencies(const_node->value(), deps);
  }
}

std::vector<base_ptr> import_resolver_c::topological_sort() {
  for (const auto &decl : _all_declarations) {
    std::unordered_set<std::string> deps;
    analyze_dependencies(decl.get(), deps);
    _decl_dependencies[decl.get()] = deps;
  }

  std::unordered_map<const base_c *, int> in_degree;
  std::unordered_map<const base_c *, std::vector<const base_c *>> adj_list;

  for (const auto &decl : _all_declarations) {
    in_degree[decl.get()] = 0;
  }

  for (const auto &decl : _all_declarations) {
    const auto &deps = _decl_dependencies[decl.get()];
    for (const auto &dep_name : deps) {
      auto it = _symbol_to_decl.find(dep_name);
      if (it != _symbol_to_decl.end()) {
        const base_c *dep_decl = it->second;
        adj_list[dep_decl].push_back(decl.get());
        in_degree[decl.get()]++;
      }
    }
  }

  std::vector<const base_c *> queue;
  for (const auto &decl : _all_declarations) {
    if (in_degree[decl.get()] == 0) {
      queue.push_back(decl.get());
    }
  }

  std::vector<const base_c *> sorted;
  while (!queue.empty()) {
    const base_c *current = queue.back();
    queue.pop_back();
    sorted.push_back(current);

    for (const base_c *neighbor : adj_list[current]) {
      in_degree[neighbor]--;
      if (in_degree[neighbor] == 0) {
        queue.push_back(neighbor);
      }
    }
  }

  if (sorted.size() != _all_declarations.size()) {
    _errors.push_back(
        {"Circular dependency detected in declarations", "", 0, 0});
    return std::move(_all_declarations);
  }

  std::vector<base_ptr> result;
  std::unordered_map<const base_c *, base_ptr> decl_map;
  for (auto &decl : _all_declarations) {
    decl_map[decl.get()] = std::move(decl);
  }

  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    result.push_back(std::move(decl_map[*it]));
  }

  return result;
}

} // namespace truk::commands
