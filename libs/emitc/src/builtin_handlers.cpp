#include <language/builtins.hpp>
#include <truk/emitc/builtin_handler.hpp>
#include <truk/emitc/cdef.hpp>
#include <truk/emitc/emitter.hpp>

namespace truk::emitc {

using namespace truk::language::nodes;

class make_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (!node.arguments().empty()) {
      if (auto type_param =
              dynamic_cast<const type_param_c *>(node.arguments()[0].get())) {
        if (node.arguments().size() == 1) {
          if (emitter.is_map_type(type_param->type())) {
            auto *map_type =
                dynamic_cast<const map_type_c *>(type_param->type());
            emitter.ensure_map_typedef(map_type->key_type(),
                                       map_type->value_type());

            std::string map_name = emitter.get_map_type_name(
                map_type->key_type(), map_type->value_type());
            std::string hash_fn = emitter.get_map_hash_fn(map_type->key_type());
            std::string cmp_fn = emitter.get_map_cmp_fn(map_type->key_type());
            int key_size = emitter.get_key_size(map_type->key_type());

            emitter._current_expr << "({" << map_name
                                  << " __tmp; __truk_map_init_generic(&__tmp, "
                                  << key_size << ", " << hash_fn << ", "
                                  << cmp_fn << "); __tmp;})";
            return;
          }

          std::string type_str = emitter.emit_type(type_param->type());
          emitter._current_expr << cdef::emit_builtin_make(type_str);
          return;
        } else if (node.arguments().size() == 2) {
          std::string elem_type_for_sizeof =
              emitter.emit_type_for_sizeof(type_param->type());
          emitter.ensure_slice_typedef(type_param->type());

          std::string count_expr =
              emitter.emit_expression(node.arguments()[1].get());

          std::string slice_type =
              emitter.get_slice_type_name(type_param->type());

          std::string cast_type;
          if (auto arr =
                  dynamic_cast<const array_type_c *>(type_param->type())) {
            if (arr->size().has_value()) {
              cast_type = emitter.emit_array_pointer_type(type_param->type());
            } else {
              cast_type = elem_type_for_sizeof + "*";
            }
          } else {
            cast_type = elem_type_for_sizeof + "*";
          }

          emitter._current_expr << cdef::emit_builtin_make_array(
              cast_type, elem_type_for_sizeof, count_expr);
          return;
        }
      }
    }
  }
};

class delete_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (!node.arguments().empty()) {
      if (auto idx = dynamic_cast<const index_c *>(node.arguments()[0].get())) {
        if (auto ident = dynamic_cast<const identifier_c *>(idx->object())) {
          if (emitter.is_variable_map(ident->id().name)) {
            std::string obj_expr = emitter.emit_expression(idx->object());
            std::string idx_expr = emitter.emit_expression(idx->index());

            bool key_is_slice = false;
            auto *key_literal = dynamic_cast<const literal_c *>(idx->index());
            bool key_is_string_literal =
                key_literal && key_literal->type() == literal_type_e::STRING;
            bool key_is_non_string_literal =
                key_literal && !key_is_string_literal;

            if (auto key_ident =
                    dynamic_cast<const identifier_c *>(idx->index())) {
              key_is_slice = emitter.is_variable_slice(key_ident->id().name);
            }

            if (key_is_slice) {
              emitter._current_expr << "__truk_map_remove_generic(&("
                                    << obj_expr << "), &((" << idx_expr
                                    << ").data))";
            } else if (key_is_string_literal) {
              emitter._current_expr
                  << "({ const __truk_u8* __truk_key_tmp = " << idx_expr
                  << "; __truk_map_remove_generic(&(" << obj_expr
                  << "), &__truk_key_tmp); })";
            } else if (key_is_non_string_literal) {
              emitter._current_expr << "({ typeof(" << idx_expr
                                    << ") __truk_key_tmp = " << idx_expr
                                    << "; __truk_map_remove_generic(&("
                                    << obj_expr << "), &__truk_key_tmp); })";
            } else {
              emitter._current_expr << "__truk_map_remove_generic(&("
                                    << obj_expr << "), &(" << idx_expr << "))";
            }
            return;
          }
        }
      }

      std::string arg = emitter.emit_expression(node.arguments()[0].get());

      if (emitter.is_variable_map(arg)) {
        emitter._current_expr << "__truk_map_deinit(&(" << arg << "))";
      } else if (emitter.is_variable_slice(arg)) {
        emitter._current_expr << cdef::emit_builtin_delete_array(arg);
      } else {
        emitter._current_expr << cdef::emit_builtin_delete(arg);
      }
      return;
    }
  }
};

class len_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (!node.arguments().empty()) {
      std::string arg = emitter.emit_expression(node.arguments()[0].get());
      emitter._current_expr << "(" << arg << ").len";
      return;
    }
  }
};

class sizeof_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (!node.arguments().empty()) {
      if (auto type_param =
              dynamic_cast<const type_param_c *>(node.arguments()[0].get())) {
        std::string type_str = emitter.emit_type_for_sizeof(type_param->type());
        emitter._current_expr << cdef::emit_builtin_sizeof(type_str);
        return;
      }
    }
  }
};

class panic_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (!node.arguments().empty()) {
      std::string arg = emitter.emit_expression(node.arguments()[0].get());
      emitter._current_expr << "TRUK_PANIC((" << arg << ").data, (" << arg
                            << ").len)";
      return;
    }
  }
};

class each_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (node.arguments().size() == 3) {
      bool is_slice = false;
      bool is_map = false;

      if (auto ident =
              dynamic_cast<const identifier_c *>(node.arguments()[0].get())) {
        is_slice = emitter.is_variable_slice(ident->id().name);
        is_map = emitter.is_variable_map(ident->id().name);
      }

      std::string collection_var =
          emitter.emit_expression(node.arguments()[0].get());
      std::string context_var =
          emitter.emit_expression(node.arguments()[1].get());

      if (auto lambda =
              dynamic_cast<const lambda_c *>(node.arguments()[2].get())) {
        std::string callback_func =
            emitter.emit_expression(node.arguments()[2].get());

        emitter._functions << cdef::indent(emitter._indent_level) << "{\n";
        emitter._indent_level++;

        if (is_slice) {
          emitter._functions << cdef::indent(emitter._indent_level)
                             << "for (__truk_u64 __truk_idx = 0; __truk_idx < ("
                             << collection_var << ").len; __truk_idx++) {\n";
          emitter._indent_level++;
          emitter._functions
              << cdef::indent(emitter._indent_level)
              << "__truk_bool __truk_continue = " << callback_func << "(&("
              << collection_var << ").data[__truk_idx], " << context_var
              << ");\n";
          emitter._functions << cdef::indent(emitter._indent_level)
                             << "if (!__truk_continue) break;\n";
          emitter._indent_level--;
          emitter._functions << cdef::indent(emitter._indent_level) << "}\n";
        } else {
          std::string key_type =
              emitter.emit_type(lambda->params()[0].type.get());

          emitter._functions
              << cdef::indent(emitter._indent_level)
              << "__truk_map_iter_t __truk_iter = __truk_map_iter();\n";
          emitter._functions << cdef::indent(emitter._indent_level) << key_type
                             << "* __truk_key_ptr;\n";
          emitter._functions << cdef::indent(emitter._indent_level)
                             << "while ((__truk_key_ptr = (" << key_type
                             << "*)__truk_map_next_generic(&(" << collection_var
                             << "), &__truk_iter)) != NULL) {\n";
          emitter._indent_level++;
          emitter._functions << cdef::indent(emitter._indent_level) << key_type
                             << " __truk_key = *__truk_key_ptr;\n";
          emitter._functions
              << cdef::indent(emitter._indent_level)
              << "__truk_bool __truk_continue = " << callback_func
              << "(__truk_key, __truk_map_get_generic(&(" << collection_var
              << "), __truk_key_ptr), " << context_var << ");\n";
          emitter._functions << cdef::indent(emitter._indent_level)
                             << "if (!__truk_continue) break;\n";
          emitter._indent_level--;
          emitter._functions << cdef::indent(emitter._indent_level) << "}\n";
        }

        emitter._indent_level--;
        emitter._functions << cdef::indent(emitter._indent_level) << "}\n";
      } else {
        std::string callback_func =
            emitter.emit_expression(node.arguments()[2].get());

        emitter._functions << cdef::indent(emitter._indent_level) << "{\n";
        emitter._indent_level++;

        if (is_slice) {
          emitter._functions << cdef::indent(emitter._indent_level)
                             << "for (__truk_u64 __truk_idx = 0; __truk_idx < ("
                             << collection_var << ").len; __truk_idx++) {\n";
          emitter._indent_level++;
          emitter._functions
              << cdef::indent(emitter._indent_level)
              << "__truk_bool __truk_continue = " << callback_func << "(&("
              << collection_var << ").data[__truk_idx], " << context_var
              << ");\n";
          emitter._functions << cdef::indent(emitter._indent_level)
                             << "if (!__truk_continue) break;\n";
          emitter._indent_level--;
          emitter._functions << cdef::indent(emitter._indent_level) << "}\n";
        } else {
          if (auto func_call =
                  dynamic_cast<const call_c *>(node.arguments()[2].get())) {
            if (auto func_ident =
                    dynamic_cast<const identifier_c *>(func_call->callee())) {
              std::string key_type = "__truk_u8*";
              emitter._functions
                  << cdef::indent(emitter._indent_level)
                  << "__truk_map_iter_t __truk_iter = __truk_map_iter();\n";
              emitter._functions << cdef::indent(emitter._indent_level)
                                 << key_type << " __truk_key_ptr;\n";
              emitter._functions << cdef::indent(emitter._indent_level)
                                 << "while ((__truk_key_ptr = (" << key_type
                                 << ")__truk_map_next_generic(&("
                                 << collection_var
                                 << "), &__truk_iter)) != NULL) {\n";
              emitter._indent_level++;
              emitter._functions << cdef::indent(emitter._indent_level)
                                 << key_type
                                 << " __truk_key = *__truk_key_ptr;\n";
              emitter._functions
                  << cdef::indent(emitter._indent_level)
                  << "__truk_bool __truk_continue = " << callback_func
                  << "(__truk_key, __truk_map_get_generic(&(" << collection_var
                  << "), __truk_key_ptr), " << context_var << ");\n";
              emitter._functions << cdef::indent(emitter._indent_level)
                                 << "if (!__truk_continue) break;\n";
              emitter._indent_level--;
              emitter._functions << cdef::indent(emitter._indent_level)
                                 << "}\n";
            }
          }
        }

        emitter._indent_level--;
        emitter._functions << cdef::indent(emitter._indent_level) << "}\n";
      }

      emitter._current_expr.str("");
      emitter._current_expr.clear();
      return;
    }
  }
};

class va_arg_builtin_handler_c : public builtin_handler_if {
public:
  void emit_call(const call_c &node, emitter_c &emitter) override {
    if (auto ident = dynamic_cast<const identifier_c *>(node.callee())) {
      const std::string &func_name = ident->id().name;
      auto builtin = language::builtins::lookup_builtin(func_name);

      if (builtin) {
        switch (builtin->kind) {
        case language::builtins::builtin_kind_e::VA_ARG_I32:
          emitter._current_expr << "va_arg(__truk_va_args, __truk_i32)";
          return;
        case language::builtins::builtin_kind_e::VA_ARG_I64:
          emitter._current_expr << "va_arg(__truk_va_args, __truk_i64)";
          return;
        case language::builtins::builtin_kind_e::VA_ARG_F64:
          emitter._current_expr << "va_arg(__truk_va_args, __truk_f64)";
          return;
        case language::builtins::builtin_kind_e::VA_ARG_PTR:
          emitter._current_expr << "va_arg(__truk_va_args, __truk_void*)";
          return;
        default:
          break;
        }
      }
    }
  }
};

void register_builtin_handlers(builtin_registry_c &registry) {
  registry.register_handler("make", std::make_unique<make_builtin_handler_c>());
  registry.register_handler("delete",
                            std::make_unique<delete_builtin_handler_c>());
  registry.register_handler("len", std::make_unique<len_builtin_handler_c>());
  registry.register_handler("sizeof",
                            std::make_unique<sizeof_builtin_handler_c>());
  registry.register_handler("panic",
                            std::make_unique<panic_builtin_handler_c>());
  registry.register_handler("each", std::make_unique<each_builtin_handler_c>());
  registry.register_handler("__TRUK_VA_ARG_I32",
                            std::make_unique<va_arg_builtin_handler_c>());
  registry.register_handler("__TRUK_VA_ARG_I64",
                            std::make_unique<va_arg_builtin_handler_c>());
  registry.register_handler("__TRUK_VA_ARG_F64",
                            std::make_unique<va_arg_builtin_handler_c>());
  registry.register_handler("__TRUK_VA_ARG_PTR",
                            std::make_unique<va_arg_builtin_handler_c>());
}

} // namespace truk::emitc
