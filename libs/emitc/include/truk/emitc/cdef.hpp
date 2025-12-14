#pragma once

#include "embedded_runtime.hpp"
#include <fmt/core.h>
#include <sstream>
#include <string>

namespace truk::emitc::cdef {

inline std::string strip_pragma_and_includes(const std::string &content) {
  std::stringstream result;
  std::istringstream stream(content);
  std::string line;

  while (std::getline(stream, line)) {
    if (line.find("#pragma once") != std::string::npos) {
      continue;
    }
    if (line.find("#include \"types.h\"") != std::string::npos) {
      continue;
    }
    if (line.find("#include <sxs/runtime.h>") != std::string::npos) {
      continue;
    }
    if (line.empty() && result.str().empty()) {
      continue;
    }
    result << line << "\n";
  }

  return result.str();
}

inline std::string assemble_runtime_for_application() {
  std::stringstream ss;

  ss << "#include <stdlib.h>\n";
  ss << "#include <stdio.h>\n";
  ss << "#include <string.h>\n";
  ss << "#include <stdarg.h>\n\n";

  if (embedded::runtime_files.count("include/sxs/types.h")) {
    ss << strip_pragma_and_includes(
        embedded::runtime_files.at("include/sxs/types.h").content);
  }

  if (embedded::runtime_files.count("include/sxs/runtime.h")) {
    ss << strip_pragma_and_includes(
        embedded::runtime_files.at("include/sxs/runtime.h").content);
  }

  ss << "#define TRUK_PANIC(msg, len) sxs_panic((msg), (len))\n";
  ss << "#define TRUK_BOUNDS_CHECK(idx, len) sxs_bounds_check((idx), "
        "(len))\n\n";

  ss << "#define TRUK_DEFER_SCOPE_BEGIN() do {\n";
  ss << "#define TRUK_DEFER_SCOPE_END(...) } while(0); __VA_ARGS__\n";
  ss << "#define TRUK_ANONYMOUS(body) do { body } while(0)\n\n";

  if (embedded::runtime_files.count("src/runtime.c")) {
    ss << strip_pragma_and_includes(
        embedded::runtime_files.at("src/runtime.c").content);
  }

  return ss.str();
}

inline std::string assemble_runtime_for_library() {
  std::stringstream ss;

  if (embedded::runtime_files.count("include/sxs/types.h")) {
    ss << strip_pragma_and_includes(
        embedded::runtime_files.at("include/sxs/types.h").content);
  }

  if (embedded::runtime_files.count("include/sxs/runtime.h")) {
    ss << strip_pragma_and_includes(
        embedded::runtime_files.at("include/sxs/runtime.h").content);
  }

  return ss.str();
}

inline std::string emit_program_header() {
  return assemble_runtime_for_application();
}

inline std::string emit_library_header() {
  return assemble_runtime_for_library();
}

inline std::string emit_slice_typedef(const std::string &element_type,
                                      const std::string &slice_name) {
  return fmt::format("typedef struct {{\n  {}* data;\n  u64 len;\n}} {};\n\n",
                     element_type, slice_name);
}

inline std::string emit_builtin_alloc(const std::string &type_str) {
  return fmt::format("({0}*)sxs_alloc(sizeof({0}))", type_str);
}

inline std::string emit_builtin_free(const std::string &ptr_expr) {
  return fmt::format("sxs_free({})", ptr_expr);
}

inline std::string emit_builtin_alloc_array(const std::string &cast_type,
                                            const std::string &elem_type_for_sizeof,
                                            const std::string &count_expr) {
  return fmt::format("{{({0})sxs_alloc_array(sizeof({1}), ({2})), ({2})}}",
                     cast_type, elem_type_for_sizeof, count_expr);
}

inline std::string emit_builtin_free_array(const std::string &arr_expr) {
  return fmt::format("sxs_free_array(({}).data)", arr_expr);
}

inline std::string emit_builtin_len(const std::string &arr_expr) {
  return fmt::format("({}).len", arr_expr);
}

inline std::string emit_builtin_sizeof(const std::string &type_str) {
  return fmt::format("sxs_sizeof_type(sizeof({}))", type_str);
}

inline std::string emit_builtin_panic(const std::string &msg_expr) {
  return fmt::format("TRUK_PANIC(({0}).data, ({0}).len)", msg_expr);
}

inline std::string emit_bounds_check(const std::string &idx_expr,
                                     const std::string &len_expr) {
  return fmt::format("TRUK_BOUNDS_CHECK({}, {})", idx_expr, len_expr);
}

inline std::string indent(int level) { return std::string(level * 2, ' '); }

} // namespace truk::emitc::cdef