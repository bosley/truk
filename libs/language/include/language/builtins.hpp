#pragma once

#include "node.hpp"
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace truk::language::builtins {

enum class builtin_kind_e {
  ALLOC,
  FREE,
  ALLOC_ARRAY,
  FREE_ARRAY,
  LEN,
  SIZEOF,
  PANIC,
  PRINTF,
  VA_ARG_I32,
  VA_ARG_I64,
  VA_ARG_F64,
  VA_ARG_PTR,
  ARGC,
  ARGV
};

struct builtin_signature_s {
  std::string name;
  builtin_kind_e kind;
  bool takes_type_param;
  bool is_variadic;
  std::vector<std::string> param_names;
  std::function<nodes::type_ptr(const nodes::type_c *)> build_signature;
};

const std::vector<builtin_signature_s> &get_builtins();
const builtin_signature_s *lookup_builtin(const std::string &name);

nodes::type_ptr clone_type(const nodes::type_c *type);

} // namespace truk::language::builtins
