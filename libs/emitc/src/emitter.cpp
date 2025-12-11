#include <language/builtins.hpp>
#include <language/keywords.hpp>
#include <truk/emitc/cdef.hpp>
#include <truk/emitc/emitter.hpp>

namespace truk::emitc {

using namespace truk::language;
using namespace truk::language::nodes;

emitter_c::emitter_c() { _header << cdef::emit_program_header(); }

void emitter_c::emit(const base_c *root) {
  if (root) {
    root->accept(*this);
  }
}

void emitter_c::finalize() {
  _result.chunks.push_back(_header.str());
  _result.chunks.push_back(_structs.str());
  _result.chunks.push_back(_functions.str());
}

std::string emitter_c::emit_type(const type_c *type) {
  if (!type) {
    return "void";
  }

  if (auto prim = dynamic_cast<const primitive_type_c *>(type)) {
    switch (prim->keyword()) {
    case keywords_e::I8:
      return "i8";
    case keywords_e::I16:
      return "i16";
    case keywords_e::I32:
      return "i32";
    case keywords_e::I64:
      return "i64";
    case keywords_e::U8:
      return "u8";
    case keywords_e::U16:
      return "u16";
    case keywords_e::U32:
      return "u32";
    case keywords_e::U64:
      return "u64";
    case keywords_e::F32:
      return "f32";
    case keywords_e::F64:
      return "f64";
    case keywords_e::BOOL:
      return "bool";
    case keywords_e::VOID:
      return "void";
    default:
      return "void";
    }
  }

  if (auto named = dynamic_cast<const named_type_c *>(type)) {
    return named->name().name;
  }

  if (auto ptr = dynamic_cast<const pointer_type_c *>(type)) {
    return emit_type(ptr->pointee_type()) + "*";
  }

  if (auto arr = dynamic_cast<const array_type_c *>(type)) {
    if (arr->size().has_value()) {
      return emit_type(arr->element_type());
    } else {
      return get_slice_type_name(arr->element_type());
    }
  }

  return "void";
}

std::string emitter_c::get_slice_type_name(const type_c *element_type) {
  std::string elem_type_str = emit_type(element_type);
  std::string slice_name = "truk_slice_" + elem_type_str;
  for (auto &c : slice_name) {
    if (c == '*')
      c = 'p';
    if (c == '[' || c == ']')
      c = '_';
  }
  return slice_name;
}

void emitter_c::ensure_slice_typedef(const type_c *element_type) {
  std::string slice_name = get_slice_type_name(element_type);
  if (_slice_types_emitted.find(slice_name) == _slice_types_emitted.end()) {
    _slice_types_emitted.insert(slice_name);
    std::string elem_type_str = emit_type(element_type);
    _header << cdef::emit_slice_typedef(elem_type_str, slice_name);
  }
}

bool emitter_c::is_slice_type(const type_c *type) {
  if (auto arr = dynamic_cast<const array_type_c *>(type)) {
    return !arr->size().has_value();
  }
  return false;
}

void emitter_c::register_variable_type(const std::string &name,
                                       const type_c *type) {
  _variable_is_slice[name] = is_slice_type(type);
}

bool emitter_c::is_variable_slice(const std::string &name) {
  auto it = _variable_is_slice.find(name);
  if (it != _variable_is_slice.end()) {
    return it->second;
  }
  return false;
}

void emitter_c::visit(const primitive_type_c &node) {
  _current_expr << emit_type(&node);
}

void emitter_c::visit(const named_type_c &node) {
  _current_expr << node.name().name;
}

void emitter_c::visit(const pointer_type_c &node) {
  _current_expr << emit_type(&node);
}

void emitter_c::visit(const array_type_c &node) {
  _current_expr << emit_type(&node);
}

void emitter_c::visit(const function_type_c &node) {}

void emitter_c::visit(const fn_c &node) {
  std::string return_type = emit_type(node.return_type());
  _functions << return_type << " " << node.name().name << "(";

  for (size_t i = 0; i < node.params().size(); ++i) {
    if (i > 0)
      _functions << ", ";
    const auto &param = node.params()[i];
    register_variable_type(param.name.name, param.type.get());

    std::string param_type = emit_type(param.type.get());

    if (auto arr = dynamic_cast<const array_type_c *>(param.type.get())) {
      if (arr->size().has_value()) {
        _functions << param_type << " " << param.name.name << "["
                   << arr->size().value() << "]";
        continue;
      } else {
        ensure_slice_typedef(arr->element_type());
      }
    }

    _functions << param_type << " " << param.name.name;
  }

  _functions << ") ";

  if (node.body()) {
    node.body()->accept(*this);
  }

  _functions << "\n\n";
}

void emitter_c::visit(const struct_c &node) {
  _structs << "typedef struct {\n";

  for (const auto &field : node.fields()) {
    std::string field_type = emit_type(field.type.get());
    _structs << "  " << field_type << " " << field.name.name;

    if (auto arr = dynamic_cast<const array_type_c *>(field.type.get())) {
      if (arr->size().has_value()) {
        _structs << "[" << arr->size().value() << "]";
      } else {
        ensure_slice_typedef(arr->element_type());
      }
    }

    _structs << ";\n";
  }

  _structs << "} " << node.name().name << ";\n\n";
}

void emitter_c::visit(const var_c &node) {
  register_variable_type(node.name().name, node.type());

  std::string type_str = emit_type(node.type());

  if (_indent_level == 0) {
    _functions << type_str << " " << node.name().name;
  } else {
    _functions << cdef::indent(_indent_level) << type_str << " "
               << node.name().name;
  }

  if (auto arr = dynamic_cast<const array_type_c *>(node.type())) {
    if (arr->size().has_value()) {
      _functions << "[" << arr->size().value() << "]";
    } else {
      ensure_slice_typedef(arr->element_type());
    }
  }

  if (node.initializer()) {
    _functions << " = ";
    _in_expression = true;
    node.initializer()->accept(*this);
    _in_expression = false;
    _functions << _current_expr.str();
    _current_expr.str("");
    _current_expr.clear();
  }

  _functions << ";\n";
}

void emitter_c::visit(const const_c &node) {
  register_variable_type(node.name().name, node.type());

  std::string type_str = emit_type(node.type());

  if (_indent_level == 0) {
    _functions << "const " << type_str << " " << node.name().name;
  } else {
    _functions << cdef::indent(_indent_level) << "const " << type_str << " "
               << node.name().name;
  }

  if (auto arr = dynamic_cast<const array_type_c *>(node.type())) {
    if (arr->size().has_value()) {
      _functions << "[" << arr->size().value() << "]";
    }
  }

  _functions << " = ";
  _in_expression = true;
  node.value()->accept(*this);
  _in_expression = false;
  _functions << _current_expr.str();
  _current_expr.str("");
  _current_expr.clear();
  _functions << ";\n";
}

void emitter_c::visit(const if_c &node) {
  _functions << cdef::indent(_indent_level) << "if (";
  _in_expression = true;
  node.condition()->accept(*this);
  _in_expression = false;
  _functions << _current_expr.str();
  _current_expr.str("");
  _current_expr.clear();
  _functions << ") ";

  node.then_block()->accept(*this);

  if (node.else_block()) {
    _functions << " else ";
    node.else_block()->accept(*this);
  }

  _functions << "\n";
}

void emitter_c::visit(const while_c &node) {
  _functions << cdef::indent(_indent_level) << "while (";
  _in_expression = true;
  node.condition()->accept(*this);
  _in_expression = false;
  _functions << _current_expr.str();
  _current_expr.str("");
  _current_expr.clear();
  _functions << ") ";

  node.body()->accept(*this);
  _functions << "\n";
}

void emitter_c::visit(const for_c &node) {
  _functions << cdef::indent(_indent_level) << "for (";

  if (node.init()) {
    if (dynamic_cast<const var_c *>(node.init())) {
      std::stringstream temp_functions;
      std::swap(temp_functions, _functions);

      node.init()->accept(*this);

      std::string init_str = _functions.str();
      std::swap(temp_functions, _functions);

      while (!init_str.empty() &&
             (init_str.back() == '\n' || init_str.back() == ' ')) {
        init_str.pop_back();
      }
      if (!init_str.empty() && init_str.back() == ';') {
        init_str.pop_back();
      }
      _functions << init_str;
    } else {
      _in_expression = true;
      node.init()->accept(*this);
      _in_expression = false;
      _functions << _current_expr.str();
      _current_expr.str("");
      _current_expr.clear();
    }
  }
  _functions << "; ";

  if (node.condition()) {
    _in_expression = true;
    node.condition()->accept(*this);
    _in_expression = false;
    _functions << _current_expr.str();
    _current_expr.str("");
    _current_expr.clear();
  }
  _functions << "; ";

  if (node.post()) {
    _in_expression = true;
    node.post()->accept(*this);
    _in_expression = false;
    _functions << _current_expr.str();
    _current_expr.str("");
    _current_expr.clear();
  }

  _functions << ") ";

  node.body()->accept(*this);
  _functions << "\n";
}

void emitter_c::visit(const return_c &node) {
  _functions << cdef::indent(_indent_level) << "return";

  if (node.expression()) {
    _functions << " ";
    _in_expression = true;
    node.expression()->accept(*this);
    _in_expression = false;
    _functions << _current_expr.str();
    _current_expr.str("");
    _current_expr.clear();
  }

  _functions << ";\n";
}

void emitter_c::visit(const break_c &node) {
  _functions << cdef::indent(_indent_level) << "break;\n";
}

void emitter_c::visit(const continue_c &node) {
  _functions << cdef::indent(_indent_level) << "continue;\n";
}

void emitter_c::visit(const binary_op_c &node) {
  _current_expr << "(";
  node.left()->accept(*this);

  switch (node.op()) {
  case binary_op_e::ADD:
    _current_expr << " + ";
    break;
  case binary_op_e::SUB:
    _current_expr << " - ";
    break;
  case binary_op_e::MUL:
    _current_expr << " * ";
    break;
  case binary_op_e::DIV:
    _current_expr << " / ";
    break;
  case binary_op_e::MOD:
    _current_expr << " % ";
    break;
  case binary_op_e::EQ:
    _current_expr << " == ";
    break;
  case binary_op_e::NE:
    _current_expr << " != ";
    break;
  case binary_op_e::LT:
    _current_expr << " < ";
    break;
  case binary_op_e::LE:
    _current_expr << " <= ";
    break;
  case binary_op_e::GT:
    _current_expr << " > ";
    break;
  case binary_op_e::GE:
    _current_expr << " >= ";
    break;
  case binary_op_e::AND:
    _current_expr << " && ";
    break;
  case binary_op_e::OR:
    _current_expr << " || ";
    break;
  case binary_op_e::BITWISE_AND:
    _current_expr << " & ";
    break;
  case binary_op_e::BITWISE_OR:
    _current_expr << " | ";
    break;
  case binary_op_e::BITWISE_XOR:
    _current_expr << " ^ ";
    break;
  case binary_op_e::LEFT_SHIFT:
    _current_expr << " << ";
    break;
  case binary_op_e::RIGHT_SHIFT:
    _current_expr << " >> ";
    break;
  }

  node.right()->accept(*this);
  _current_expr << ")";
}

void emitter_c::visit(const unary_op_c &node) {
  switch (node.op()) {
  case unary_op_e::NEG:
    _current_expr << "(-";
    break;
  case unary_op_e::NOT:
    _current_expr << "(!";
    break;
  case unary_op_e::BITWISE_NOT:
    _current_expr << "(~";
    break;
  case unary_op_e::ADDRESS_OF:
    _current_expr << "(&";
    break;
  case unary_op_e::DEREF:
    _current_expr << "(*";
    break;
  }

  node.operand()->accept(*this);
  _current_expr << ")";
}

void emitter_c::visit(const cast_c &node) {
  _current_expr << "((" << emit_type(node.target_type()) << ")";
  node.expression()->accept(*this);
  _current_expr << ")";
}

void emitter_c::visit(const call_c &node) {
  if (auto ident = dynamic_cast<const identifier_c *>(node.callee())) {
    const std::string &func_name = ident->id().name;
    auto builtin = builtins::lookup_builtin(func_name);

    if (builtin) {
      switch (builtin->kind) {
      case builtins::builtin_kind_e::ALLOC: {
        if (!node.arguments().empty()) {
          if (auto type_param = dynamic_cast<const type_param_c *>(
                  node.arguments()[0].get())) {
            std::string type_str = emit_type(type_param->type());
            _current_expr << cdef::emit_builtin_alloc(type_str);
            return;
          }
        }
        break;
      }
      case builtins::builtin_kind_e::FREE: {
        if (!node.arguments().empty()) {
          _current_expr << cdef::emit_builtin_free("");
          std::string saved = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          node.arguments()[0]->accept(*this);
          std::string arg = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          _current_expr << "free(" << arg << ")";

          if (!_in_expression) {
            _functions << cdef::indent(_indent_level) << _current_expr.str()
                       << ";\n";
            _current_expr.str("");
            _current_expr.clear();
          }
          return;
        }
        break;
      }
      case builtins::builtin_kind_e::ALLOC_ARRAY: {
        if (node.arguments().size() >= 2) {
          if (auto type_param = dynamic_cast<const type_param_c *>(
                  node.arguments()[0].get())) {
            std::string elem_type_str = emit_type(type_param->type());
            ensure_slice_typedef(type_param->type());

            std::string saved = _current_expr.str();
            _current_expr.str("");
            _current_expr.clear();
            node.arguments()[1]->accept(*this);
            std::string count_expr = _current_expr.str();
            _current_expr.str("");
            _current_expr.clear();

            std::string slice_type = get_slice_type_name(type_param->type());
            _current_expr << "(" << slice_type << "){(" << elem_type_str
                          << "*)malloc(sizeof(" << elem_type_str << ") * ("
                          << count_expr << ")), (" << count_expr << ")}";
            return;
          }
        }
        break;
      }
      case builtins::builtin_kind_e::FREE_ARRAY: {
        if (!node.arguments().empty()) {
          std::string saved = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          node.arguments()[0]->accept(*this);
          std::string arg = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          _current_expr << "free((" << arg << ").data)";

          if (!_in_expression) {
            _functions << cdef::indent(_indent_level) << _current_expr.str()
                       << ";\n";
            _current_expr.str("");
            _current_expr.clear();
          }
          return;
        }
        break;
      }
      case builtins::builtin_kind_e::LEN: {
        if (!node.arguments().empty()) {
          std::string saved = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          node.arguments()[0]->accept(*this);
          std::string arg = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          _current_expr << "(" << arg << ").len";
          return;
        }
        break;
      }
      case builtins::builtin_kind_e::SIZEOF: {
        if (!node.arguments().empty()) {
          if (auto type_param = dynamic_cast<const type_param_c *>(
                  node.arguments()[0].get())) {
            std::string type_str = emit_type(type_param->type());
            _current_expr << "sizeof(" << type_str << ")";
            return;
          }
        }
        break;
      }
      case builtins::builtin_kind_e::PANIC: {
        if (!node.arguments().empty()) {
          std::string saved = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          node.arguments()[0]->accept(*this);
          std::string arg = _current_expr.str();
          _current_expr.str("");
          _current_expr.clear();
          _current_expr << "TRUK_PANIC((" << arg << ").data, (" << arg
                        << ").len)";
          return;
        }
        break;
      }
      }
    }
  }

  node.callee()->accept(*this);
  _current_expr << "(";

  for (size_t i = 0; i < node.arguments().size(); ++i) {
    if (i > 0)
      _current_expr << ", ";
    node.arguments()[i]->accept(*this);
  }

  _current_expr << ")";

  if (!_in_expression) {
    _functions << cdef::indent(_indent_level) << _current_expr.str() << ";\n";
    _current_expr.str("");
    _current_expr.clear();
  }
}

void emitter_c::visit(const index_c &node) {
  std::stringstream obj_stream;
  std::swap(obj_stream, _current_expr);
  node.object()->accept(*this);
  std::string obj_expr = _current_expr.str();
  std::swap(obj_stream, _current_expr);

  std::stringstream idx_stream;
  std::swap(idx_stream, _current_expr);
  node.index()->accept(*this);
  std::string idx_expr = _current_expr.str();
  std::swap(idx_stream, _current_expr);

  bool is_slice = false;
  if (auto ident = dynamic_cast<const identifier_c *>(node.object())) {
    is_slice = is_variable_slice(ident->id().name);
  } else {
    is_slice = true;
  }

  if (is_slice) {
    _current_expr << "({ truk_bounds_check(" << idx_expr << ", (" << obj_expr
                  << ").len); (" << obj_expr << ").data[" << idx_expr
                  << "]; })";
  } else {
    _current_expr << obj_expr << "[" << idx_expr << "]";
  }
}

void emitter_c::visit(const member_access_c &node) {
  node.object()->accept(*this);
  _current_expr << "." << node.field().name;
}

void emitter_c::visit(const literal_c &node) {
  switch (node.type()) {
  case literal_type_e::INTEGER:
  case literal_type_e::FLOAT:
    _current_expr << node.value();
    break;
  case literal_type_e::STRING:
    _current_expr << "\"" << node.value() << "\"";
    break;
  case literal_type_e::BOOL:
    _current_expr << (node.value() == "true" ? "true" : "false");
    break;
  case literal_type_e::NIL:
    _current_expr << "NULL";
    break;
  }
}

void emitter_c::visit(const identifier_c &node) {
  _current_expr << node.id().name;
}

void emitter_c::visit(const assignment_c &node) {
  bool was_in_expr = _in_expression;

  if (auto idx = dynamic_cast<const index_c *>(node.target())) {
    bool is_slice = false;
    if (auto ident = dynamic_cast<const identifier_c *>(idx->object())) {
      is_slice = is_variable_slice(ident->id().name);
    } else {
      is_slice = true;
    }

    if (is_slice && !was_in_expr) {
      std::stringstream obj_stream;
      std::swap(obj_stream, _current_expr);
      _in_expression = true;
      idx->object()->accept(*this);
      std::string obj_expr = _current_expr.str();
      std::swap(obj_stream, _current_expr);

      std::stringstream idx_stream;
      std::swap(idx_stream, _current_expr);
      idx->index()->accept(*this);
      std::string idx_expr = _current_expr.str();
      std::swap(idx_stream, _current_expr);

      node.value()->accept(*this);
      std::string value = _current_expr.str();
      _current_expr.str("");
      _current_expr.clear();
      _in_expression = was_in_expr;

      _functions << cdef::indent(_indent_level);
      _functions << "truk_bounds_check(" << idx_expr << ", (" << obj_expr
                 << ").len);\n";
      _functions << cdef::indent(_indent_level);
      _functions << "(" << obj_expr << ").data[" << idx_expr << "] = " << value
                 << ";\n";
      return;
    }
  }

  if (!was_in_expr) {
    _functions << cdef::indent(_indent_level);
  }

  _in_expression = true;
  node.target()->accept(*this);
  std::string target = _current_expr.str();
  _current_expr.str("");
  _current_expr.clear();

  node.value()->accept(*this);
  std::string value = _current_expr.str();
  _current_expr.str("");
  _current_expr.clear();
  _in_expression = was_in_expr;

  if (was_in_expr) {
    _current_expr << target << " = " << value;
  } else {
    _functions << target << " = " << value << ";\n";
  }
}

void emitter_c::visit(const block_c &node) {
  _functions << "{\n";
  _indent_level++;

  for (const auto &stmt : node.statements()) {
    stmt->accept(*this);
  }

  _indent_level--;
  _functions << cdef::indent(_indent_level) << "}";
}

void emitter_c::visit(const array_literal_c &node) {
  _current_expr << "{";

  for (size_t i = 0; i < node.elements().size(); ++i) {
    if (i > 0)
      _current_expr << ", ";
    node.elements()[i]->accept(*this);
  }

  _current_expr << "}";
}

void emitter_c::visit(const struct_literal_c &node) {
  _current_expr << "(" << node.struct_name().name << "){";

  for (size_t i = 0; i < node.field_initializers().size(); ++i) {
    if (i > 0)
      _current_expr << ", ";
    const auto &field_init = node.field_initializers()[i];
    _current_expr << "." << field_init.field_name.name << " = ";
    field_init.value->accept(*this);
  }

  _current_expr << "}";
}

void emitter_c::visit(const type_param_c &node) {}

} // namespace truk::emitc
