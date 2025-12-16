#include "../include/language/builtins.hpp"
#include <memory>

namespace truk::language::builtins {

using namespace truk::language::nodes;

nodes::type_ptr clone_type(const nodes::type_c *type) {
  if (!type) {
    return nullptr;
  }

  if (auto *primitive = dynamic_cast<const primitive_type_c *>(type)) {
    return std::make_unique<primitive_type_c>(primitive->keyword(),
                                              primitive->source_index());
  }

  if (auto *named = dynamic_cast<const named_type_c *>(type)) {
    return std::make_unique<named_type_c>(named->source_index(), named->name());
  }

  if (auto *pointer = dynamic_cast<const pointer_type_c *>(type)) {
    auto pointee = clone_type(pointer->pointee_type());
    return std::make_unique<pointer_type_c>(pointer->source_index(),
                                            std::move(pointee));
  }

  if (auto *array = dynamic_cast<const array_type_c *>(type)) {
    auto element = clone_type(array->element_type());
    return std::make_unique<array_type_c>(array->source_index(),
                                          std::move(element), array->size());
  }

  if (auto *function = dynamic_cast<const function_type_c *>(type)) {
    std::vector<type_ptr> param_types;
    for (const auto &param : function->param_types()) {
      param_types.push_back(clone_type(param.get()));
    }
    auto return_type = clone_type(function->return_type());
    return std::make_unique<function_type_c>(function->source_index(),
                                             std::move(param_types),
                                             std::move(return_type));
  }

  return nullptr;
}

static type_ptr build_make_signature(const type_c *type_param) {
  std::vector<type_ptr> params;

  auto pointee = clone_type(type_param);
  auto return_type = std::make_unique<pointer_type_c>(0, std::move(pointee));

  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_delete_signature(const type_c *type_param) {
  std::vector<type_ptr> params;

  auto void_type = std::make_unique<primitive_type_c>(keywords_e::VOID, 0);
  auto ptr_type = std::make_unique<pointer_type_c>(0, std::move(void_type));
  params.push_back(std::move(ptr_type));

  auto return_type = std::make_unique<primitive_type_c>(keywords_e::VOID, 0);

  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_len_signature(const type_c *type_param) {
  std::vector<type_ptr> params;

  auto void_type = std::make_unique<primitive_type_c>(keywords_e::VOID, 0);
  auto array_param =
      std::make_unique<array_type_c>(0, std::move(void_type), std::nullopt);
  params.push_back(std::move(array_param));

  auto return_type = std::make_unique<primitive_type_c>(keywords_e::U64, 0);

  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_sizeof_signature(const type_c *type_param) {
  std::vector<type_ptr> params;

  auto return_type = std::make_unique<primitive_type_c>(keywords_e::U64, 0);

  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_panic_signature(const type_c *type_param) {
  std::vector<type_ptr> params;

  auto u8_type = std::make_unique<primitive_type_c>(keywords_e::U8, 0);
  auto message_param =
      std::make_unique<array_type_c>(0, std::move(u8_type), std::nullopt);
  params.push_back(std::move(message_param));

  auto return_type = std::make_unique<primitive_type_c>(keywords_e::VOID, 0);

  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_va_arg_i32_signature(const type_c *type_param) {
  std::vector<type_ptr> params;
  auto return_type = std::make_unique<primitive_type_c>(keywords_e::I32, 0);
  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_va_arg_i64_signature(const type_c *type_param) {
  std::vector<type_ptr> params;
  auto return_type = std::make_unique<primitive_type_c>(keywords_e::I64, 0);
  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_va_arg_f64_signature(const type_c *type_param) {
  std::vector<type_ptr> params;
  auto return_type = std::make_unique<primitive_type_c>(keywords_e::F64, 0);
  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static type_ptr build_va_arg_ptr_signature(const type_c *type_param) {
  std::vector<type_ptr> params;
  auto void_type = std::make_unique<primitive_type_c>(keywords_e::VOID, 0);
  auto return_type = std::make_unique<pointer_type_c>(0, std::move(void_type));
  return std::make_unique<function_type_c>(0, std::move(params),
                                           std::move(return_type));
}

static std::vector<builtin_signature_s> builtin_registry = {
    {"make", builtin_kind_e::MAKE, true, false, {}, build_make_signature},
    {"delete",
     builtin_kind_e::DELETE,
     false,
     false,
     {"ptr"},
     build_delete_signature},
    {"len", builtin_kind_e::LEN, false, false, {"arr"}, build_len_signature},
    {"sizeof", builtin_kind_e::SIZEOF, true, false, {}, build_sizeof_signature},
    {"panic",
     builtin_kind_e::PANIC,
     false,
     false,
     {"message"},
     build_panic_signature},
    {"__TRUK_VA_ARG_I32",
     builtin_kind_e::VA_ARG_I32,
     false,
     false,
     {},
     build_va_arg_i32_signature},
    {"__TRUK_VA_ARG_I64",
     builtin_kind_e::VA_ARG_I64,
     false,
     false,
     {},
     build_va_arg_i64_signature},
    {"__TRUK_VA_ARG_F64",
     builtin_kind_e::VA_ARG_F64,
     false,
     false,
     {},
     build_va_arg_f64_signature},
    {"__TRUK_VA_ARG_PTR",
     builtin_kind_e::VA_ARG_PTR,
     false,
     false,
     {},
     build_va_arg_ptr_signature}};

const std::vector<builtin_signature_s> &get_builtins() {
  return builtin_registry;
}

const builtin_signature_s *lookup_builtin(const std::string &name) {
  for (const auto &builtin : builtin_registry) {
    if (builtin.name == name) {
      return &builtin;
    }
  }
  return nullptr;
}

} // namespace truk::language::builtins
