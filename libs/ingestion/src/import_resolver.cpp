#include <algorithm>
#include <truk/ingestion/file_utils.hpp>
#include <truk/ingestion/import_resolver.hpp>
#include <truk/ingestion/parser.hpp>

namespace truk::ingestion {

using namespace truk::language::nodes;

void dependency_visitor_c::visit(const primitive_type_c &) {}

void dependency_visitor_c::visit(const named_type_c &node) {
  _deps.insert(node.name().name);
}

void dependency_visitor_c::visit(const pointer_type_c &node) {
  if (node.pointee_type()) {
    node.pointee_type()->accept(*this);
  }
}

void dependency_visitor_c::visit(const array_type_c &node) {
  if (node.element_type()) {
    node.element_type()->accept(*this);
  }
}

void dependency_visitor_c::visit(const function_type_c &node) {
  for (const auto &param : node.param_types()) {
    if (param) {
      param->accept(*this);
    }
  }
  if (node.return_type()) {
    node.return_type()->accept(*this);
  }
}

void dependency_visitor_c::visit(const fn_c &) {}

void dependency_visitor_c::visit(const struct_c &) {}

void dependency_visitor_c::visit(const var_c &node) {
  _local_scope.insert(node.name().name);
  if (node.initializer()) {
    node.initializer()->accept(*this);
  }
}

void dependency_visitor_c::visit(const const_c &) {}

void dependency_visitor_c::visit(const if_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.then_block()) {
    node.then_block()->accept(*this);
  }
  if (node.else_block()) {
    node.else_block()->accept(*this);
  }
}

void dependency_visitor_c::visit(const while_c &node) {
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.body()) {
    node.body()->accept(*this);
  }
}

void dependency_visitor_c::visit(const for_c &node) {
  std::unordered_set<std::string> for_scope = _local_scope;
  std::swap(_local_scope, for_scope);

  if (node.init()) {
    node.init()->accept(*this);
  }
  if (node.condition()) {
    node.condition()->accept(*this);
  }
  if (node.post()) {
    node.post()->accept(*this);
  }
  if (node.body()) {
    node.body()->accept(*this);
  }

  std::swap(_local_scope, for_scope);
}

void dependency_visitor_c::visit(const return_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);
  }
}

void dependency_visitor_c::visit(const break_c &) {}

void dependency_visitor_c::visit(const continue_c &) {}

void dependency_visitor_c::visit(const defer_c &node) {
  if (node.deferred_code()) {
    node.deferred_code()->accept(*this);
  }
}

void dependency_visitor_c::visit(const binary_op_c &node) {
  if (node.left()) {
    node.left()->accept(*this);
  }
  if (node.right()) {
    node.right()->accept(*this);
  }
}

void dependency_visitor_c::visit(const unary_op_c &node) {
  if (node.operand()) {
    node.operand()->accept(*this);
  }
}

void dependency_visitor_c::visit(const cast_c &node) {
  if (node.expression()) {
    node.expression()->accept(*this);
  }
  if (node.target_type()) {
    node.target_type()->accept(*this);
  }
}

void dependency_visitor_c::visit(const call_c &node) {
  if (node.callee()) {
    node.callee()->accept(*this);
  }
  for (const auto &arg : node.arguments()) {
    if (arg) {
      arg->accept(*this);
    }
  }
}

void dependency_visitor_c::visit(const index_c &node) {
  if (node.object()) {
    node.object()->accept(*this);
  }
  if (node.index()) {
    node.index()->accept(*this);
  }
}

void dependency_visitor_c::visit(const member_access_c &node) {
  if (node.object()) {
    node.object()->accept(*this);
  }
}

void dependency_visitor_c::visit(const literal_c &) {}

void dependency_visitor_c::visit(const identifier_c &node) {
  const std::string &name = node.id().name;
  if (_local_scope.find(name) == _local_scope.end() &&
      _symbol_to_decl.find(name) != _symbol_to_decl.end()) {
    _deps.insert(name);
  }
}

void dependency_visitor_c::visit(const assignment_c &node) {
  if (node.target()) {
    node.target()->accept(*this);
  }
  if (node.value()) {
    node.value()->accept(*this);
  }
}

void dependency_visitor_c::visit(const block_c &node) {
  std::unordered_set<std::string> block_scope = _local_scope;
  std::swap(_local_scope, block_scope);

  for (const auto &stmt : node.statements()) {
    if (stmt) {
      stmt->accept(*this);
    }
  }

  std::swap(_local_scope, block_scope);
}

void dependency_visitor_c::visit(const array_literal_c &node) {
  for (const auto &elem : node.elements()) {
    if (elem) {
      elem->accept(*this);
    }
  }
}

void dependency_visitor_c::visit(const struct_literal_c &node) {
  const std::string &struct_name = node.struct_name().name;
  if (_symbol_to_decl.find(struct_name) != _symbol_to_decl.end()) {
    _deps.insert(struct_name);
  }
  for (const auto &field : node.field_initializers()) {
    if (field.value) {
      field.value->accept(*this);
    }
  }
}

void dependency_visitor_c::visit(const type_param_c &node) {
  if (node.type()) {
    node.type()->accept(*this);
  }
}

void dependency_visitor_c::visit(const import_c &) {}

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
  std::string canonical = canonicalize_path(file_path);

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
    source = read_file(file_path);
  } catch (const std::exception &e) {
    _errors.push_back({e.what(), file_path, 0, 0});
    _import_stack.pop_back();
    return;
  }

  parser_c parser(source.c_str(), source.size());
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
      std::string resolved_path = resolve_path(import_node->path(), file_path);
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

void import_resolver_c::analyze_dependencies(
    const base_c *decl, std::unordered_set<std::string> &deps) {
  std::unordered_set<std::string> local_scope;
  dependency_visitor_c visitor(_symbol_to_decl, deps, local_scope);

  if (auto *fn_node = dynamic_cast<const fn_c *>(decl)) {
    if (fn_node->return_type()) {
      fn_node->return_type()->accept(visitor);
    }

    for (const auto &param : fn_node->params()) {
      if (param.type) {
        param.type->accept(visitor);
      }
      local_scope.insert(param.name.name);
    }

    if (fn_node->body()) {
      fn_node->body()->accept(visitor);
    }
  } else if (auto *struct_node = dynamic_cast<const struct_c *>(decl)) {
    for (const auto &field : struct_node->fields()) {
      if (field.type) {
        field.type->accept(visitor);
      }
    }
  } else if (auto *var_node = dynamic_cast<const var_c *>(decl)) {
    if (var_node->type()) {
      var_node->type()->accept(visitor);
    }
    if (var_node->initializer()) {
      var_node->initializer()->accept(visitor);
    }
  } else if (auto *const_node = dynamic_cast<const const_c *>(decl)) {
    if (const_node->type()) {
      const_node->type()->accept(visitor);
    }
    if (const_node->value()) {
      const_node->value()->accept(visitor);
    }
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

} // namespace truk::ingestion
